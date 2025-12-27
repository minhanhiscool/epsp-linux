#pragma once
#include "message.h"
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>

struct PeerActions {
    virtual ~PeerActions() = default;

    virtual void start_acceptor() = 0;
    virtual void stop_acceptor() = 0;
};

class ConnectionPeer : public PeerActions,
                       public std::enable_shared_from_this<ConnectionPeer> {
public:
    static auto create(asio::io_context &io_context)
        -> std::shared_ptr<ConnectionPeer>;

    void add_peer(uint32_t peer_id, asio::ip::address ip_addr);
    void remove_peer(uint32_t peer_id);

    void start(uint32_t target_id);
    void stop(uint32_t target_id);

    void start_acceptor() override;
    void stop_acceptor() override;

private:
    struct Peer {
        epsp_state_peer_t state{
            epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED};
        asio::ip::address ip_addr;
        std::chrono::steady_clock::time_point last_seen;

        asio::ip::tcp::socket socket;
        asio::streambuf buffer;

        explicit Peer(asio::io_context &io_context) : socket(io_context) {};
    };
    std::unordered_map<uint32_t, std::unique_ptr<Peer>> peers_;
    explicit ConnectionPeer(asio::io_context &io_context);

    PeerStates states_;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    void do_accept();
    void handle_new_peer(asio::ip::tcp::socket socket);
};
