#pragma once
#include "message.h"
#include <asio/io_context.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <memory>

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

    void start(const uint32_t &target_id,
               const asio::ip::tcp::endpoint &endpoint);
    void stop(uint32_t target_id);

    void start_acceptor() override;
    void stop_acceptor() override;

private:
    struct Peer : public std::enable_shared_from_this<Peer> {
        epsp_state_peer_t state{
            epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED};
        std::chrono::steady_clock::time_point last_seen;
        asio::ip::tcp::endpoint endpoint;
        std::shared_ptr<spdlog::logger> logger;

        asio::ip::tcp::socket socket;
        asio::streambuf buffer;

        explicit Peer(asio::io_context &io_context,
                      std::shared_ptr<spdlog::logger> logger);
        void read();
        void handle_response(std::string_view response);
        void write(std::string_view response);
    };
    std::unordered_map<uint32_t, std::shared_ptr<Peer>> peers_;
    std::unordered_set<std::shared_ptr<Peer>> peers_pending_;
    std::shared_ptr<spdlog::logger> peer_logger_;
    explicit ConnectionPeer(asio::io_context &io_context);

    PeerStates states_;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    void do_accept();
    void handle_new_peer(asio::ip::tcp::socket socket);
};
