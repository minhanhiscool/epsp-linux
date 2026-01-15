#include "comms/handshake.h"
#include "comms/peer.h"
#include <asio/connect.hpp>

int main(int argc, char **argv) {
    std::shared_ptr<asio::io_context> server_io_context =
        init_server_connection("localhost");
    std::thread server_thread(
        [server_io_context]() -> void { server_io_context->run(); });

    auto peer_io_context = init_peer_connection();
    std::thread peer_thread(
        [peer_io_context]() -> void { peer_io_context.io_context->run(); });

    server_thread.join();
    peer_thread.join();
    return 0;
}
