#pragma once

// constants here
#include <string_view>
constexpr std::string_view EPSP_PROTOCOL_VER = "0.38";
constexpr std::string_view EPSP_CLIENT_NAME = "P2PClient-Linux";
constexpr std::string_view EPSP_CLIENT_VER = "Alpha0.1";
constexpr int EPSP_MAX_PEERS = 8;
constexpr int EPSP_MAX_ADDR_LEN = 64;
constexpr int EPSP_PORT = 6911;

inline const std::array<std::string_view, 4> EPSP_SERVERS = {
    "p2pquake.info", "www.p2pquake.net", "p2pquake.xyz", "p2pquake.ddo.jp"};

inline std::atomic<std::uint32_t> id{0};
