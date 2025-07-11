//
// Created by Urpagin on 2023-12-01.
//

#include "Mcping.h"
#include <algorithm>
#include <array>
#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/socket.h>

#include "DataTypesUtils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utility>
#include <vector>

// Constructor definition
Mcping::Mcping(std::string server_addr, uint16_t server_port, int timeout)
    : server_addr(std::move(server_addr)), server_port(server_port),
      timeout(timeout) {}

int unpack_varint(int sock, uint64_t *out) {
  uint64_t result = 0;
  uint8_t byte = 0;
  int bytes_read = 0;

  for (int i = 0; i < 5; ++i) {
    ssize_t r = read(sock, &byte, 1);
    printf("VARINT BYTE: %X\n", byte);

    if (r <= 0)
      return -1; // error or EOF

    // byte & 0x7F is the actual data bits, without the continuation bit
    result |= (byte & 0x7F) << (7 * i);
    bytes_read++;

    // If the continuation bit on the byte is 0, we return.
    if (!(byte & 0x80)) {
      *out = result;
      return bytes_read;
    }
  }

  return -1; // Too many bytes, malformed VarInt
}

static std::string hex_str(const uint8_t *data, int len) {
  std::stringstream ss;
  ss << std::hex;

  for (int i(0); i < len; ++i)
    ss << std::setw(2) << std::setfill('0') << (int)data[i];

  return ss.str();
}

/// A function that reads the string bytes from sock.
/// Returns -1 if error.
static ssize_t read_json_string(int sock, std::size_t bytes_needed,
                                std::vector<std::uint8_t> &out) {
  constexpr std::size_t BUF_SIZE = 256;
  std::array<std::uint8_t, BUF_SIZE> tmp;
  out.clear();
  out.reserve(bytes_needed);

  while (out.size() < bytes_needed) {
    size_t to_read = std::min(BUF_SIZE, bytes_needed - out.size());
    ssize_t r = ::read(sock, tmp.data(), to_read);
    if (r <= 0)
      return r; // EOF or error

    out.insert(out.end(), tmp.begin(), tmp.begin() + r);
  }
  return static_cast<ssize_t>(out.size());
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
  uint8_t protocol_version = DataTypesUtils::pack_varint(
      -1); // if the client doesn't know, default to -1.
  uint8_t server_address_length =
      this->server_addr
          .size(); // IPv4 address, max size is 15: (255.255.255.255)
  uint16_t next_state =
      DataTypesUtils::pack_varint(1); // 1 for status, 2 for login.

  uint8_t packet_data_length =
      1 // packet_id is 1 Byte
      + DataTypesUtils::bytes_used(
            protocol_version) // Number of bytes used by the protocol version
      + DataTypesUtils::bytes_used(
            server_address_length) // Number of bytes of the String prefix
      + server_address_length // Number of bytes in the actual String (UTF-8)
                              // (we assume it's ASCII)
      + 2                     // port uses 2 Bytes (Unsigned Short)
      + 1;                    // next_state is either 1 or 2, so uses 1 Byte

  // Total packet size. (remember that Packet: (VarInt)packet_length + data)
  uint8_t packet_length = DataTypesUtils::bytes_used(
                              DataTypesUtils::pack_varint(packet_data_length)) +
                          packet_data_length;
  // std::cout << "Total packet length: " << (int)packet_length << std::endl;

  // Building the Handshake packet

  auto *data = new uint8_t[packet_length];
  uint32_t data_offset_ptr{0};

  DataTypesUtils::insert_bytes_in_data(
      DataTypesUtils::pack_varint(packet_data_length), &data, &data_offset_ptr);
  DataTypesUtils::insert_bytes_in_data(packet_id, &data, &data_offset_ptr);
  DataTypesUtils::insert_bytes_in_data(protocol_version, &data,
                                       &data_offset_ptr);

  // Server Address String
  DataTypesUtils::insert_bytes_in_data(
      DataTypesUtils::pack_varint(server_address_length), &data,
      &data_offset_ptr);
  DataTypesUtils::insert_string_in_data(this->server_addr, &data,
                                        &data_offset_ptr);

  DataTypesUtils::insert_bytes_in_data(this->server_port, &data,
                                       &data_offset_ptr);
  DataTypesUtils::insert_bytes_in_data(
      next_state, &data,
      &data_offset_ptr); // VarInt encodedNumbers under 127 included remain the
                         // same.

  std::cout << "Handshake packet in string: " << hex_str(data, packet_length)
            << std::endl;

  // -------- Networking --------
  uint8_t status_request_packet[2] = {1, 0};

  // Initializing socket
  int sock = 0, valread, client_fd;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    exit(1);
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(this->server_port);

  // Convert IPv4 and IPv6 addresses from text to binary
  if (inet_pton(AF_INET, this->server_addr.c_str(), &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address / Address not supported\n");
    exit(1);
  }

  if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                           sizeof(serv_addr))) < 0) {
    printf("\nConnection Failed \n");
    exit(1);
  }

  // --- Sending the data ---
  // Send the Handshake packet to the server
  if (send(sock, data, packet_length, 0) != packet_length) {
    printf("Error sending the Handshake packet to the server!\n");
    exit(1);
  }

  // Sends the status request packet
  if (send(sock, status_request_packet, 2, 0) != 2) {
    printf("Error sending the Status Request packet to the server\n");
    exit(1);
  }

  // ----- READING SERVER -----

  // Read packet length (VarInt)
  uint64_t srv_p_len;
  if (unpack_varint(sock, &srv_p_len) < 1) {
    printf("Error reading the Status Response length\n");
    exit(1);
  }
  printf("Status Response packet length: %ld\n", srv_p_len);

  // Read packet ID (VarInt, but 0x00 is still 0x00)
  uint8_t srv_p_id;
  if (read(sock, &srv_p_id, 1) < 1) {
    printf("Error reading the Status Response ID\n");
    exit(1);
  }
  printf("Status Response packet ID: %d\n", srv_p_id);
  if (srv_p_id != 0) {
    printf("Status Response ID is incorrect; expected 0x00, got 0x%02X\n",
           srv_p_id);
  }

  uint64_t srv_str_len;
  if (unpack_varint(sock, &srv_str_len) < 1) {
    printf("Error reading the Status Response string length\n");
    exit(1);
  }
  printf("Status Response string length: %ld\n", srv_str_len);
  if (srv_str_len >= srv_p_len) {
    printf("Status Response string length is greater than the packet itself; "
           "nonsense.\n");
    exit(1);
  }

  // Read the JSON bytes.
  std::vector<uint8_t> json_data;
  json_data.reserve(srv_str_len);
  read_json_string(sock, (size_t)srv_str_len, json_data);

  std::string json_string(json_data.begin(), json_data.end());
  std::cout << json_string << std::endl;

  // std::cout << "\n\n\n\n" << buff << std::endl;

  // closing the connected socket
  close(client_fd);

  free(data);
  // free(buffer);
}
