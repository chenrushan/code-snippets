#ifndef _CHENRS_CPOOL_H_
#define _CHENRS_CPOOL_H_

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <mutex>
#include <queue>
#include <semaphore.h>

#include <glog/logging.h>

namespace util {

struct ServerAddress {
    std::string ip;
    std::string port;
};

#define DEFAULT_CONN_SIZE_PER_SERVER 15
#define DEFAULT_MAX_TRY_ROUNDS 2
#define DEFAULT_CONN_TIMEOUT 100
#define DEFAULT_RECV_TIMEOUT 3000

// ----------------------------------------------------------------------
// 对于每一次的处理请求，自动找出一个 client 处理
// 找 client 算法：
//   1. client 被分为多组，每个 server 对应一组 client，即一个 client queue
//   2. 随机从某个 server address 开始 poll，直到找到一个可用的 client 为止
//      有两种情况会去 poll 下一个 server address
//      1. 当前 server 无法链接
//      2. 当前 server 所有可用的 client 都已分配出去
//   3. 被选中的 client 会从对应的 queue 中移除，用完后 return 给 queue
//   4. 每次遍历完所有的 server 为一轮，每遍历一轮 try_rounds + 1，每遍历
//      完一轮，如果没有找到 client，则 wait (0.1 * 2 ^^ try_rounds) 秒，
//      如果遍历了 max_try_rounds 后依旧没有找到 client 则返回失败
// ----------------------------------------------------------------------
// TODO: ip pool 是否需要定期更新，还是就 engine 启动时设置一下就不动了
// ----------------------------------------------------------------------
template<typename Client>
class ThriftConnectionPool {
public:

    ThriftConnectionPool(size_t max_try_rounds=DEFAULT_MAX_TRY_ROUNDS);
    ~ThriftConnectionPool();

    void set_addr_pool(
            const ServerAddress &addr,
            size_t max_nconns_per_server=DEFAULT_CONN_SIZE_PER_SERVER,
            unsigned recv_timeout_ms=DEFAULT_RECV_TIMEOUT,
            unsigned conn_timeout_ms=DEFAULT_CONN_TIMEOUT);
    void set_addr_pool(
            const std::vector<ServerAddress> &addrs,
            size_t max_nconns_per_server=DEFAULT_CONN_SIZE_PER_SERVER,
            unsigned recv_timeout_ms=DEFAULT_RECV_TIMEOUT,
            unsigned conn_timeout_ms=DEFAULT_CONN_TIMEOUT);

    template<typename ...Args_t>
    int handle(void (Client::*handle)(Args_t...), Args_t&&... args);

private:

    struct Connection {
        boost::shared_ptr<apache::thrift::transport::TTransport> socket;
        boost::shared_ptr<apache::thrift::transport::TTransport> transport;
        boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol;
        Client *client;

        ~Connection() {
            // XXX: 这个可能 throw exception
            try { transport->close(); } catch(...) {}
            delete client;
        }
    };

    class ConnectionQueue {
    public:
        ConnectionQueue(size_t max_nconns, const ServerAddress *addr,
                unsigned recv_timeout_ms=DEFAULT_RECV_TIMEOUT,
                unsigned conn_timeout_ms=DEFAULT_CONN_TIMEOUT);
        ~ConnectionQueue();
        void add(Connection *conn);
        Connection *pop();

    private:
        const ServerAddress *addr;
        std::queue<Connection *> queue;
        // 由于允许多个 thread 同时访问队列，而队列的修改不能同时有多个
        // thread，因此当一个 thread 通过了 sem 的测试后，仍需要一个 mutex
        // 来保护队列的修改
        std::mutex mutex;
        // sem 用于控制最多并发链接数
        sem_t sem;
        unsigned recv_timeout_ms;
        unsigned conn_timeout_ms;
    };

private:

    std::pair<unsigned, Connection *> get_conn();
    void return_conn(const Connection *conn);
    void release_conn(Connection *conn);

private:

    std::vector<ServerAddress> addrs;
    size_t max_try_rounds;
    size_t try_rounds;

    // connections that are available
    // 每个 addr 对应一个 queue，conn_queues 是线程安全的，因为 conn_queues
    // 是一开始就初始化好，不会在变更了，后面都是读，而每个 queue 是有可能线程
    // 不安全的，因此需要考虑竞争
    // TODO: 将来如果允许运行过程中，自动更新 server 地址，那 @conn_queues 也会
    // 变成一个竞争资源
    std::vector<ConnectionQueue *> conn_queues;
};

// ======================================================================
// connection queue implementation
// ======================================================================

// 一个 queue 对应一个服务器
// @max_nconns: 服务器的链接上限
// @addr: 服务器地址
template<typename Client>
ThriftConnectionPool<Client>::ConnectionQueue::ConnectionQueue(
        size_t max_nconns, const ServerAddress *addr,
        unsigned recv_timeout_ms, unsigned conn_timeout_ms)
    : addr(addr), recv_timeout_ms(recv_timeout_ms), conn_timeout_ms(conn_timeout_ms)
{
    sem_init(&sem, 0, max_nconns);
}

// ----------------------------------------------------------------------

template<typename Client>
ThriftConnectionPool<Client>::ConnectionQueue::~ConnectionQueue()
{
    while (queue.size()) {
        delete queue.front();
        queue.pop();
    }
    sem_destroy(&sem);
}

// ----------------------------------------------------------------------

// 往 queue 添加一个链接
// 如果 conn == nullptr 表示一个链接被 destroy 掉了
template<typename Client>
void ThriftConnectionPool<Client>::ConnectionQueue::add(
        ThriftConnectionPool::Connection *conn)
{
    DLOG(INFO) << "add a " << (conn == nullptr ? "nullptr" : "normal")
               << " connection to queue";
    if (conn != nullptr) {
        std::lock_guard<std::mutex> guard(mutex);
        queue.push(conn);
    }
    sem_post(&sem);
    // print_sem_value(sem, "add");
}

// ----------------------------------------------------------------------

// 从 queue 中取出一个链接
// 1. 首先判断是不是已经达到链接上限了，如果是，则返回失败
// 2. 然后看看 queue 有没有空闲的链接，有则取出并返回
// 3. 没有空闲链接的情况下创建一个，如果失败，则返回失败
template<typename Client>
typename ThriftConnectionPool<Client>::Connection *
ThriftConnectionPool<Client>::ConnectionQueue::pop()
{
    DLOG(INFO) << "request a connection";

    // don't block, just return and try next server
    if (sem_trywait(&sem) != 0 && errno == EAGAIN) {
        LOG(ERROR) << "no more connections available on "
                   << addr->ip << ":" << addr->port;
        return nullptr;
    }
    // print_sem_value(sem, "wait");

    // try to acquire a connection
    Connection *cn = nullptr;
    {
        std::lock_guard<std::mutex> guard(mutex);
        auto sz = queue.size();
        if (sz != 0) {
            cn = queue.front();
            queue.pop();
        }
    }

    // if connection is found, return
    if (cn != nullptr) {
        return cn;
    }

    // if no connection is found in the queue, create one
    DLOG(INFO) << "create a new connection";
    cn = new Connection();

    // ----------------------------------------------------------------------
    // TODO: 貌似不管 server 是否限制链接个数，以下连接都能成功，但是
    // 只有指定个数的链接是可以返回结果的，这个比较恶心，连是连上了，结果
    // 不能用，但是又没有一个很好的方法判断一个链接是否是有效的，同时你又
    // 没有什么办法知道 server 还有多少链接是可用的
    // 
    // 这样的结果就是你返回的链接可能是不能用的，现在只能暂时通过 timeout
    // 的方法缓解一下，因为不能用的链接是等不到结果的
    // ----------------------------------------------------------------------
    try {
        using TSocket = apache::thrift::transport::TSocket;
        using TBufferedTransport = apache::thrift::transport::TBufferedTransport;
        using TBinaryProtocol = apache::thrift::protocol::TBinaryProtocol;

        auto *sock = new TSocket(addr->ip, atoi(addr->port.c_str()));
        if (conn_timeout_ms != 0) {
            DLOG(INFO) << "set connection timeout: " << conn_timeout_ms;
            sock->setConnTimeout(conn_timeout_ms);
        }
        if (recv_timeout_ms != 0) {
            DLOG(INFO) << "set recv timeout: " << recv_timeout_ms;
            sock->setRecvTimeout(recv_timeout_ms);
        }
        cn->socket.reset(sock);
        cn->transport.reset(new TBufferedTransport(cn->socket));
        cn->protocol.reset(new TBinaryProtocol(cn->transport));
        cn->client = new Client(cn->protocol);
        cn->transport->open();

    } catch(std::exception &e) {
        LOG(ERROR) << "unable to create a connection to "
                   << addr->ip << ":" << addr->port;
        delete cn;
        sem_post(&sem); // XXX: 别忘了 increase semaphore
        // print_sem_value(sem, "get exception");
        return nullptr;
    }

    DLOG(INFO) << "successfully connect to " << addr->ip << ":" << addr->port;
    return cn;
}

// ======================================================================
// connection pool implementation
// ======================================================================

template<typename Client>
ThriftConnectionPool<Client>::ThriftConnectionPool(size_t max_try_rounds)
    : max_try_rounds(max_try_rounds)
{
    srand(time(nullptr));
}

// ----------------------------------------------------------------------

template<typename Client>
ThriftConnectionPool<Client>::~ThriftConnectionPool()
{
    for (unsigned i = 0; i < conn_queues.size(); ++i) {
        delete conn_queues[i];
    }
}

// ----------------------------------------------------------------------

template<typename Client>
void ThriftConnectionPool<Client>::set_addr_pool(
        const std::vector<ServerAddress> &addrs, size_t max_nconns_per_server,
        unsigned recv_timeout_ms, unsigned conn_timeout_ms)
{
    DLOG(INFO) << "max_nconns: " << max_nconns_per_server;
    for (unsigned i = 0; i < addrs.size(); ++i) {
        this->addrs.push_back(addrs[i]);
    }
    for (unsigned i = 0; i < addrs.size(); ++i) {
        auto *q = new ConnectionQueue(max_nconns_per_server, &this->addrs[i],
                                      recv_timeout_ms, conn_timeout_ms);
        conn_queues.push_back(q);
    }
}

// ----------------------------------------------------------------------

template<typename Client>
void ThriftConnectionPool<Client>::set_addr_pool(
        const ServerAddress &addr, size_t max_nconns_per_server,
        unsigned recv_timeout_ms, unsigned conn_timeout_ms)
{
    this->addrs.push_back(addr);
    auto *q = new ConnectionQueue(max_nconns_per_server, &this->addrs[0],
                                  recv_timeout_ms, conn_timeout_ms);
    conn_queues.push_back(q);
}

// ----------------------------------------------------------------------

template<typename Client>
template<typename ...Args_t>
int ThriftConnectionPool<Client>::handle(
        void (Client::*handle)(Args_t...), Args_t&&... args)
{
    int err = 0;

    for (auto try_rounds = 0u; try_rounds < max_try_rounds; ++try_rounds) {
        auto cn = get_conn();
        if (cn.second == nullptr) {
            continue;
        }
        try {
            (cn.second->client->*handle)(std::forward<Args_t>(args)...);
            // 如正常完成任务，则将链接归还
            conn_queues[cn.first]->add(cn.second);
            break;

        } catch(std::exception &e) {
            LOG(ERROR) << "get exception " << e.what()
                       << ", release connection and get new one";
            delete cn.second;
            // 非正常情况，也要调用 add 返回，只不过参数是 nullptr
            conn_queues[cn.first]->add(nullptr);
        }
    }

    return err;
}

// ----------------------------------------------------------------------

// 这个函数尝试遍历所有的 queue 以得到一个可用的链接，如果成功，则返回队列号
// 及相应的链接
template<typename Client>
std::pair<unsigned, typename ThriftConnectionPool<Client>::Connection *>
ThriftConnectionPool<Client>::get_conn()
{
    auto base = rand() % conn_queues.size();
    DLOG(INFO) << "poll connection from " << base << "/" << conn_queues.size();
    DLOG(INFO) << "try connection from " << base;

    // 每一轮都遍历所有的 queue，尝试获取一个链接
    for (auto rounds = 0u; rounds < max_try_rounds; ++rounds) {
        DLOG(INFO) << "try rounds: " << rounds;

        // try to get a connection from a random server
        for (auto i = 0u; i < conn_queues.size(); ++i) {
            auto idx = (i+base) % conn_queues.size();
            auto *cn = conn_queues[idx]->pop();
            if (cn != nullptr) {
                DLOG(INFO) << "get connection from "
                           << addrs[idx].ip << ":" << addrs[idx].port;
                return std::pair<unsigned, Connection *>(idx, cn);
            }
        }
        // 必须加个判断，否则会有一次不必要的 sleep
        if (rounds != max_try_rounds-1) {
            DLOG(INFO) << "sleep for a while";
            sleep(0.1 * (1 << rounds));
        }
    }

    return std::make_pair(-1, nullptr);
}

}

#endif
