#include "comms.h"

using asio::ip::tcp;

// Implementations of class Connection

std::shared_ptr<Connection> Connection::create(asio::io_context &io_context,
                                               epsp_role_t role) {
    return std::shared_ptr<Connection>(new Connection(io_context, role));
}

Connection::Connection(asio::io_context &io_context, epsp_role_t role)
    : socket_(io_context), role_(role) {}
auto Connection::socket() -> asio::ip::tcp::socket & { return socket_; }
auto Connection::role() -> epsp_role_t { return role_; }
void Connection::start() { do_read(); }

void Connection::do_read() {
    auto self(shared_from_this());
    asio::async_read_until(
        socket_, buffer_, '\n',
        [this, self](std::error_code ecode, std::size_t) -> void {
            if (ecode) {
                std::cerr << "Read error: " << ecode.message() << "\n";
                return;
            }

            std::istream input(&buffer_);
            std::string line;
            std::getline(input, line);
            std::cout << "Received: " << line << "\n";

            if (line.size() < 5) {
                std::cout << "Invalid message";
                return;
            }

            handle_message(line);
        }

    );
}

void Connection::handle_message(std::string line) {
    if (line.back() == '\r') {
        line.pop_back();
    }

    uint16_t code = std::stoul(line.substr(0, 3));
    uint16_t hop = std::stoul(line.substr(4, 1));
    std::string data;
    if (line.size() > 6) {
        data = line.substr(6);
    }

    std::string response;

    if (200 <= code && code < 300) {
        if (code ==
            std::to_underlying(epsp_server_code_t::EPSP_SERVER_PRTL_QRY)) {
            response = std::to_string(std::to_underlying(
                           epsp_client_code_t::EPSP_CLIENT_PRTL_VER)) +
                       " 1 0." + EPSP_PROTOCOL_VER + ':' + EPSP_CLIENT_NAME +
                       ':' + EPSP_CLIENT_VER;
        }
    }

    response += "\r\n";
    do_write(response);
}

void Connection::do_write(std::string data) {
    auto self(shared_from_this());
    asio::async_write(socket_, asio::buffer(data),
                      [this, self](std::error_code ecode, std::size_t) -> void {
                          if (ecode) {
                              std::cerr << "Write error: " << ecode.message()
                                        << "\n";
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
                        [io_context, server](std::error_code ecode,
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
