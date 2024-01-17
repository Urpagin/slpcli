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

uint64_t unpack_varint(int *sock, int *valread){
    uint64_t unpackedVarint = 0;
    uint8_t tmp = 0x80;
    uint8_t i = 0;

    while((tmp & 0x80) && i < 5) { // Add a limit to the number of iterations
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


    // Uncompressed Packet format:
    // VarInt: length_packet_id + data_length
    // VarInt: packet_id
    // ByteArray: data

    // Handshake packet:



    //Handshake request:
    //VarInt: request length in bytes   (example : 1 + 1 + 9 + 2 + 1 = 14)
    //VarInt: packetID                  (example : 0)
    //VarInt: protocol Version          (example : pack_varint(760))
    //VarInt: Server address length     (example : 9)
    //String: Server address            (example : "127.0.0.1")
    //uint16_t: Server Port             (example : 25565)
    //VarInt: next state                (example : 0)

    // func returns an uint64, don't know why the LSP doesn't give a type warning/error as I'm assigning an uint32
    // https://github.com/LhAlant/MinecraftSLP/blob/main/MinecraftSLP.c#L34 ofc
    uint32_t protocol_version_varint = DataTypesUtils::pack_varint(-1); // todo: -1
    uint8_t packet_id = 0x00;
    uint32_t server_address_length = this->server_addr.size();
    uint8_t next_state = DataTypesUtils::pack_varint(1);

    uint32_t request_length = DataTypesUtils::bytes_used(packet_id)
                              + DataTypesUtils::bytes_used(protocol_version_varint)
                              + DataTypesUtils::bytes_used(server_address_length)
                              + server_address_length
                              + 2  // Port uses 2 Bytes
                              + 1; // next_state is either 0 or 1, so 1 Byte

    server_address_length = DataTypesUtils::pack_varint(server_address_length); // Length will be sent as a varint
    uint32_t total_request_length = request_length + DataTypesUtils::bytes_used(request_length); // ???
    std::cout << total_request_length << std::endl;
    auto *data = new uint8_t[total_request_length];
    //std::unique_ptr<uint8_t[]> data(new uint8_t[total_request_length]);
    uint32_t data_offset_ptr{0};

    DataTypesUtils::insert_bytes_in_data(DataTypesUtils::pack_varint(request_length), &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(packet_id, &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(protocol_version_varint, &data, &data_offset_ptr);
    DataTypesUtils::insert_string_in_data(this->server_addr, server_address_length, &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(this->server_port, &data, &data_offset_ptr);
    DataTypesUtils::insert_bytes_in_data(next_state, &data, &data_offset_ptr);

    //// -----------------------------
    uint8_t status_request_packet[2] = {1, 0};
//
    //using boost::asio::ip::tcp;
//
    //boost::asio::io_context io_context;
    //tcp::socket socket(io_context);
    //tcp::resolver resolver(io_context);
    //boost::asio::connect(socket, resolver.resolve(this->server_addr, std::to_string(this->server_port)));
//
    //boost::asio::write(socket, boost::asio::buffer(data, total_request_length));
    //boost::asio::write(socket, boost::asio::buffer(status_request_packet, 2));
//
    //char reply[10240];
    //std::cout.write("noir", 6);
    //size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, 10240));
    //std::cout.write(reply, reply_length);
    //std::cout << "\n";
    //
    //// -----


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
                 = connect(sock, (struct sockaddr*)&serv_addr,
                           sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    //Sending the data
    send(sock, data, total_request_length, 0); //Sends the data to the server
    send(sock, status_request_packet, 2, 0);   //Sends the status request packet

    // The server will now send information, we need to read it.
    unpack_varint(&sock, &valread); // Total packet length, not needed



    uint8_t tmp;
    valread = read(sock, &tmp, 1);  //PacketID, not needed
    uint64_t stringLength = unpack_varint(&sock, &valread);

    char *buffer = static_cast<char *>(malloc(stringLength));
    valread = read(sock, buffer, stringLength);
    printf("%s\n", buffer);

    // closing the connected socket
    close(client_fd);


    // Clean up
    delete[] data;


}

