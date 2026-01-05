#pragma once
#include "comms.h"
#include "message.h"
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <spdlog/spdlog.h>

class ConnectionServer : public std::enable_shared_from_this<ConnectionServer> {
public:
    static auto create(asio::io_context &io_context)
        -> std::shared_ptr<ConnectionServer>;

    auto socket() -> asio::ip::tcp::socket &;

    void start();
    void stop();

private:
    explicit ConnectionServer(asio::io_context &io_context);
    ServerStates states_;
    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
    std::shared_ptr<spdlog::logger> server_logger_;

    void do_read();
    void handle_message(std::string &line);
    void do_write(std::string data);
};
auto init_server_connection(const std::string &ip_address)
    -> std::shared_ptr<asio::io_context>;
