#include "tcpool.h"
#include "gen-cpp/HelloSevice.h"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace util;

int main(int argc, char *argv[])
{
    ThriftConnectionPool<HelloSeviceClient> conn_pool;

    conn_pool.set_addr_pool(ServerAddress{"127.0.0.1", "19999"});

    Response resp;
    Request req;
    req.msg_size = std::stoi(argv[1]);
    req.garbage = std::stoi(argv[2]);
    conn_pool.handle(&HelloSeviceClient::say_hello, resp, (const Request &)req);

    std::cout << resp.msg << " " << resp.garbage << std::endl;
    
    return 0;
}

