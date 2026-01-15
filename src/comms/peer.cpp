#include "peer.h"
#include "message.h"
#include <asio/connect.hpp>
#include <asio/error_code.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>
using asio::ip::tcp;

auto ConnectionPeer::create(asio::io_context &io_context)
    -> std::shared_ptr<ConnectionPeer> {
    return std::shared_ptr<ConnectionPeer>(new ConnectionPeer(io_context));
}

ConnectionPeer::ConnectionPeer(asio::io_context &io_context)
    : peer_logger_(spdlog::default_logger()->clone("\033[34mpeer\033[0m")),
      io_context_(&io_context), acceptor_(io_context) {};

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
    auto peer = std::make_shared<Peer>(*io_context_, shared_from_this());

    peer->endpoint = socket.remote_endpoint();
    peer->socket = std::move(socket);
    peer->state = epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED;

    peers_pending_.emplace(std::move(peer));
}

auto ConnectionPeer::start(const uint32_t &target_id,
                           const asio::ip::tcp::endpoint &endpoint) -> bool {
    auto self(shared_from_this());
    auto peer = std::make_shared<Peer>(*io_context_, shared_from_this());

    peer->endpoint = endpoint;
    peer->state = epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PID_RQST;

    asio::error_code ecode;

    asio::connect(peer->socket, std::array<tcp::endpoint, 1>{endpoint}, ecode);

    if (ecode) {
        self->peer_logger_->error("Connect error: {}", ecode.message());
        return false;
    }

    self->peers_.emplace(target_id, std::move(peer));
    return true;
}

void ConnectionPeer::write_broad(const Peer &from_peer,
                                 std::string_view message) {
    for (auto &peer : peers_) {
        if (peer.second->state !=
            epsp_state_peer_t::EPSP_STATE_PEER_CONNECTED) {
            continue;
        }
        if (peer.second->endpoint == from_peer.endpoint) {
            continue;
        }
        peer.second->write_uni(message);
    }
}
ConnectionPeer::Peer::Peer(asio::io_context &io_context,
                           const std::shared_ptr<ConnectionPeer> &parent)
    : parent(parent), socket(io_context) {}

void ConnectionPeer::Peer::read() {
    auto self(shared_from_this());
    asio::async_read_until(
        socket, buffer, '\n',
        [self](asio::error_code ecode, std::size_t) -> void {
            if (ecode) {
                if (auto shared_parent = self->parent.lock()) {
                    shared_parent->peer_logger_->error(
                        "Read error: {}, from: {}", ecode.message(),
                        self->endpoint.address().to_string() + ":" +
                            std::to_string(self->endpoint.port()));
                }
                return;
            }

            std::istream input(&self->buffer);
            std::string line;
            std::getline(input, line);

            if (auto shared_parent = self->parent.lock()) {
                shared_parent->peer_logger_->info(
                    "Received: {}, from: {}", line,
                    self->endpoint.address().to_string() + ":" +
                        std::to_string(self->endpoint.port()));
            }

            if (line.size() < 5) {
                if (auto shared_parent = self->parent.lock()) {
                    shared_parent->peer_logger_->error(
                        "Invalid message: {}, from: {}", line,
                        self->endpoint.address().to_string() + ":" +
                            std::to_string(self->endpoint.port()));
                }
                // stop();
                return;
            }

            self->handle_message(line);

            self->read();
        });
}

void ConnectionPeer::Peer::handle_message(std::string &response) {
    auto self(shared_from_this());
    std::optional<PeerStates::PeerReply> message_struct;
    if (auto shared_parent = parent.lock()) {
        message_struct =
            shared_parent->states_.handle_message(response, self->state);
    }

    if (!message_struct.has_value()) {
        return;
    }

    std::string message = std::to_string(message_struct.value().code) + " " +
                          std::to_string(message_struct.value().hop) + " " +
                          message_struct.value().payload + "\r\n";
    if (message_struct.value().target == epsp_peer_target_t::TARGET_UNICAST) {
        write_uni(message);
    } else if (message_struct.value().target ==
               epsp_peer_target_t::TARGET_BROADCAST) {
        if (auto shared_parent = parent.lock()) {
            shared_parent->write_broad(*self, message);
        }
    }
}

void ConnectionPeer::Peer::write_uni(std::string_view response) {
    auto self(shared_from_this());
    asio::async_write(
        socket, asio::buffer(response),
        [self](asio::error_code ecode, std::size_t) -> void {
            if (ecode) {
                if (auto shared_parent = self->parent.lock()) {
                    shared_parent->peer_logger_->error(
                        "Write error: {}, to: {}", ecode.message(),
                        self->endpoint.address().to_string() + ":" +
                            std::to_string(self->endpoint.port()));
                }
            }
        });
}

auto init_peer_connection() -> PeerInit {
    auto peer_io_context = std::make_shared<asio::io_context>();
    auto peer = ConnectionPeer::create(*peer_io_context);
    PeerInit init_return = {.connection_peer = peer,
                            .io_context = peer_io_context};
    return init_return;
}
