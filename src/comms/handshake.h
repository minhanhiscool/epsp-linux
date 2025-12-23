#pragma once
#include "comms.h"
#include "message.h"
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>

// classes - a nice way to headbash developers!

class Connection : public std::enable_shared_from_this<Connection> {
public:
    static auto create(asio::io_context &io_context,
                       epsp_role_t role = epsp_role_t::UNKNOWN)
        -> std::shared_ptr<Connection>;

    auto socket() -> asio::ip::tcp::socket &;
    auto role() -> epsp_role_t;

    void start();
    void stop();

private:
    explicit Connection(asio::io_context &io_context,
                        epsp_role_t role = epsp_role_t::UNKNOWN);
    States states_;
    asio::ip::tcp::socket socket_;
    epsp_role_t role_;
    asio::streambuf buffer_;

    void do_read();
    void handle_message(std::string line);
    void do_write(std::string data);
};
void init_connection(const std::string &ip_address);
