//
// Created by Urpagin on 2023-12-01.
// My first real C++ project
//

#ifndef SLP_H
#define SLP_H

#include <asio/ip/address_v4.hpp>
#include <asio/ip/tcp.hpp>
#include <string>
#include <vector>

/// Implementation of a simple Server List Ping protocol for Minecraft: Java
/// Edition.
///
/// Source: https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping
class slp {
    /// The Minecraft: Java Edition server address
    std::string_view server_addr;
    /// The Minecraft server port
    uint16_t server_port;
    /// A number of seconds at which the query will be cancelled.
    int timeout;
    /// The protocol version that will be sent to the server inside the Handshake
    /// packet.
    int protocol_version;

    /// Context for all IO events.
    asio::io_context io_context_;

    /// The timeout deadline for the whole time an SLP ping happens.
    asio::steady_timer deadline_;

    /// Returns the ASIO socket, connected to the MC server.
    asio::ip::tcp::socket get_conn_socket();

    /// Packet size + PacketID(0x00) + No data
    static constexpr size_t STATUS_REQUEST_SIZE{2};

    /// Builds the Handshake packet
    [[nodiscard]] std::vector<uint8_t> make_handshake_packet() const;
    /// Builds the Status Request packet.
    [[nodiscard]] static std::array<uint8_t, STATUS_REQUEST_SIZE> make_status_request_packet();
    /// Parses the socket to read the JSON.
    static std::string read_json_status_response_packet(asio::ip::tcp::socket &);

    /// Does plenty of things and return the Status Response JSON string from
    /// the MC server.
    [[nodiscard]] std::string _query_slp();

public:
    // Constructor
    // why the explicit, I don't really know. To not construct anonymously?
    explicit slp(std::string_view server_addr, uint16_t server_port = 25565, int timeout = 5,
                 int handshake_protocol_version = -1);

    /// Queries the Minecraft notchian server using the Server List Ping protocol
    /// and returns a JSON string.
    /// This is a small wrapper over the real private _query_slp() function to
    /// Add the timeout.
    [[nodiscard]] std::string query_slp();
};

#endif // SLP_H
