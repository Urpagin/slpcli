//
// Created by urpagin on 01/12/2023.
//

#include "Mcping.h"
#include <boost/asio.hpp>
#include <iostream>
#include <utility>
#include "DataTypesUtils.h"


// Constructor definition
Mcping::Mcping(std::string server_addr, uint16_t server_port, int timeout)
        : server_addr(std::move(server_addr)), server_port(server_port), timeout(timeout) {

}

void Mcping::ping() {
    // TODO: Make the tcp connection after packing the packet

    using boost::asio::ip::tcp;

    // The io_context is required for all I/O
    boost::asio::io_context io_context;

    // Resolve the server address and port
    tcp::resolver resolver(io_context);
    // Server address and port must be strings
    auto endpoints = resolver.resolve(this->server_addr, std::to_string(this->server_port));

    // Create and connect the socket
    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    // Now the socket is connected and can be used to send and receive data

    //Handshake request:
    //Varint: request length in bytes   (example : 1 + 1 + 9 + 2 + 1 = 14)
    //Varint: packetID                  (example : 0)
    //Varint: protocol Version          (example : pack_varint(760))
    //Varint: Server address length     (example : 9)
    //String: Server address            (example : "127.0.0.1")
    //uint16_t: Server Port             (example : 25565)
    //Varint: next state                (example : 0)

    // func returns an uint64, don't know why the LSP doesn't give a type warning/error as I'm assigning an uint32
    // https://github.com/LhAlant/MinecraftSLP/blob/main/MinecraftSLP.c#L34 ofc
    uint32_t protocol_version_varint = DataTypesUtils::pack_varint(-1);
    uint8_t packet_id = 0;
    uint32_t server_address_length = this->server_addr.size();
    uint8_t next_state = DataTypesUtils::pack_varint(1);

    uint32_t request_length = DataTypesUtils::bytes_used(packet_id)
                              + DataTypesUtils::bytes_used(protocol_version_varint)
                              + DataTypesUtils::bytes_used(server_address_length)
                              + server_address_length
                              + 2 // Port uses 2 Bytes
                              + 1; // next_state is either 0 or 1, so 1 Byte
    server_address_length = DataTypesUtils::pack_varint(server_address_length); // Length will be sent as a varint
    uint32_t total_request_length = request_length + DataTypesUtils::bytes_used(request_length); // ???
    std::unique_ptr<uint8_t[]> data(new uint8_t[total_request_length]);



    // Convert to big-endian if the system is little-endian
    uint16_t big_endian_port = htons(this->server_port);
    // We then break the 16-bit port number into two 8-bit parts and store them in a std::array.
    // The high-order byte is placed first (big_endian_port >> 8), followed by the low-order byte (big_endian_port & 0xFF).
    // The resulting packed_port array contains the port number in big-endian byte order, similar to the Ruby pack("S>") method.
    std::array<uint8_t, sizeof(uint16_t)> packed_port = {
            static_cast<uint8_t>(big_endian_port >> 8), // to get the eight last bits
            static_cast<uint8_t>(big_endian_port & 0xFF) // 0xFF = 255 - to get the eight first bits
    };

    // packed_host = pack_data(@server.encode("UTF-8"))

}

























