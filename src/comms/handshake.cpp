#include "asio/write.hpp"
#include "include/common.h"
#include "include/comms.h"

using asio::ip::tcp;

std::string respond_from_code(uint16_t code, uint16_t hops,
                              std::string message) {

  std::string response;

  if (200 <= code && code < 300) {
    if (code == epsp_server_code_t::EPSP_SERVER_PRTL_QRY) {
      response = std::to_string(epsp_client_code_t::EPSP_CLIENT_PRTL_VER) +
                 " 1 0." + std::to_string(EPSP_PROTOCOL_VER) + ':' +
                 EPSP_CLIENT_NAME + ':' + EPSP_CLIENT_VER;
    }
  }

  response += "\r\n";
  return response;
}

void read_server(std::shared_ptr<tcp::socket> socket,
                 std::shared_ptr<asio::streambuf> buffer) {

  asio::async_read_until(
      *socket, *buffer, "\n",
      [socket, buffer](std::error_code ec, std::size_t) {
        if (ec) {
          std::cerr << "Read error: " << ec.message() << "\n";
          return;
        }

        std::istream input(buffer.get());
        std::string line;
        std::getline(input, line);
        std::cout << "Received: " << line << "\n";

        if (line.size() < 3) {
          std::cout << "Invalid message";
          return;
        }

        uint16_t code = std::stoul(line.substr(0, 3));
        uint16_t hops = std::stoul(line.substr(4, 1));
        std::string message = line.substr(5);

        std::string response = respond_from_code(code, hops, message);

        asio::async_write(*socket, asio::buffer(response),
                          [socket, buffer](std::error_code ec, std::size_t) {
                            if (ec) {
                              std::cerr << "Write error: " << ec.message()
                                        << "\n";
                              return;
                            }

                            read_server(socket, buffer);
                          });
      });
}

void init_handshake() {
  auto io = std::make_shared<asio::io_context>();
  auto resolver = std::make_shared<tcp::resolver>(*io);
  auto socket = std::make_shared<tcp::socket>(*io);
  auto buffer = std::make_shared<asio::streambuf>();

  auto endpoints = resolver->resolve("localhost", "5555");
  asio::connect(*socket, endpoints);

  read_server(socket, buffer);

  io->run();
}
