//
// Created by urpagin on 01/12/2023.
//

#include "Mcping.h"
#include <boost/asio.hpp>
#include <iostream>
#include <sys/socket.h>

#include <utility>
#include "DataTypesUtils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

// Constructor definition
Mcping::Mcping(std::string server_addr, uint16_t server_port, int timeout)
        : server_addr(std::move(server_addr)), server_port(server_port), timeout(timeout) {

}

uint64_t unpack_varint(int *sock, int *valread) {
    uint64_t unpackedVarint = 0;
    uint8_t tmp = 0x80;
    uint8_t i = 0;
    int a;

    while ((tmp & 0x80) && i < 5) { // Add a limit to the number of iterations
        *valread = read(*sock, &tmp, 1);

        if (*valread <= 0) { // Check for read errors or EOF
            // Handle the error or EOF
            break;
        }

        unpackedVarint |= (tmp & 0x7F) << (7 * i);
        i++;
    }
    return unpackedVarint;
}


void Mcping::ping() {
    // TODO: Make the tcp connection after packing the packet


    // [Uncompressed Packet Format]:
    // VarInt: Length of packet_id and data
    // VarInt: packet_id
    // ByteArray: data


    // [String Format]: UTF-8 string prefixed with its size in bytes as a VarInt.


    // [Handshake Packet Format]:
    // VarInt: request length in bytes   (example : 1 + 1 + 9 + 2 + 1 = 14)
    // VarInt: packetID                  (example : 0)
    // VarInt: protocol version          (example : pack_varint(760))
    // VarInt: server address length     (example : 9)
    // String: server address            (example : "127.0.0.1")
    // uint16_t: server port             (example : 25565)
    // VarInt: next state                (example : 0)

    // Preparing the Handshake packet
    uint8_t packet_id{0x00}; // 0x00 VarInt encoded remains 0x00.
    uint8_t protocol_version = DataTypesUtils::pack_varint(-1); // if the client doesn't know, default to -1.
    uint8_t server_address_length = this->server_addr.size(); // IPv4 address, max size is 15: (255.255.255.255)
    uint16_t next_state = DataTypesUtils::pack_varint(1); // 1 for status, 2 for login.

    uint8_t packet_data_length =  1                                                     // packet_id is 1 Byte
                                 + DataTypesUtils::bytes_used(protocol_version)         // Number of bytes used by the protocol version
                                 + DataTypesUtils::bytes_used(server_address_length)    // Number of bytes of the String prefix
                                 + server_address_length                                // Number of bytes in the actual String (UTF-8) (we assume it's ASCII)
                                 + 2 // port uses 2 Bytes (Unsigned Short)
                                 + 1; // next_state is either 1 or 2, so uses 1 Byte

    // Total packet size. (remember that Packet: (VarInt)packet_length + data)
    uint8_t packet_length = DataTypesUtils::bytes_used(DataTypesUtils::pack_varint(packet_data_length)) + packet_data_length;
    //std::cout << "Total packet length: " << (int)packet_length << std::endl;

    // Building the Handshake packet

    auto *data = new uint8_t[packet_length];
    uint32_t data_offset_ptr{0};

    DataTypesUtils::insert_bytes_in_data(DataTypesUtils::pack_varint(packet_data_length), &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(packet_id, &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(protocol_version, &data, &data_offset_ptr);

    // Server Address String
    DataTypesUtils::insert_bytes_in_data(DataTypesUtils::pack_varint(server_address_length), &data, &data_offset_ptr);
    DataTypesUtils::insert_string_in_data(this->server_addr, &data, &data_offset_ptr);

    DataTypesUtils::insert_bytes_in_data(this->server_port, &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(next_state, &data,
                                         &data_offset_ptr); // VarInt encodedNumbers under 127 included remain the same.

    std::string packet_data{};
    for (size_t c = 0; c < packet_length; ++c) {
        packet_data += data[c];
    }

    std::cout << "Handshake packet in string: " << packet_data << std::endl;

    // Networking:
    //// -----------------------------
    uint8_t status_request_packet[2] = {1, 0};


    //Initializing socket
    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->server_port);

    // Convert IPv4 and IPv6 addresses from text to binary
    if (inet_pton(AF_INET, this->server_addr.c_str(), &serv_addr.sin_addr)
        <= 0) {
        printf("\nInvalid address / Address not supported\n");
        exit(1);
    }

    if ((client_fd
                 = connect(sock, (struct sockaddr *) &serv_addr,
                           sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    //Sending the data
    send(sock, data, packet_length, 0); //Sends the data to the server
    send(sock, status_request_packet, 2, 0);   //Sends the status request packet

    // The server will now send information, we need to read it.
    unpack_varint(&sock, &valread); //Total packet length, not needed
    uint8_t tmp;
    valread = read(sock, &tmp, 1);  // PacketID, not needed
    uint16_t stringLength = unpack_varint(&sock, &valread);

    char *buffer = (char *) malloc(stringLength);
    valread = read(sock, buffer, stringLength);

    char *buffer2 = (char *) malloc(stringLength);
    valread = read(sock, buffer2, stringLength);

    //char *buffer3 = (char*)malloc(stringLength);
    //valread = read(sock, buffer3, stringLength);





    //printf("%s\n", buffer3);

    std::string buff{};
    buff.reserve(stringLength);

    for (size_t i{0}; i < stringLength; ++i) {
        if (buffer[i] == '\0')
            break;
        buff += buffer[i];
    }

    for (size_t i{0}; i < stringLength; ++i) {
        if (buffer2[i] == '\0')
            break;
        buff += buffer2[i];
    }

    //for (size_t i{0}; i < stringLength; ++i) {
    //    if (buffer3[i] == '\0')
    //        break;
    //    buff += buffer3[i];
    //}

    std::cout << "\n\n\n\n" << buff << std::endl;

    // closing the connected socket
    close(client_fd);

    free(data);
    free(buffer);


}

