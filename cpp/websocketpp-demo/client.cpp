#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <cstdlib>
#include <atomic>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;
using websocketpp_client = websocketpp::client<websocketpp::config::asio_client>;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;

class WebSocketClient {
public:
    using callback = std::function<void(message_ptr)>;

    WebSocketClient(const std::string uri, callback msg_cb)
        : uri_(move(uri)), msg_cb_(msg_cb), is_opened_(false) {}

    // 如果 connection 没有 open，则 client_.send() 自己会 throw exception，
    // 不需要我在额外判断 is_opened 是否为 true
    void send(const std::string &msg) {
        client_.send(hdl_, msg, websocketpp::frame::opcode::text);
    }

    void run_connection_in_new_thread() {
    }

private:

    void on_open(connection_hdl hdl) {
        is_opened_ = true;
    }

    void on_fail(connection_hdl hdl) {
    }

    void init_client() {
        // Set logging level
        client_.set_access_channels(websocketpp::log::alevel::fail);
        client_.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        client_.init_asio();

        // Register our message handler
        client_.set_fail_handler(::bind(&WebSocketClient::on_fail, this, ::_1));
        client_.set_open_handler(::bind(&WebSocketClient::on_open, this, ::_1));
    }

    void connect_to_server() {
        // in case client_.run() has been invoked before, client_.reset()
        // should be called before executing the following code
        client_.reset();

        // create connection work for asio loop
        websocketpp::lib::error_code ec;
        websocketpp_client::connection_ptr con;
        con = client_.get_connection(uri_, ec);
        if (ec) {
            std::cerr << "could not create connection because: "
                      << ec.message() << std::endl;
            return;
        }
        client_.connect(con);

        // start asio loop. If successful, run() will not return
        client_.run();
    }

    void inifinite_connection_loop() {
        do {
            is_opened_ = false;
            // should not return if successful
            connect_to_server();
            // TODO: hardcode 1s
            sleep(1);
        } while (true);
    }

private:

    std::string uri_;
    websocketpp_client client_;
    callback msg_cb_;
    connection_hdl hdl_;
    std::atomic<bool> is_opened_;

};

