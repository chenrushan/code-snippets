#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

class WebsocketServer {
    using connection_hdl = websocketpp::connection_hdl;
    using websocket_server = websocketpp::server<websocketpp::config::asio>;
    using message_ptr = websocket_server::message_ptr;

public:
    // 需要知道 message 是来自那个 client 的，因此 callback 中需要有个
    // connection_hdl 的参数
    using on_message_callback =
        std::function<void(connection_hdl, const std::string &msg)>;

    WebsocketServer(int port, on_message_callback msg_cb) 
        : port_(port), msg_cb_(msg_cb) {
        using websocketpp::lib::placeholders::_1;
        using websocketpp::lib::placeholders::_2;

        // Set logging settings
        server_.set_access_channels(websocketpp::log::alevel::all);
        server_.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        server_.init_asio();

        // Register our message handler
        server_.set_message_handler(websocketpp::lib::bind(
                    &WebsocketServer::on_message, this, _1, _2));
    }

    ~WebsocketServer() {
        server_.stop_listening();
    }

    void start() {
        server_.listen(port_);
        // Start the server accept loop
        server_.start_accept();
        // Start the ASIO io_service run loop
        server_.run();
    }

    void send(connection_hdl hdl, const std::string &msg) {
        server_.send(hdl, msg, websocketpp::frame::opcode::text);
    }

private:

    void on_message(connection_hdl hdl, message_ptr msg) {
        msg_cb_(hdl, msg->get_payload());
    }

private:

    websocket_server server_;
    int port_;
    on_message_callback msg_cb_;
    connection_hdl hdl_;
};

// ======================================================================

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

class ThreadPool {
public:
    ThreadPool(size_t nthreads);
    ~ThreadPool();

    template<class F> void execute(F f);

private:
    boost::asio::io_service service;
    boost::thread_group threadpool;
    boost::asio::io_service::work work;
};

ThreadPool::ThreadPool(size_t nthreads)
    : work(service)
{
    for (size_t i = 0; i < nthreads; ++i) {
        threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &service));
    }
}

ThreadPool::~ThreadPool()
{
    service.stop();
    threadpool.join_all();
}

template<class F>
void ThreadPool::execute(F f)
{
    service.post(f);
}

// ======================================================================

#include "objectpool.h"
#include <chrono>

struct Message {
    websocketpp::connection_hdl hdl;
    std::string msg;

    bool operator<(const Message &that) const {
        auto this_hdl = hdl.lock().get();
        auto that_hdl = that.hdl.lock().get();
        if (this_hdl != that_hdl) {
            return this_hdl < that_hdl;
        } else {
            return msg < that.msg;
        }
    }
};

void on_message(websocketpp::connection_hdl hdl, const std::string &msg,
                OneWriterMultiReaderObjectPool<Message> &msg_pool)
{
    std::cout << "received " << msg << std::endl;
    msg_pool.add({hdl, msg});
}

void print_message(OneWriterMultiReaderObjectPool<Message> &msg_pool)
{
    std::cerr << "print_message" << std::endl;
    while (true) {
        msg_pool.for_each([](const Message &msg) {
            std::cout << "***" << msg.msg << " from "
                      << msg.hdl.lock().get() << std::endl;
            return 0;
        });
        std::cout << "==========" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main(int argc, char *argv[])
{
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;

    OneWriterMultiReaderObjectPool<Message> msg_pool;

    std::thread(std::bind(print_message, std::ref(msg_pool))).detach();

    try {
        WebsocketServer server(16868, std::bind(on_message, _1, _2, std::ref(msg_pool)));
        server.start();
    } catch(...) {
        std::cerr << "caught exception" << std::endl;
    }

    return 0;
}

