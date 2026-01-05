#include "message.h"
#include "comms.h"

ServerStates::ServerStates(epsp_state_server_t server_state)
    : server_state_(server_state) {}

auto ServerStates::handle_message(std::string &line) -> std::string {
    if (line.back() == '\r') {
        line.pop_back();
    }

    uint16_t code = std::stoul(line.substr(0, 3));
    std::string data;
    if (line.size() > 6) {
        data = line.substr(6);
    }

    std::string response;

    if (200 <= code && code < 300) {
        response = return_server_codes(code, data);
    }

    return response;
}

auto ServerStates::return_server_codes(uint16_t code, std::string_view data)
    -> std::string {
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PRTL_QRY) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED) {
        return return_epsp_server_prtl_qry();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PRTL_RET) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PRTL_RET) {
        return return_epsp_server_prtl_ret();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PID_TEMP) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PID_TEMP) {
        uint32_t temp_id = 0;
        auto [ptr, errc] =
            std::from_chars(data.data(), data.data() + data.size(), temp_id);
        if (errc == std::errc() && !has_session_id()) {
            peer_id.store(temp_id, std::memory_order_relaxed);
            return return_epsp_server_pid_temp(EPSP_PORT);
        }
        std::error_code ecode = std::make_error_code(errc);
        spdlog::error("Error parsing server temp id: {}", ecode.message());
        return request_epsp_client_end_sess();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PORT_RET) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PORT_RET) {
        return return_epsp_server_port_ret();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PEER_DAT) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PEER_DAT) {
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_END_SESS) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED) {
        spdlog::info("Server end session");
        return "stop";
    }
    return request_epsp_client_end_sess();
}

auto ServerStates::return_epsp_server_prtl_qry() -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PRTL_RET;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PRTL_VER)) +
           " 1 " + std::string(EPSP_PROTOCOL_VER) + ':' +
           std::string(EPSP_CLIENT_NAME) + ':' + std::string(EPSP_CLIENT_VER) +
           "\r\n";
}

auto ServerStates::return_epsp_server_prtl_ret() -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PID_TEMP;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PID_TEMP)) +
           " 1\r\n";
}

auto ServerStates::return_epsp_server_pid_temp(uint16_t port) -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PORT_RET;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PORT_CHK)) +
           " 1 " + std::to_string(peer_id) + ":" + std::to_string(port) +
           "\r\n";
}

auto ServerStates::return_epsp_server_port_ret() -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PEER_DAT;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PEER_QRY)) +
           " 1 " + std::to_string(peer_id) + "\r\n";
}

auto ServerStates::request_epsp_client_end_sess() -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_END_SESS)) +
           " 1\r\n";
}

PeerStates::PeerStates() = default;

auto PeerStates::handle_message(std::string &line, epsp_state_peer_t peer_state)
    -> std::pair<bool, std::string> {
    if (line.back() == '\r') {
        line.pop_back();
    }

    uint16_t code = std::stoul(line.substr(0, 3));

    size_t pos = line.find(' ', 4);
    uint8_t hop = std::stoul(line.substr(4, pos - 4));

    std::string data;
    if (line.size() > 6) {
        data = line.substr(6);
    }

    if (hop >= std::max(10, static_cast<int>(std::sqrt(total_peer)))) {
        return {0, ""};
    }

    std::string response;
    std::string rep_code;
    bool do_write = false;

    if (500 <= code && code < 700) {
        std::tie(do_write, rep_code, response) =
            return_peer_codes(code, response, peer_state);
    }

    if (!do_write) {
        return {false, ""};
    }
    response = rep_code + " " + std::to_string(++hop) + " " + response + "\r\n";
    return {true, response};
}

auto PeerStates::return_peer_codes(uint16_t code, std::string_view data,
                                   epsp_state_peer_t peer_state)
    -> std::tuple<bool, std::string, std::string> {
    return {1, "", ""};
}
