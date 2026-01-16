#include "../src/comms/handshake.h"
#include "../src/comms/peer.h"
#include "asio.hpp"
#include <catch2/catch_test_macros.hpp>
#include <mutex>

TEST_CASE("Connection works", "[connection][network]") {
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(
        io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 6910));

    bool connected = false;
    bool message_received = false;
    std::string received;
    std::condition_variable cv_connect;
    std::condition_variable cv_received;
    std::mutex mutex;

    std::thread server_thread([&] -> void {
        asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);
        {
            std::lock_guard<std::mutex> lock(mutex);
            connected = true;
        }
        cv_connect.notify_one();
        asio::write(socket, asio::buffer("211 1\r\n"));

        asio::streambuf buf;
        asio::read_until(socket, buf, '\n');
        std::istream input(&buf);
        {
            std::lock_guard<std::mutex> lock(mutex);
            std::getline(input, received);
            message_received = true;
        }
        cv_received.notify_one();
    });

    auto peer_dummy = init_peer_connection();
    std::shared_ptr<asio::io_context> client_io_context =
        init_server_connection("localhost", peer_dummy.connection_peer);
    std::thread client_thread(
        [client_io_context]() -> void { client_io_context->run(); });

    {
        std::unique_lock<std::mutex> lock(mutex);
        REQUIRE(cv_received.wait_for(lock, std::chrono::seconds(5),
                                     [&] -> bool { return connected; }));
    }

    SECTION("Client connects to server") { SUCCEED("Connected"); }
    SECTION("Client receives correct message") {
        std::unique_lock<std::mutex> lock(mutex);
        REQUIRE(cv_connect.wait_for(lock, std::chrono::seconds(5),
                                    [&] -> bool { return message_received; }));
        REQUIRE_FALSE(received.empty());
        REQUIRE(received.starts_with("131"));
    }
    if (server_thread.joinable()) {
        server_thread.join();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
}
