//
// Created by urpagin on 01/12/2023.
//

#include "Mcping.h"
#include <boost/asio.hpp>
#include <iostream>
#include <utility>


// Constructor definition
Mcping::Mcping(std::string server_addr, u_int16_t server_port, int timeout)
: server_addr(std::move(server_addr)), server_port(server_port), timeout(timeout) {

}

void Mcping::ping() {

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

























