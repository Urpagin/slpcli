//
// Created by Urpagin on 2023-12-24.
//

#include "DataTypesUtils.h"

#include <asio.hpp>

/// https://minecraft.wiki/w/Java_Edition_protocol/Data_types#VarInt_and_VarLong
/// @brief Makes a VarInt from an int.
/// @return A vector of bytes ranging from 1 to 5 bytes inclusive.
std::vector<uint8_t> DataTypesUtils::make_varint(const int value) {
    auto val = static_cast<uint32_t>(value); // Don't bother with negative.
    std::vector<uint8_t> varint{}; // Byte by Byte vector
    varint.reserve(MAX_VARINT_SIZE); // Max 5 bytes for VarInt.

    while (val & (~DATA_BITS)) {
        // From the LSB's side.
        varint.emplace_back((val & DATA_BITS) | CONTINUE_BIT);
        val >>= 7;
    }
    varint.emplace_back(val & DATA_BITS);
    return varint;
}

/// https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String
/// @brief Makes an MC Protocol String.
/// @details Format: [VarInt of the size in bytes of the string, string bytes]
std::vector<uint8_t> DataTypesUtils::make_string(const std::string_view s) {
    const auto varint = DataTypesUtils::make_varint(static_cast<int>(s.size()));
    std::vector<uint8_t> data;
    data.reserve(varint.size() + s.size());
    data.insert(data.end(), varint.begin(), varint.end());
    data.insert(data.end(), s.begin(), s.end());
    return data;
}

/// @brief Tries to read the first VarInt from `sock`. Which corresponds to the
/// Status Response's JSON string size in bytes.
/// @returns The VarInt read from `sock`.
/// @param sock The socket to read from.
/// @throws asio::system_error of code type asio::error::eof if EOF, some other
/// type otherwise.
asio::awaitable<int> DataTypesUtils::read_varint(asio::ip::tcp::socket &sock) {
    static constexpr uint8_t DATA_BITS{0x7F};
    static constexpr uint8_t CONTINUATION_BIT{0x80};
    // A container with at least 5 bytes to hold the VarInt data.
    uint64_t result = 0;
    // The buffer, one byte at a time.
    uint8_t byte = 0;

    for (size_t i{0}; i < 5; ++i) {
        // Throws exception if error or EOF.
        // asio::read(sock, asio::buffer(&byte, 1));
        if (auto [ec, read] = co_await asio::async_read(sock, asio::buffer(&byte, 1), asio::as_tuple(asio::use_awaitable)); ec)
            throw asio::system_error(ec);

        // Must be before the check for the zero-continuation bit.
        // Add the data bits to result.
        result |= (byte & DATA_BITS) << (7 * i);

        // If the continuation bit on the byte is 0, we return.
        if (!(byte & CONTINUATION_BIT)) {
            co_return static_cast<int>(result);
        }
    }

    // Should throw if we read 5 bytes without finding the end
    throw std::runtime_error("VarInt too large");
}


/// @brief Builds the Handshake packet.
std::vector<uint8_t> DataTypesUtils::make_handshake_packet(const std::string_view address, const uint16_t port,
                                                           const int p_version) {
    // https://minecraft.wiki/w/Java_Edition_protocol/Packets#Without_compression
    // [Uncompressed Packet Format]:
    // VarInt: Length of packet_id and data
    // VarInt: packet_id
    // ByteArray: data

    // https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String
    // [String Format]: UTF-8 string prefixed with its size in bytes as a VarInt.

    // https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping
    //  [Handshake Packet Format]:
    //  First, the client sends a Handshake packet with its state set to 1.
    //
    //  Packet ID   Field Name        Field Type

    // 0x00        Protocol Version  VarInt
    //             Server Address    String
    //             Server Port       Unsigned Short
    //             Next State        VarInt


    const auto packet_id = make_varint(0x00);
    const auto protocol_version = make_varint(p_version);
    const auto server_address = make_string(address);
    // const uint16_t server_port = this->server_port;
    const std::array server_port = {
            static_cast<uint8_t>(port >> 8), // High byte
            static_cast<uint8_t>(port & 0xFF), // Low byte
    };
    const auto next_state = make_varint(1); // 1 for status, 2 for login, 3 for transfer

    const size_t packet_size =
            packet_id.size() + protocol_version.size() + server_address.size() + server_port.size() + next_state.size();

    const auto packet_size_varint = make_varint(static_cast<int>(packet_size));

    std::vector<uint8_t> packet;
    packet.reserve(packet_size + packet_size_varint.size());
    packet.insert(packet.end(), packet_size_varint.begin(), packet_size_varint.end());
    packet.insert(packet.end(), packet_id.begin(), packet_id.end());
    packet.insert(packet.end(), protocol_version.begin(), protocol_version.end());
    packet.insert(packet.end(), server_address.begin(), server_address.end());
    packet.insert(packet.end(), server_port.begin(), server_port.end());
    packet.insert(packet.end(), next_state.begin(), next_state.end());

    return packet;
}
