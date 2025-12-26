#include "handshake.h"
#include "message.h"
#include <asio/connect.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>
using asio::ip::tcp;

// Implementations of class ConnectionServer

auto ConnectionServer::create(asio::io_context &io_context)
    -> std::shared_ptr<ConnectionServer> {
    return std::shared_ptr<ConnectionServer>(new ConnectionServer(io_context));
}

ConnectionServer::ConnectionServer(asio::io_context &io_context)
    : states_(epsp_state_server_t::EPSP_STATE_DISCONNECTED),
      socket_(io_context) {}
auto ConnectionServer::socket() -> asio::ip::tcp::socket & { return socket_; }
void ConnectionServer::start() { do_read(); }

void ConnectionServer::stop() {
    auto self(shared_from_this());
    asio::post(socket_.get_executor(), [this, self]() -> void {
        asio::error_code ecode;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ecode);
        if (ecode) {
            spdlog::warn("Shutdown error: {}", ecode.message());
        }
        socket_.close(ecode);
        if (ecode) {
            spdlog::error("Close error: {}", ecode.message());
        }
    });
}

void ConnectionServer::do_read() {
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

void ConnectionServer::handle_message(std::string line) {
    std::string response = states_.handle_message(line);
    if (response == "stop") {
        stop();
        return;
    }
    spdlog::info("Sending: {}", response);
    do_write(response);
}

void ConnectionServer::do_write(std::string data) {
    auto self(shared_from_this());
    asio::async_write(
        socket_, asio::buffer(data),
        [this, self, data](asio::error_code ecode, std::size_t) -> void {
            if (ecode) {
                spdlog::error("Write error: {}", ecode.message());
                stop();
                return;
            }
            do_read();
        });
}

auto init_server_connection(const std::string &ip_address)
    -> std::shared_ptr<asio::io_context> {
    auto server_io_context = std::make_shared<asio::io_context>();
    auto server_resolver = std::make_shared<tcp::resolver>(*server_io_context);
    auto server_endpoints = server_resolver->resolve(ip_address, "6910");
    auto server = ConnectionServer::create(*server_io_context);

    asio::async_connect(
        server->socket(), server_endpoints,
        [server_io_context, server, server_resolver](
            asio::error_code ecode, const tcp::endpoint &) -> void {
            if (ecode) {
                std::cerr << "Connect error: " << ecode.message() << "\n";
                return;
            }
            server->start();
        });

    return server_io_context;
}
