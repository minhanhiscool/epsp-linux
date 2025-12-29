#include "peer.h"
#include <asio/connect.hpp>
#include <asio/ip/tcp.hpp>
using asio::ip::tcp;

void ConnectionPeer::start_acceptor() {
    tcp::endpoint endpoint(tcp::v4(), 6911);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    do_accept();
}

void ConnectionPeer::do_accept() {
    auto self(shared_from_this());
    acceptor_.async_accept(
        [self](asio::error_code ecode, tcp::socket socket) -> void {
            if (ecode) {
                spdlog::error("Accept error: {}", ecode.message());
            } else {
                spdlog::info("New connection from {}:{}",
                             socket.remote_endpoint().address().to_string(),
                             socket.remote_endpoint().port());
                self->handle_new_peer(std::move(socket));
            }

            if (self->acceptor_.is_open()) {
                self->do_accept();
            }
        });
}

void ConnectionPeer::handle_new_peer(tcp::socket socket) {
    auto peer = std::make_unique<Peer>(io_context_);

    peer->socket = std::move(socket);
    peer->ip_addr = peer->socket.remote_endpoint().address();
    peer->state = epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED;

    peers_pending_.emplace(std::move(peer));
}

void ConnectionPeer::start(const uint32_t &target_id,
                           const asio::ip::tcp::endpoint &endpoint) {
    auto peer = std::make_unique<Peer>(io_context_);
    auto *raw = peer.get();

    peer->ip_addr = endpoint.address();
    peer->state = epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PID_RQST;
    peers_.emplace(target_id, std::move(peer));

    asio::async_connect(
        raw->socket, std::array<tcp::endpoint, 1>{endpoint},
        [raw](asio::error_code ecode, const tcp::endpoint &) -> void {
            if (ecode) {
                std::cerr << "Connect error: " << ecode.message() << "\n";
                return;
            }
        });

    peers_.emplace(target_id, std::move(peer));
}
