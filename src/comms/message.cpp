#include "message.h"
#include "comms.h"
#include "peer.h"
#include <charconv> // for Mac clang
#include <cstdint>
#include <optional>
#include <utility>

ServerStates::ServerStates(epsp_state_server_t server_state,
                           std::shared_ptr<ConnectionPeer> peer)
    : server_state_(server_state), peer_(std::move(peer)) {}

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
        server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PRTL_RET;
        return return_epsp_server_prtl_qry();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PRTL_RET) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PRTL_RET) {
        server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PID_TEMP;
        return return_epsp_server_prtl_ret();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PID_TEMP) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PID_TEMP) {
        server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PORT_RET;
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
        server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PEER_DAT;
        return return_epsp_server_port_ret();
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_PEER_DAT) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_WAIT_PEER_DAT) {
        server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_WAIT_KEY_ASGN;
        return return_epsp_server_peer_dat(data);
    }
    if (code == std::to_underlying(epsp_server_code_t::EPSP_SERVER_END_SESS) &&
        server_state_ == epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED) {
        server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED;
        spdlog::info("Server end session");
        return "stop";
    }
    server_state_ = epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED;
    return request_epsp_client_end_sess();
}

auto ServerStates::return_epsp_server_prtl_qry() -> std::string {
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PRTL_VER)) +
           " 1 " + std::string(EPSP_PROTOCOL_VER) + ':' +
           std::string(EPSP_CLIENT_NAME) + ':' + std::string(EPSP_CLIENT_VER) +
           "\r\n";
}

auto ServerStates::return_epsp_server_prtl_ret() -> std::string {
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PID_TEMP)) +
           " 1\r\n";
}

auto ServerStates::return_epsp_server_pid_temp(uint16_t port) -> std::string {
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PORT_CHK)) +
           " 1 " + std::to_string(peer_id) + ":" + std::to_string(port) +
           "\r\n";
}

auto ServerStates::return_epsp_server_port_ret() -> std::string {
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PEER_QRY)) +
           " 1 " + std::to_string(peer_id) + "\r\n";
}

auto ServerStates::return_epsp_server_peer_dat(std::string_view data)
    -> std::string {
    size_t index = 0;
    std::vector<uint32_t> successful_conn;
    while (index < data.size()) {
        size_t colon_pos = data.find(':', index);
        std::string_view peer;
        if (colon_pos == std::string::npos) {
            peer = data.substr(index);
            index = data.size();
        } else {
            peer = data.substr(index, colon_pos - index);
            index = colon_pos + 1;
        }

        size_t comma1 = peer.find(',');
        size_t comma2 = peer.find(',', comma1 + 1);

        if (comma1 == std::string::npos || comma2 == std::string::npos) {
            spdlog::error("Invalid peer data: {}", data);
            return request_epsp_client_end_sess();
        }

        std::string_view ip_str = peer.substr(0, comma1);
        std::string_view port_str =
            peer.substr(comma1 + 1, comma2 - comma1 - 1);
        std::string_view pid_str = peer.substr(comma2 + 1);

        asio::ip::address ip_addr;
        asio::error_code ecode;
        ip_addr = asio::ip::make_address(std::string(ip_str), ecode);
        if (ecode) {
            spdlog::error("Invalid IP: {}", ip_str);
            continue;
        }

        uint16_t port = 0;
        auto [ptr, ec_conv] = std::from_chars(
            port_str.data(), port_str.data() + port_str.size(), port);
        if (ec_conv != std::errc()) {
            spdlog::error("Invalid port: {}", port_str);
            continue;
        }

        uint32_t pid = 0;
        std::from_chars(pid_str.data(), pid_str.data() + pid_str.size(), pid);
        asio::ip::tcp::endpoint endpoint(ip_addr, port);

        if (peer_ && peer_->start(pid, endpoint)) {
            successful_conn.push_back(pid);
        }
    }
    if (successful_conn.size() == 0) {
        return request_epsp_client_end_sess();
    }

    std::string payload;
    for (auto pid : successful_conn) {
        payload += std::to_string(pid) + ":";
    }
    payload.pop_back();
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_PEER_CON)) +
           " 1 " + payload + "\r\n";
}

auto ServerStates::request_epsp_client_end_sess() -> std::string {
    return std::to_string(
               std::to_underlying(epsp_client_code_t::EPSP_CLIENT_END_SESS)) +
           " 1\r\n";
}

PeerStates::PeerStates() = default;

auto PeerStates::handle_message(std::string &line,
                                epsp_state_peer_t &peer_state)
    -> std::optional<PeerReply> {
    if (line.back() == '\r') {
        line.pop_back();
    }

    uint16_t code = std::stoul(line.substr(0, 3));

    size_t pos = line.find(' ', 4);
    if (pos == std::string::npos) {
        spdlog::error("Invalid message: {}", line);
    }
    uint8_t hop = std::stoul(line.substr(4, pos - 4));

    std::string data;
    if (line.size() > 6) {
        data = line.substr(6);
    }

    if (hop >= std::max(10, static_cast<int>(std::sqrt(total_peer)))) {
        return std::nullopt;
    }

    std::optional<PeerReply> message(PeerReply{});
    message->code = code;
    message->hop = hop;
    message->payload = data;

    if (500 <= code && code < 700) {
        return_peer_codes(message, peer_state);
    }

    return message;
}

void PeerStates::return_peer_codes(std::optional<PeerReply> &message,
                                   epsp_state_peer_t &peer_state) {

    if (message->code ==
            std::to_underlying(epsp_peer_code_t::EPSP_PEER_PRTL_REQ) &&
        peer_state == epsp_state_peer_t::EPSP_STATE_PEER_DISCONNECTED) {
        peer_state = epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PRTL_REP;
        return_peer_prtl_req(message);
        return;
    }
    if (message->code ==
            std::to_underlying(epsp_peer_code_t::EPSP_PEER_PRTL_REP) &&
        peer_state == epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PRTL_REP) {
        peer_state = epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PID_REPL;
        return_peer_prtl_rep(message);
        return;
    }
    if (message->code ==
            std::to_underlying(epsp_peer_code_t::EPSP_PEER_PID_RQST) &&
        peer_state == epsp_state_peer_t::EPSP_STATE_PEER_WAIT_PID_RQST) {
        peer_state = epsp_state_peer_t::EPSP_STATE_PEER_CONNECTED;
        uint32_t temp_id = 0;
        if (has_session_id()) {
            temp_id = peer_id.load(std::memory_order_relaxed);
        }
        return_peer_pid_rqst(temp_id, message);
        return;
    }
    message = std::nullopt;
}

void PeerStates::return_peer_prtl_req(std::optional<PeerReply> &message) {
    message->target = epsp_peer_target_t::TARGET_UNICAST;
    message->code = std::to_underlying(epsp_peer_code_t::EPSP_PEER_PRTL_REP);
    message->hop = 1;
    message->payload = std::string(EPSP_PROTOCOL_VER) + ":" +
                       std::string(EPSP_CLIENT_NAME) + ":" +
                       std::string(EPSP_CLIENT_VER);
}
void PeerStates::return_peer_prtl_rep(std::optional<PeerReply> &message) {
    message->target = epsp_peer_target_t::TARGET_UNICAST;
    message->code = std::to_underlying(epsp_peer_code_t::EPSP_PEER_PID_RQST);
    message->hop = 1;
    message->payload = "";
}
void PeerStates::return_peer_pid_rqst(uint32_t pid,
                                      std::optional<PeerReply> &message) {
    message->target = epsp_peer_target_t::TARGET_UNICAST;
    message->code = std::to_underlying(epsp_peer_code_t::EPSP_PEER_PID_REPL);
    message->hop = 1;
    message->payload = std::to_string(pid);
}
