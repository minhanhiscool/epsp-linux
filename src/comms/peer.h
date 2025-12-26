#pragma once
#include "comms.h"
#include "message.h"
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <chrono>

class ConnectionPeer : public std::enable_shared_from_this<ConnectionPeer> {
public:
    static auto create(asio::io_context &io_context)
        -> std::shared_ptr<ConnectionPeer>;

    auto socket() -> asio::ip::tcp::socket &;

    void start();
    void stop();

private:
    struct Peer {
        epsp_state_peer_t state;
        std::string ip_address;
        std::chrono::steady_clock::time_point last_seen;
    };
    std::unordered_map<uint32_t, Peer> peers_;
    explicit ConnectionPeer(asio::io_context &io_context);

    PeerStates states_;
    asio::ip::tcp::socket socket_;
    asio::streambuf buffer_;
};
auto init_peer_connection(const std::string &ip_address)
    -> std::shared_ptr<asio::io_context>;
