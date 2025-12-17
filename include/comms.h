#pragma once
#include "asio.hpp"
#include "common.h"

#define EPSP_PROTOCOL_VER 38
#define EPSP_CLIENT_NAME "P2PClient-Linux"
#define EPSP_CLIENT_VER "Alpha0.1"

#define EPSP_MAX_PEERS 8
#define EPSP_MAX_ADDR_LEN 64

// define codes

const std::string server_list[4] = {"p2pquake.info", "www.p2pquake.net",
                                    "p2pquake.xyz", "p2pquake.ddo.jp"};

typedef enum {
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
} epsp_client_code_t;

typedef enum {
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
} epsp_server_code_t;

typedef enum {
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
} epsp_peer_code_t;

typedef enum {
  EPSP_STATE_DISCONNECTED,
  EPSP_STATE_CONNECTED,
  EPSP_STATE_TEMP_ID_ASSIGNED,
  EPSP_STATE_FINAL_ID_ASSIGNED,
  EPSP_STATE_KEY_ASSIGNED,
  EPSP_STATE_ACTIVE
} epsp_state_t;

typedef struct {
  uint32_t peer_id;

  char ip[EPSP_MAX_ADDR_LEN];
  uint16_t port;

  uint16_t region_code;

  time_t last_echo;
  bool connected;

  uint16_t protocol_ver;
} epsp_peer_t;

typedef struct {
  uint8_t *public_key;
  size_t public_key_size;

  uint8_t *private_key;
  size_t private_key_size;

  uint8_t *signature;
  size_t signature_size;

  time_t expires_at;

  bool is_server_key;
  bool verified;
} epsp_key_t;

typedef struct {
  uint16_t code;
  uint16_t hop;

  char **data;
  size_t data_size;

  uint8_t *signature;
  size_t signature_size;

  time_t received_at;
} epsp_message_t;
