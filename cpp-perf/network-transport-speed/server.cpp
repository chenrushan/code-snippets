#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

// ======================================================================
// 这个程序实现了一个这样的 websocket server 框架
// 1. server 端接受请求 --> 触发 on_message 调用 --> on_message 将 message
//    加入一个 message pool
// 2. 另外有一个独立的 thread，不断从 message pool 中取 message 并不断分给
//    worker thread 打印
// ======================================================================

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

#include <chrono>

void on_message(websocketpp::connection_hdl hdl, const std::string &msg)
{
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto nanos = duration_cast<nanoseconds>(now.time_since_epoch()).count();
    std::cout << "server time: " << nanos << std::endl;
    std::cout << msg[0] << msg[1] << msg[2] << "..." << std::endl;
}

// ----------------------------------------------------------------------

int main(int argc, char *argv[])
{
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;

    try {
        WebsocketServer server(16868, on_message);
        server.start();
    } catch(...) {
        std::cerr << "caught exception" << std::endl;
    }

    return 0;
}

