#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/HelloSevice.h"

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

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
    HelloSeviceIf *getHandler(const TConnectionInfo &conn) {
        return (HelloSeviceIf *)(new HelloEngine());
    }

    void releaseHandler(HelloSeviceIf *eg) {
        delete eg;
    }
};

int main(int argc, char *argv[])
{
    boost::shared_ptr<ThreadManager> threadManager =
        ThreadManager::newSimpleThreadManager(50);
    boost::shared_ptr<PosixThreadFactory> threadFactory =
        boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();

    boost::shared_ptr<HelloFactory> handler(new HelloFactory());
    boost::shared_ptr<TProcessorFactory> processor(new HelloSeviceProcessorFactory(handler));
    boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(19999));
    boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TThreadPoolServer server(processor, serverTransport,
            transportFactory, protocolFactory, threadManager);
    fprintf(stderr, "start engine server ...\n");
    server.serve();
    return 0;
}

