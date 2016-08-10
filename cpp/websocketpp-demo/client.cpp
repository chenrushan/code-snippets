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
    using callback = std::function<void(const std::string &)>;

    WebSocketClient(const std::string uri, callback msg_cb)
        : uri_(move(uri)), msg_cb_(msg_cb), is_opened_(false) {
        // Set logging level
        client_.set_access_channels(websocketpp::log::alevel::fail);
        client_.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        client_.init_asio();

        // Register our message handler
        client_.set_fail_handler(::bind(&WebSocketClient::on_fail, this, ::_1));
        client_.set_open_handler(::bind(&WebSocketClient::on_open, this, ::_1));
        client_.set_message_handler(
                ::bind(&WebSocketClient::on_message, this, ::_1, ::_2));
    }

    // if connection is not open, client_.send() will throw exception，
    // so no need to check is_opened_ before sending
    int send(const std::string &msg) {
        websocketpp::lib::error_code ec;
        if (!is_opened_) {
            return -1;
        }
        client_.send(hdl_, msg, websocketpp::frame::opcode::text, ec);
        if (ec) {
            return -1;
        }
        return 0;
    }

    // once connection is established, asio run loop will not return, so
    // asio loop should be run in a non-master thread
    void run_connection_in_new_thread() {
        auto f = std::bind(&WebSocketClient::inifinite_connection_loop, this);
        std::thread(f).detach();
    }

    // TODO: use condition variable
    void wait_until_connection_open() {
        while (!is_opened_) {
            // TODO: hardcode 1s
            sleep(1);
        }
    }

    std::string id() {
        std::ostringstream ss;
        ss << hdl_.lock().get();
        return ss.str();
    }

private:

    void on_open(connection_hdl hdl) { is_opened_ = true; }
    void on_fail(connection_hdl hdl) {}
    void on_message(connection_hdl hdl, message_ptr msg) {
        msg_cb_(msg->get_payload());
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

void message_handler(const std::string &msg)
{
    std::cout << "received message" << msg << std::endl;
}

int main(int argc, char *argv[])
{
    std::string uri = "ws://127.0.0.1:16868";

    WebSocketClient client(uri, message_handler);
    client.run_connection_in_new_thread();
    client.wait_until_connection_open();

    std::string msg = "hello, this is dinosaur " + client.id();
    while (true) {
        std::cerr << "send " << msg << std::endl;
        client.send(msg);
        sleep(1);
    }

    return 0;
}

