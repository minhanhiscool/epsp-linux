#include "../src/comms/message.h"
#include "../src/comms/comms.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Process Server Protocol Query", "[comms][message]") {
    ServerStates states(epsp_state_server_t::EPSP_STATE_DISCONNECTED);

    std::string message = "211 1\r\n";
    std::string response = states.handle_message(message);

    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PRTL_VER)) +
                            " 1 " + std::string(EPSP_PROTOCOL_VER) + ":" +
                            std::string(EPSP_CLIENT_NAME) + ":" +
                            std::string(EPSP_CLIENT_VER) + "\r\n");
}

TEST_CASE("Process Server Protocol Response", "[comms][message]") {
    ServerStates states(epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PRTL_RET);

    std::string message = "212 1 0.35:P2PDemo:0.0\r\n";
    std::string response = states.handle_message(message);

    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PID_TEMP)) +
                            " 1\r\n");
}

TEST_CASE("Process Server Temp ID Return", "[comms][message]") {
    reset_peer_id();
    ServerStates states(epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PID_TEMP);

    std::string message = "233 1 2011\r\n";
    std::string response = states.handle_message(message);

    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PORT_CHK)) +
                            " 1 2011:6911\r\n");
}

TEST_CASE("Process Server Port Return", "[comms][message]") {
    reset_peer_id();
    ServerStates states(epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PORT_RET);
    if (!has_session_id()) {
        peer_id.store(2011, std::memory_order_relaxed);
    }

    std::string message = "234 1 1\r\n";
    std::string response = states.handle_message(message);

    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PEER_QRY)) +
                            " 1 2011\r\n");
}

TEST_CASE("Process invalid/unimplemented code", "[comms][message]") {
    ServerStates states(epsp_state_server_t::EPSP_STATE_ACTIVE);

    std::string message = "299 1\r\n";
    std::string response = states.handle_message(message);
    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_END_SESS)) +
                            " 1\r\n");
}

TEST_CASE("Process Connection Stop", "[comms][message]") {
    reset_peer_id();
    ServerStates states(epsp_state_server_t::EPSP_STATE_DISCONNECTED);

    std::string message = "239 1\r\n";
    std::string response = states.handle_message(message);
    REQUIRE(response == "stop");
}

TEST_CASE("Run Full Handshake", "[comms][message]") {
    reset_peer_id();
    ServerStates states(epsp_state_server_t::EPSP_STATE_DISCONNECTED);
    std::string message;
    std::string response;

    message = "211 1\r\n";
    response = states.handle_message(message);
    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PRTL_VER)) +
                            " 1 " + std::string(EPSP_PROTOCOL_VER) + ":" +
                            std::string(EPSP_CLIENT_NAME) + ":" +
                            std::string(EPSP_CLIENT_VER) + "\r\n");

    message = "212 1 0.35:P2PDemo:0.0\r\n";
    response = states.handle_message(message);
    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PID_TEMP)) +
                            " 1\r\n");
    message = "233 1 2011\r\n";
    response = states.handle_message(message);
    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_PORT_CHK)) +
                            " 1 2011:6911\r\n");
    message = "298 1\r\n";
    response = states.handle_message(message);
    REQUIRE(response == std::to_string(std::to_underlying(
                            epsp_client_code_t::EPSP_CLIENT_END_SESS)) +
                            " 1\r\n");
    message = "239 1\r\n";
    response = states.handle_message(message);
    REQUIRE(response == "stop");
}
