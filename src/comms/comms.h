#pragma once
#include "asio.hpp"

// constants here
constexpr const char *EPSP_PROTOCOL_VER = "0.38";
constexpr const char *EPSP_CLIENT_NAME = "P2PClient-Linux";
constexpr const char *EPSP_CLIENT_VER = "Alpha0.1";
constexpr int EPSP_MAX_PEERS = 8;
constexpr int EPSP_MAX_ADDR_LEN = 64;

inline const std::array<const char *, 4> EPSP_SERVERS = {
    "p2pquake.info", "www.p2pquake.net", "p2pquake.xyz", "p2pquake.ddo.jp"};

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

enum class epsp_state_t : uint8_t {
    EPSP_STATE_DISCONNECTED,
    EPSP_STATE_CONNECTING,
    EPSP_STATE_CONNECTED,

    EPSP_STATE_WAIT_SERVER_PRTL_QRY,
    EPSP_STATE_WAIT_SERVER_PRTL_RET,
    EPSP_STATE_WAIT_SERVER_PID_TEMP,
    EPSP_STATE_WAIT_SERVER_PORT_RET,
    EPSP_STATE_WAIT_SERVER_PEER_DAT,
    EPSP_STATE_WAIT_SERVER_PID_FINL,
    EPSP_STATE_WAIT_SERVER_KEY_ASGN,
    EPSP_STATE_WAIT_SERVER_TIME_REF,

    EPSP_STATE_ACTIVE
};

enum class epsp_state_peer_t : uint8_t {
    EPSP_STATE_PEER_CONNECTING,
    EPSP_STATE_PEER_HANDSHAKE,
    EPSP_STATE_PEER_ACTIVE,
    EPSP_STATE_PEER_CLOSED
};

// classes - a nice way to headbash developers!

enum class epsp_role_t : uint8_t { UNKNOWN, SERVER, PEER };

class Connection : public std::enable_shared_from_this<Connection> {
  public:
    static auto create(asio::io_context &io_context,
                       epsp_role_t role = epsp_role_t::UNKNOWN)
        -> std::shared_ptr<Connection>;

    auto socket() -> asio::ip::tcp::socket &;
    auto role() -> epsp_role_t;

    void start();

  private:
    explicit Connection(asio::io_context &io_context,
                        epsp_role_t role = epsp_role_t::UNKNOWN);
    asio::ip::tcp::socket socket_;
    epsp_role_t role_;
    asio::streambuf buffer_;

    void do_read();
    void handle_message(std::string line);
    void do_write(std::string data);
};

// may be obsolete idk
//
// typedef struct {
//  uint32_t peer_id;
//
//  char ip[EPSP_MAX_ADDR_LEN];
//  uint16_t port;
//
//  uint16_t region_code;
//
//  time_t last_echo;
//  bool connected;
//
//  uint16_t protocol_ver;
//} epsp_peer_t;
//
// typedef struct {
//  uint8_t *public_key;
//  size_t public_key_size;
//
//  uint8_t *private_key;
//  size_t private_key_size;
//
//  uint8_t *signature;
//  size_t signature_size;
//
//  time_t expires_at;
//
//  bool is_server_key;
//  bool verified;
//} epsp_key_t;
//
// typedef struct {
//  uint16_t code;
//  uint16_t hop;
//
//  char **data;
//  size_t data_size;
//
//  uint8_t *signature;
//  size_t signature_size;
//
//  time_t received_at;
//} epsp_message_t;
