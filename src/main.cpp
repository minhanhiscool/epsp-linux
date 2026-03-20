#include "comms/handshake.h"
#include "comms/peer.h"
#include "gui/gui_main.h"
#include <asio/connect.hpp>

const std::shared_ptr<spdlog::logger> main_logger =
    spdlog::default_logger()->clone("\033[31mmain\033[0m");

int main(int /*argc*/, char ** /*argv*/) {
    if (init_gui() == 1) {
        main_logger->info("Failed to init GUI");
        return 1;
    }

    auto peer_io_context = init_peer_connection();
    auto peer_work = asio::make_work_guard(*peer_io_context.io_context);
    std::shared_ptr<asio::io_context> server_io_context =
        init_server_connection("localhost", peer_io_context.connection_peer);

    std::thread server_thread([server_io_context]() -> void {
        main_logger->info("Starting server thread");
        server_io_context->run();
        main_logger->info("Server thread stopped");
    });
    std::thread peer_thread([peer_io_context]() -> void {
        main_logger->info("Starting peer thread");
        peer_io_context.io_context->run();
        main_logger->info("Peer thread stopped");
    });

    gui_loop();
    cleanup_gui();

    server_thread.join();
    peer_work.reset();
    peer_thread.join();
    return 0;
}
