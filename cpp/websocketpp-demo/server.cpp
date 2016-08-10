#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <iostream>

class WebsocketServer {
    using connection_hdl = websocketpp::connection_hdl;
    using websocket_server = websocketpp::server<websocketpp::config::asio>;
    using message_ptr = websocket_server::message_ptr;

public:
    using on_message_callback = std::function<void(const std::string &msg)>;

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

    void start() {
        server_.listen(port_);
        // Start the server accept loop
        server_.start_accept();
        // Start the ASIO io_service run loop
        server_.run();
    }

    void send(const std::string &msg) {
        server_.send(hdl_, msg, websocketpp::frame::opcode::text);
    }

private:

    void on_open(connection_hdl hdl) { hdl_ = hdl; }

    void on_message(connection_hdl hdl, message_ptr msg) {
        msg_cb_(msg->get_payload());
    }

private:

    websocket_server server_;
    int port_;
    on_message_callback msg_cb_;
    connection_hdl hdl_;
};

void on_message(const std::string &msg)
{
    std::cout << "received " << msg << std::endl;
}

int main(int argc, char *argv[])
{
    WebsocketServer server(16868, on_message);

    server.start();

    return 0;
}

