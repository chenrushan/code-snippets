#include "gen-cpp/HelloSevice.h"
#include "tserver.h"

using namespace util;

class HelloEngine : public HelloSeviceIf {
public:
    void say_hello(Response &resp, const Request &req) {
        char buf[256];
        for (auto i = 0; i < req.msg_size-1 && i < 256; ++i) {
            buf[i] = 'A';
        }
        buf[req.msg_size-1] = 0;
        std::cout << "receive garbage: " << req.garbage << std::endl;
        resp.msg = buf;
        resp.garbage = req.garbage;
    }

};

class HelloFactory : public HelloSeviceIfFactory {
public:
    HelloSeviceIf *getHandler(const apache::thrift::TConnectionInfo &conn) {
        return (HelloSeviceIf *)(new HelloEngine());
    }

    void releaseHandler(HelloSeviceIf *eg) {
        delete eg;
    }
};

int main(int argc, char *argv[])
{
    MultiThreadedThriftServerConfig conf;
    conf.num_threads = 10;
    conf.port = 19999;

    MultiThreadedThriftServer<HelloFactory, HelloSeviceProcessorFactory> server(conf);
    server.serve();

    return 0;
}

