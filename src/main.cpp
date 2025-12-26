#include "comms/handshake.h"
#include <asio/connect.hpp>

int main(int argc, char **argv) {
    std::shared_ptr<asio::io_context> server_io_context =
        init_server_connection("localhost");
    std::thread server_thread(
        [server_io_context]() -> void { server_io_context->run(); });

    server_thread.join();
}
