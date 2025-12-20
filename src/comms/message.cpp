#include "message.h"

States::States(epsp_state_server_t server_state)
    : server_state_(server_state), id_(-1) {}

auto States::handle_message(std::string &line) -> std::string {
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
        response = return_server_codes(code, data);
    }

    return response;
}

auto States::return_server_codes(uint16_t code, std::string &data)
    -> std::string {
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PRTL_QRY) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_DISCONNECTED) {
        return return_epsp_server_prtl_qry();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PRTL_RET) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PRTL_RET) {
        return return_epsp_server_prtl_ret();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PID_TEMP) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PID_TEMP) {
        id_ = std::stoul(data);
        return return_epsp_server_pid_temp(6911);
    }
}

auto States::return_epsp_server_prtl_qry() -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PRTL_RET;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PRTL_VER)) +
           " 1 " + EPSP_PROTOCOL_VER + ':' + EPSP_CLIENT_NAME + ':' +
           EPSP_CLIENT_VER + "\r\n";
}

auto States::return_epsp_server_prtl_ret() -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PID_TEMP;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PID_TEMP)) +
           " 1\r\n";
}

auto States::return_epsp_server_pid_temp(uint16_t port) -> std::string {
    server_state_ = epsp_state_server_t::EPSP_STATE_WAIT_SERVER_PORT_RET;
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PORT_CHK)) +
           " 1 " + std::to_string(id_) + ":" + std::to_string(port) + "\r\n";
}
