#include "handshake.h"
#include "message.h"
#include <asio/connect.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>
using asio::ip::tcp;

// Implementations of class Connection

auto Connection::create(asio::io_context &io_context, epsp_role_t role)
    -> std::shared_ptr<Connection> {
    return std::shared_ptr<Connection>(new Connection(io_context, role));
}

Connection::Connection(asio::io_context &io_context, epsp_role_t role)
    : states_(epsp_state_server_t::EPSP_STATE_DISCONNECTED),
      socket_(io_context), role_(role) {}
auto Connection::socket() -> asio::ip::tcp::socket & { return socket_; }
auto Connection::role() -> epsp_role_t { return role_; }
void Connection::start() { do_read(); }

void Connection::stop() {
    asio::error_code ecode;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ecode);
    if (ecode) {
        spdlog::warn("Shutdown error: {}", ecode.message());
    }
    socket_.close(ecode);
    if (ecode) {
        spdlog::error("Close error: {}", ecode.message());
    }
}

void Connection::do_read() {
    auto self(shared_from_this());
    asio::async_read_until(
        socket_, buffer_, '\n',
        [this, self](asio::error_code ecode, std::size_t) -> void {
            if (ecode) {
                spdlog::error("Read error: {}", ecode.message());
                return;
            }

            std::istream input(&buffer_);
            std::string line;
            std::getline(input, line);
            spdlog::info("Received: {}", line);

            if (line.size() < 5) {
                spdlog::error("Invalid message: {}", line);
                stop();
                return;
            }

            handle_message(line);
        }

    );
}

void Connection::handle_message(std::string line) {
    std::string response = states_.handle_message(line);
    if (response == "stop") {
        stop();
    }
    do_write(response);
}

void Connection::do_write(std::string data) {
    auto self(shared_from_this());
    asio::async_write(
        socket_, asio::buffer(data),
        [this, self](asio::error_code ecode, std::size_t) -> void {
            if (ecode) {
                spdlog::error("Write error: {}", ecode.message());
                stop();
                return;
            }
            do_read();
        });
}

void init_connection(const std::string &ip_address) {
    auto io_context = std::make_shared<asio::io_context>();
    auto resolver = std::make_shared<tcp::resolver>(*io_context);
    auto endpoints = resolver->resolve(ip_address, "6910");
    auto server = Connection::create(*io_context, epsp_role_t::SERVER);

    asio::async_connect(server->socket(), endpoints,
                        [io_context, server](asio::error_code ecode,
                                             const tcp::endpoint &) -> void {
                            if (ecode) {
                                std::cerr
                                    << "Connect error: " << ecode.message()
                                    << "\n";
                                return;
                            }
                            server->start();
                        });
    io_context->run();
}
