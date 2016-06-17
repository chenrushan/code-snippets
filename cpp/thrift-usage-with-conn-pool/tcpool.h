#ifndef _THRIFT_CONNECTION_POOL_H_
#define _THRIFT_CONNECTION_POOL_H_

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

struct ConnectionConfig {
    size_t max_num_conns=15;
    unsigned recv_timeout_ms=3000;
    unsigned conn_timeout_ms=100;
};

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
template<typename ClientType>
class ThriftConnectionPool {
public:

    ThriftConnectionPool(const ServerAddress &addr,
                         const ConnectionConfig &config=ConnectionConfig(),
                         size_t max_try_rounds=2);
    ThriftConnectionPool(const std::vector<ServerAddress> &addrs,
                         const ConnectionConfig &config=ConnectionConfig(),
                         size_t max_try_rounds=2);
    ~ThriftConnectionPool();

    template<typename... Argst, typename... Argst2>
    int handle(void (ClientType::*handle)(Argst...), Argst2&&... args);

private:

    struct Connection {
        boost::shared_ptr<apache::thrift::transport::TTransport> socket;
        boost::shared_ptr<apache::thrift::transport::TTransport> transport;
        boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol;
        ClientType *client;

        ~Connection() {
            // XXX: 这个可能 throw exception
            try { transport->close(); } catch(...) {}
            delete client;
        }
    };

    // ----------------------------------------------------------------------

    class ConnectionQueue {
    public:
        ConnectionQueue(const ServerAddress &addr, const ConnectionConfig &config);
        ~ConnectionQueue();
        void add(Connection *conn);
        Connection *pop();

    private:
        int sem_value() { int val; sem_getvalue(&sem, &val); return val; }
        Connection *create_new_connection() const;

    private:
        const ServerAddress addr;
        const ConnectionConfig config;
        // connectionq queue
        std::queue<Connection *> queue;
        // 由于允许多个 thread 同时访问队列，而队列的修改不能同时有多个
        // thread，因此当一个 thread 通过了 sem 的测试后，仍需要一个 mutex
        // 来保护队列的修改
        std::mutex mutex;
        // sem 用于控制最多并发链接数
        sem_t sem;

        friend class ThriftConnectionPool;
    };

private:

    std::pair<unsigned, Connection *> get_conn();
    void return_conn(const Connection *conn);
    void release_conn(Connection *conn);

private:

    size_t max_try_rounds;

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
template<typename ClientType>
ThriftConnectionPool<ClientType>::ConnectionQueue::ConnectionQueue(
        const ServerAddress &addr, const ConnectionConfig &config)
    : addr(addr), config(config)
{
    sem_init(&sem, 0, config.max_num_conns);
}

// ----------------------------------------------------------------------

template<typename ClientType>
ThriftConnectionPool<ClientType>::ConnectionQueue::~ConnectionQueue()
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
template<typename ClientType>
void ThriftConnectionPool<ClientType>::ConnectionQueue::add(
        ThriftConnectionPool::Connection *conn)
{
    DLOG(INFO) << "add a " << (conn == nullptr ? "nullptr" : "normal")
               << " connection to queue";
    if (conn != nullptr) {
        std::lock_guard<std::mutex> guard(mutex);
        queue.push(conn);
    }
    sem_post(&sem);
    DLOG(INFO) << "add sem: " << sem_value();
}

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// TODO: 貌似不管 server 是否限制链接个数，以下连接都能成功，但是
// 只有指定个数的链接是可以返回结果的，这个比较恶心，连是连上了，结果
// 不能用，但是又没有一个很好的方法判断一个链接是否是有效的，同时你又
// 没有什么办法知道 server 还有多少链接是可用的
// 
// 这样的结果就是你返回的链接可能是不能用的，现在只能暂时通过 timeout
// 的方法缓解一下，因为不能用的链接是等不到结果的
// ----------------------------------------------------------------------
template<typename ClientType>
typename ThriftConnectionPool<ClientType>::Connection *
ThriftConnectionPool<ClientType>::ConnectionQueue::create_new_connection() const
{
    using TSocket = apache::thrift::transport::TSocket;
    using TBufferedTransport = apache::thrift::transport::TBufferedTransport;
    using TBinaryProtocol = apache::thrift::protocol::TBinaryProtocol;

    std::unique_ptr<Connection> cn(new Connection());

    std::unique_ptr<TSocket> sock(new TSocket(addr.ip, atoi(addr.port.c_str())));
    if (config.conn_timeout_ms != 0) {
        DLOG(INFO) << "set connection timeout: " << config.conn_timeout_ms;
        sock->setConnTimeout(config.conn_timeout_ms);
    }
    if (config.recv_timeout_ms != 0) {
        DLOG(INFO) << "set recv timeout: " << config.recv_timeout_ms;
        sock->setRecvTimeout(config.recv_timeout_ms);
    }
    cn->socket.reset(sock.release());
    cn->transport.reset(new TBufferedTransport(cn->socket));
    cn->protocol.reset(new TBinaryProtocol(cn->transport));
    cn->client = new ClientType(cn->protocol);
    cn->transport->open();
    return cn.release();
}

// ----------------------------------------------------------------------

// 从 queue 中取出一个链接
// 1. 首先判断是不是已经达到链接上限了，如果是，则返回失败
// 2. 然后看看 queue 有没有空闲的链接，有则取出并返回
// 3. 没有空闲链接的情况下创建一个，如果失败，则返回失败
template<typename ClientType>
typename ThriftConnectionPool<ClientType>::Connection *
ThriftConnectionPool<ClientType>::ConnectionQueue::pop()
{
    DLOG(INFO) << "request a connection";

    // don't block, just return and try next server
    if (sem_trywait(&sem) != 0 && errno == EAGAIN) {
        LOG(ERROR) << "no connection available on " << addr.ip << ":" << addr.port;
        return nullptr;
    }
    DLOG(INFO) << "wait sem: " << sem_value();

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
    try {
        cn = create_new_connection();
    } catch(std::exception &e) {
        LOG(ERROR) << "unable to create a connection to "
                   << addr.ip << ":" << addr.port;
        sem_post(&sem); // XXX: 别忘了 increase semaphore
        DLOG(INFO) << "get exception sem: " << sem_value();
        return nullptr;
    }

    DLOG(INFO) << "successfully connect to " << addr.ip << ":" << addr.port;
    return cn;
}

// ======================================================================
// connection pool implementation
// ======================================================================

template<typename ClientType>
ThriftConnectionPool<ClientType>::ThriftConnectionPool(
        const ServerAddress &addr, const ConnectionConfig &config,
        size_t max_try_rounds)
    : max_try_rounds(max_try_rounds)
{
    conn_queues.push_back(new ConnectionQueue(addr, config));
    srand(time(nullptr));
}

// ----------------------------------------------------------------------

template<typename ClientType>
ThriftConnectionPool<ClientType>::ThriftConnectionPool(
        const std::vector<ServerAddress> &addrs, const ConnectionConfig &config,
        size_t max_try_rounds)
    : max_try_rounds(max_try_rounds)
{
    for (const auto &addr : addrs) {
        conn_queues.push_back(new ConnectionQueue(addr, config));
    }
    srand(time(nullptr));
}

// ----------------------------------------------------------------------

template<typename ClientType>
ThriftConnectionPool<ClientType>::~ThriftConnectionPool()
{
    for (unsigned i = 0; i < conn_queues.size(); ++i) {
        delete conn_queues[i];
    }
}

// ----------------------------------------------------------------------

template<typename ClientType>
template<typename... Argst, typename... Argst2>
int ThriftConnectionPool<ClientType>::handle(
        void (ClientType::*handle)(Argst...), Argst2&&... args)
{
    int err = 0;

    for (auto try_rounds = 0u; try_rounds < max_try_rounds; ++try_rounds) {
        auto cn = get_conn();
        if (cn.second == nullptr) {
            continue;
        }
        try {
            (cn.second->client->*handle)(std::forward<Argst2>(args)...);
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
template<typename ClientType>
std::pair<unsigned, typename ThriftConnectionPool<ClientType>::Connection *>
ThriftConnectionPool<ClientType>::get_conn()
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
                DLOG(INFO) << "get connection from " << conn_queues[idx]->addr.ip
                           << ":" << conn_queues[idx]->addr.port;
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
