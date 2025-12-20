#pragma once

// constants here
constexpr const char *EPSP_PROTOCOL_VER = "0.38";
constexpr const char *EPSP_CLIENT_NAME = "P2PClient-Linux";
constexpr const char *EPSP_CLIENT_VER = "Alpha0.1";
constexpr int EPSP_MAX_PEERS = 8;
constexpr int EPSP_MAX_ADDR_LEN = 64;

inline const std::array<const char *, 4> EPSP_SERVERS = {
    "p2pquake.info", "www.p2pquake.net", "p2pquake.xyz", "p2pquake.ddo.jp"};

enum class epsp_role_t : uint8_t { UNKNOWN, SERVER, PEER };
