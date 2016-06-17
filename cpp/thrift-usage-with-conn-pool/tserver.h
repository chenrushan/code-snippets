#ifndef _THRIFT_SERVER_H_
#define _THRIFT_SERVER_H_

#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>

namespace util {
    
struct MultiThreadedThriftServerConfig {
    int num_threads = 0;
    int port = 0;
};

template<typename ServerFactoryType, typename ServerProcessorFactoryType>
class MultiThreadedThriftServer {
public:
    MultiThreadedThriftServer(const MultiThreadedThriftServerConfig &conf)
        : conf_(conf) {
        if (conf.num_threads == 0 || conf.port == 0) {
            throw "num_threads and port should not be 0";
        }
    }

    void serve() {
        using namespace apache::thrift;
        using namespace apache::thrift::concurrency;
        using namespace apache::thrift::protocol;
        using namespace apache::thrift::transport;
        using namespace apache::thrift::server;

        auto threadManager = ThreadManager::newSimpleThreadManager(conf_.num_threads);
        auto threadFactory = boost::shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
        threadManager->threadFactory(threadFactory);
        threadManager->start();

        boost::shared_ptr<ServerFactoryType> handler(new ServerFactoryType());
        boost::shared_ptr<TProcessorFactory> processor(new ServerProcessorFactoryType(handler));
        boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(conf_.port));
        boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
        boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

        TThreadPoolServer server(processor, serverTransport, transportFactory,
                                 protocolFactory, threadManager);
        server.serve();
    }

private:
    MultiThreadedThriftServerConfig conf_;
};

} // util

#endif
