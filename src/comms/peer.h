#pragma once

#include "message.h"
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <cstdint>

class ConnectionPeer : public std::enable_shared_from_this<ConnectionPeer> {
public:
    static auto create(asio::io_context &io_context)
        -> std::shared_ptr<ConnectionPeer>;

    auto start(const uint32_t &target_id,
               const asio::ip::tcp::endpoint &endpoint) -> bool;
    void stop(uint32_t target_id);

    void start_acceptor();
    void stop_acceptor();

    void stop_all();

private:
    struct Peer : public std::enable_shared_from_this<Peer> {
        epsp_state_peer_t state{
            epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED};
        std::weak_ptr<ConnectionPeer> parent;
        std::chrono::steady_clock::time_point last_seen;
        uint32_t peer_id;
        asio::ip::tcp::endpoint endpoint;

        asio::ip::tcp::socket socket;
        asio::streambuf buffer;

        explicit Peer(asio::io_context &io_context,
                      const std::shared_ptr<ConnectionPeer> &parent);

        void read();
        void handle_message(std::string &response);
        void write_uni(std::string_view response);
    };
    friend struct Peer;
    std::unordered_map<uint32_t, std::shared_ptr<Peer>> peers_;
    std::unordered_set<std::shared_ptr<Peer>> peers_pending_;
    std::shared_ptr<spdlog::logger> peer_logger_;
    explicit ConnectionPeer(asio::io_context &io_context);

    PeerStates states_;
    asio::io_context &io_context_;
    asio::ip::tcp::acceptor acceptor_;
    void do_accept();
    void handle_new_peer(asio::ip::tcp::socket socket);
    void write_broad(const Peer &from_peer, std::string_view message);
};

struct PeerInit {
    std::shared_ptr<ConnectionPeer> connection_peer;
    std::shared_ptr<asio::io_context> io_context;
};

auto init_peer_connection() -> PeerInit;
