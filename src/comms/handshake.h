#pragma once
#include "message.h"
#include "peer.h"
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>

class ConnectionServer : public std::enable_shared_from_this<ConnectionServer> {
public:
    static auto create(asio::io_context &io_context,
                       const std::shared_ptr<ConnectionPeer> &peer_manager)
        -> std::shared_ptr<ConnectionServer>;

    auto socket() -> asio::ip::tcp::socket &;

    void start();
    void stop();

private:
    explicit ConnectionServer(asio::io_context &io_context,
                              std::shared_ptr<ConnectionPeer> peer_manager);
    ServerStates states_;
    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
    std::shared_ptr<spdlog::logger> server_logger_;

    void do_read();
    void handle_message(std::string &line);
    void do_write(std::string data);
};
auto init_server_connection(const std::string &ip_address,
                            const std::shared_ptr<ConnectionPeer> &peer_manager)
    -> std::shared_ptr<asio::io_context>;
