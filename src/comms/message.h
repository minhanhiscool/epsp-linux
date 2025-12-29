#pragma once
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>

// define codes
enum class epsp_client_code_t : uint8_t {
    EPSP_CLIENT_PID_TEMP = 113,
    EPSP_CLIENT_PORT_CHK = 114,
    EPSP_CLIENT_PEER_QRY = 115,
    EPSP_CLIENT_PID_FINL = 116,
    EPSP_CLIENT_KEY_ASGN = 117,
    EPSP_CLIENT_TIME_REF = 118,
    EPSP_CLIENT_END_SESS = 119,
    EPSP_CLIENT_ECHO_UPD = 123,
    EPSP_CLIENT_KEY_RASG = 124,
    EPSP_CLIENT_PEER_RGN = 127,
    EPSP_CLIENT_END_PART = 128,
    EPSP_CLIENT_PRTL_VER = 131,
    EPSP_CLIENT_PEER_CON = 155,
    EPSP_CLIENT_PRTL_REJ = 192,
};

enum class epsp_server_code_t : uint16_t {
    EPSP_SERVER_PRTL_QRY = 211,
    EPSP_SERVER_PRTL_RET = 212,
    EPSP_SERVER_PID_TEMP = 233,
    EPSP_SERVER_PORT_RET = 234,
    EPSP_SERVER_PEER_DAT = 235,
    EPSP_SERVER_PID_FINL = 236,
    EPSP_SERVER_KEY_ASGN = 237,
    EPSP_SERVER_TIME_REF = 238,
    EPSP_SERVER_END_SESS = 239,
    EPSP_SERVER_ECHO_UPD = 243,
    EPSP_SERVER_KEY_RASG = 244,
    EPSP_SERVER_PEER_RGN = 247,
    EPSP_SERVER_END_PART = 248,
    EPSP_SERVER_ERR_UNKW = 291,
    EPSP_SERVER_PRTL_REJ = 292,
    EPSP_SERVER_ERR_RQST = 293,
    EPSP_SERVER_KEY_ALRY = 295,
    EPSP_SERVER_ARGS_ERR = 298,
    EPSP_SERVER_ADDR_MIS = 299
};

enum class epsp_peer_code_t : uint16_t {
    EPSP_PEER_EQK_INFO = 551,
    EPSP_PEER_TSU_INFO = 552,
    EPSP_PEER_EQK_DTCT = 555,
    EPSP_PEER_PEER_CPR = 556,

    EPSP_PEER_ECHO_REQ = 611,
    EPSP_PEER_PID_RQST = 612,
    EPSP_PEER_PRTL_REQ = 614,
    EPSP_PEER_IVST_REQ = 615,
    EPSP_PEER_ECHO_REP = 631,
    EPSP_PEER_PID_REPL = 632,
    EPSP_PEER_PRTL_REP = 634,
    EPSP_PEER_IVST_REP = 635,
    EPSP_PEER_PRTL_REJ = 694
};

enum class epsp_state_server_t : uint8_t {
    EPSP_STATE_SERVER_DISCONNECTED,
    EPSP_STATE_SERVER_CONNECTED,

    EPSP_STATE_SERVER_WAIT_PRTL_RET,
    EPSP_STATE_SERVER_WAIT_PID_TEMP,
    EPSP_STATE_SERVER_WAIT_PORT_RET,
    EPSP_STATE_SERVER_WAIT_PEER_DAT,
    EPSP_STATE_SERVER_WAIT_PID_FINL,
    EPSP_STATE_SERVER_WAIT_KEY_ASGN,
    EPSP_STATE_SERVER_WAIT_TIME_REF,

    EPSP_STATE_SERVER_ACTIVE
};

enum class epsp_state_peer_t : uint8_t {
    EPSP_STATE_PEER_DISCONNECTED,
    EPSP_STATE_PEER_CONNECTED,

    EPSP_STATE_PEER_WAIT_PRTL_REQ,
    EPSP_STATE_PEER_WAIT_PRTL_REP,
    EPSP_STATE_PEER_WAIT_PID_RQST,
    EPSP_STATE_PEER_WAIT_PID_REPL,

    EPSP_STATE_PEER_ACTIVE
};

class ServerStates {
public:
    explicit ServerStates(epsp_state_server_t server_state);

    auto handle_message(std::string &line) -> std::string;

private:
    epsp_state_server_t server_state_ =
        epsp_state_server_t::EPSP_STATE_SERVER_DISCONNECTED;

    auto return_server_codes(uint16_t code, std::string_view data)
        -> std::string;

    auto return_epsp_server_prtl_qry() -> std::string;
    auto return_epsp_server_prtl_ret() -> std::string;
    auto return_epsp_server_pid_temp(uint16_t port) -> std::string;
    auto return_epsp_server_port_ret() -> std::string;
    auto request_epsp_client_end_sess() -> std::string;
};

class PeerStates {
public:
    explicit PeerStates(epsp_state_peer_t peer_state);

private:
};
