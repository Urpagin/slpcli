//
// Created by urpagin on 24/12/2023.
//

#ifndef MCPINGERCPP_DATATYPESUTILS_H
#define MCPINGERCPP_DATATYPESUTILS_H

#include <asio.hpp>
#include <vector>

/// @brief Some utility functions for the Minecraft: Java Edition protocol data
/// types such as VarInt and Strings.
class DataTypesUtils {
    // Private constructor to prevent instantiation. It's a utility class
    DataTypesUtils() = default;

    static constexpr uint64_t DATA_BITS{0x7F}; //       0111 1111
    static constexpr uint64_t CONTINUE_BIT{0x80}; //    1000 0000
    static constexpr size_t MAX_VARINT_SIZE{5};
    /// Packet size + PacketID(0x00) + No data
    static constexpr size_t STATUS_REQUEST_SIZE{2};

public:
    static std::vector<uint8_t> make_varint(int value);

    static std::vector<uint8_t> make_string(std::string_view s);

    static asio::awaitable<int> read_varint(asio::ip::tcp::socket &sock);

    static std::vector<uint8_t> make_handshake_packet(std::string_view address, uint16_t port, int p_version);

    static constexpr std::array<uint8_t, STATUS_REQUEST_SIZE> make_status_request_packet() { return {1, 0}; }
};

#endif // MCPINGERCPP_DATATYPESUTILS_H
