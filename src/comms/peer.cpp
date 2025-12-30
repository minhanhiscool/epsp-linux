#include "peer.h"
#include <asio/connect.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>
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
                self->peer_logger_->error("Accept error: {}", ecode.message());
            } else {
                self->peer_logger_->info(
                    "New connection from {}:{}",
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
    auto peer = std::make_shared<Peer>(io_context_, peer_logger_);

    peer->socket = std::move(socket);
    peer->endpoint = socket.remote_endpoint();
    peer->state = epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED;

    peers_pending_.emplace(std::move(peer));
}

void ConnectionPeer::start(const uint32_t &target_id,
                           const asio::ip::tcp::endpoint &endpoint) {
    auto self(shared_from_this());
    auto peer = std::make_shared<Peer>(io_context_, peer_logger_);
    auto *raw = peer.get();

    peer->endpoint = endpoint;
    peer->state = epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PID_RQST;
    peers_.emplace(target_id, std::move(peer));

    asio::async_connect(
        raw->socket, std::array<tcp::endpoint, 1>{endpoint},
        [self, raw](asio::error_code ecode, const tcp::endpoint &) -> void {
            if (ecode) {
                self->peer_logger_->error("Connect error: {}", ecode.message());
                return;
            }
        });

    peers_.emplace(target_id, std::move(peer));
}

ConnectionPeer::Peer::Peer(asio::io_context &io_context,
                           std::shared_ptr<spdlog::logger> logger)
    : logger(std::move(logger)), socket(io_context) {}

void ConnectionPeer::Peer::read() {
    auto self(shared_from_this());
    asio::async_read_until(
        socket, buffer, '\n',
        [self](asio::error_code ecode, std::size_t) -> void {
            if (ecode) {
                self->logger->error("Read error: {}, from: {}", ecode.message(),
                                    self->endpoint.address().to_string() + ":" +
                                        std::to_string(self->endpoint.port()));
                return;
            }

            std::istream input(&self->buffer);
            std::string line;
            std::getline(input, line);

            self->logger->info("Received: {}, from: {}", line,
                               self->endpoint.address().to_string() + ":" +
                                   std::to_string(self->endpoint.port()));

            self->handle_response(line);
            self->read();
        });
}

void ConnectionPeer::Peer::write(std::string_view response) {
    auto self(shared_from_this());
    asio::async_write(socket, asio::buffer(response),
                      [self](asio::error_code ecode, std::size_t) -> void {
                          if (ecode) {
                              self->logger->error(
                                  "Write error: {}, to: {}", ecode.message(),
                                  self->endpoint.address().to_string() + ":" +
                                      std::to_string(self->endpoint.port()));
                          }
                      });
}
