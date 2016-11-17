//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <algorithm>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <set>
#include "chat_message.hpp"

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant {
public:
    virtual ~chat_participant() {}
    virtual void deliver(const chat_message& msg) = 0;
};

typedef boost::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room {
public:
    // 对于一个新来的 participant，room 会把在 recent_msgs_ 中的 message 都
    // 发给它
    void join(chat_participant_ptr participant) {
        participants_.insert(participant);
        std::for_each(recent_msgs_.begin(), recent_msgs_.end(),
                      boost::bind(&chat_participant::deliver, participant, _1));
    }

    void leave(chat_participant_ptr participant) {
        participants_.erase(participant);
    }

    // 将 msg 发给所有 participant
    void deliver(const chat_message& msg) {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs) {
            recent_msgs_.pop_front();
        }

        std::for_each(
            participants_.begin(), participants_.end(),
            boost::bind(&chat_participant::deliver, _1, boost::ref(msg)));
    }

private:
    std::set<chat_participant_ptr> participants_;
    enum { max_recent_msgs = 100 };
    chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

// 一个 chat_session 对应一个 chat_client (一对一)
// 一个 chat_session 属于一个 chat_room (多对一)
class chat_session : public chat_participant,
                     public boost::enable_shared_from_this<chat_session> {
public:
    chat_session(boost::asio::io_service& io_service, chat_room& room)
        : socket_(io_service), room_(room) {}

    tcp::socket& socket() { return socket_; }

    // 一启动就等待 read 一个 header
    void start() {
        room_.join(shared_from_this());
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_msg_.data(), chat_message::header_length),
            boost::bind(&chat_session::handle_read_header, shared_from_this(),
                        boost::asio::placeholders::error));
    }

    // 这里 message deliver 的策略是这样
    // 一个 chat_session 一次只能有一个 async_write 在执行，如果当前的
    // async_write 还没返回，则消息只是被 push 到 write_msgs_ 中，直到
    // async_write 返回了，才会再从 write_msgs_ 中取出一个来 deliver
    //
    // 我感觉这么做应该是为了保证消息的顺序，在上一条消息还没有投递成功的
    // 情况下，下一条消息不会被投递
    void deliver(const chat_message& msg) {
        // 只要 write_msgs_ 不为空，则必然已经有一个 async_write 存在了
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress) {
            boost::asio::async_write(
                socket_, boost::asio::buffer(write_msgs_.front().data(),
                                             write_msgs_.front().length()),
                boost::bind(&chat_session::handle_write, shared_from_this(),
                            boost::asio::placeholders::error));
        }
    }

    void handle_read_header(const boost::system::error_code& error) {
        if (!error && read_msg_.decode_header()) {
            boost::asio::async_read(
                socket_,
                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                boost::bind(&chat_session::handle_read_body, shared_from_this(),
                            boost::asio::placeholders::error));
        } else {
            room_.leave(shared_from_this());
        }
    }

    void handle_read_body(const boost::system::error_code& error) {
        if (!error) {
            room_.deliver(read_msg_);
            boost::asio::async_read(
                socket_, boost::asio::buffer(read_msg_.data(),
                                             chat_message::header_length),
                boost::bind(&chat_session::handle_read_header,
                            shared_from_this(),
                            boost::asio::placeholders::error));
        } else {
            room_.leave(shared_from_this());
        }
    }

    // 成功发给 client 的话，就将 msg 从 write_msgs_ 中删除
    // 如果 write_msgs_ 不为空，则继续调用 async_write
    void handle_write(const boost::system::error_code& error) {
        if (!error) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
                boost::asio::async_write(
                    socket_, boost::asio::buffer(write_msgs_.front().data(),
                                                 write_msgs_.front().length()),
                    boost::bind(&chat_session::handle_write, shared_from_this(),
                                boost::asio::placeholders::error));
            }
        } else {
            room_.leave(shared_from_this());
        }
    }

private:
    tcp::socket socket_;
    chat_room& room_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};

typedef boost::shared_ptr<chat_session> chat_session_ptr;

//----------------------------------------------------------------------

// 一个 chat_server 对应一个 chat_room (一对一)
class chat_server {
public:
    chat_server(boost::asio::io_service& io_service,
                const tcp::endpoint& endpoint)
        : io_service_(io_service), acceptor_(io_service, endpoint) {
        chat_session_ptr new_session(new chat_session(io_service_, room_));
        std::cout << "[chat_server]: start async_accept" << std::endl;
        acceptor_.async_accept(
            new_session->socket(),
            boost::bind(&chat_server::handle_accept, this, new_session,
                        boost::asio::placeholders::error));
    }

    // 没接收一个连接，都会再开启一个等待连接
    void handle_accept(chat_session_ptr session,
                       const boost::system::error_code& error) {
        if (!error) {
            std::cout << "[chat_server]: accepted" << std::endl;
            session->start();
            chat_session_ptr new_session(new chat_session(io_service_, room_));
            std::cout << "[chat_server]: start async_accept" << std::endl;
            acceptor_.async_accept(
                new_session->socket(),
                boost::bind(&chat_server::handle_accept, this, new_session,
                            boost::asio::placeholders::error));
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    chat_room room_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;

//----------------------------------------------------------------------

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
            return 1;
        }

        // 这里所有 chat server 公用一个 io service
        boost::asio::io_service io_service;

        chat_server_list servers;
        for (int i = 1; i < argc; ++i) {
            using namespace std;  // For atoi.
            tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
            chat_server_ptr server(new chat_server(io_service, endpoint));
            servers.push_back(server);
        }

        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
