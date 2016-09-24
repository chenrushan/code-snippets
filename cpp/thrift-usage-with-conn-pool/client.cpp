#include "tcpool.h"
#include "gen-cpp/HelloSevice.h"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace util;

int main(int argc, char *argv[])
{
    if (argc == 1) {
        std::cout << "[USAGE]: cmd <size1> <size2>" << std::endl;
        std::exit(0);
    }
    auto addr = ServerAddress{"127.0.0.1", "19999"};
    ThriftConnectionPool<HelloSeviceClient> conn_pool(addr);

    Request req;
    req.msg_size = std::stoi(argv[1]);
    req.garbage = std::stoi(argv[2]);

    Response resp;
    conn_pool.handle(&HelloSeviceClient::say_hello, resp, req);

    std::cout << resp.msg << " " << resp.garbage << std::endl;
    
    return 0;
}

