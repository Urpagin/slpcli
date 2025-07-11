//
// Created by Urpagin on 2023-12-01.
//

#include "Mcping.h"


#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "DataTypesUtils.h"

// Constructor definition
Mcping::Mcping(std::string server_addr, uint16_t server_port)
    : server_addr(std::move(server_addr)), server_port(server_port) {}

int unpack_varint(int sock, uint64_t *out) {
  uint64_t result = 0;
  uint8_t byte = 0;
  int bytes_read = 0;

  for (int i = 0; i < 5; ++i) {
    ssize_t r = read(sock, &byte, 1);

    if (r <= 0) return -1;  // error or EOF

    // byte & 0x7F is the actual data bits, without the continuation bit
    result |= (byte & 0x7F) << (7 * i);
    bytes_read++;

    // If the continuation bit on the byte is 0, we return.
    if (!(byte & 0x80)) {
      *out = result;
      return bytes_read;
    }
  }

  return -1;  // Too many bytes, malformed VarInt
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
    if (r <= 0) return r;  // EOF or error

    out.insert(out.end(), tmp.begin(), tmp.begin() + r);
  }
  return static_cast<ssize_t>(out.size());
}

std::string Mcping::query_slp() {
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
  uint8_t packet_id{0x00};  // 0x00 VarInt encoded remains 0x00.
  uint8_t protocol_version = DataTypesUtils::pack_varint(
      -1);  // if the client doesn't know, default to -1.
  uint8_t server_address_length =
      this->server_addr
          .size();  // IPv4 address, max size is 15: (255.255.255.255)
  uint16_t next_state =
      DataTypesUtils::pack_varint(1);  // 1 for status, 2 for login.

  uint8_t packet_data_length =
      1  // packet_id is 1 Byte
      + DataTypesUtils::bytes_used(
            protocol_version)  // Number of bytes used by the protocol version
      + DataTypesUtils::bytes_used(
            server_address_length)  // Number of bytes of the String prefix
      + server_address_length  // Number of bytes in the actual String (UTF-8)
                               // (we assume it's ASCII)
      + 2                      // port uses 2 Bytes (Unsigned Short)
      + 1;                     // next_state is either 1 or 2, so uses 1 Byte

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
      &data_offset_ptr);  // VarInt encodedNumbers under 127 included remain the
                          // same.

  // -------- Networking --------
  uint8_t status_request_packet[2] = {1, 0};

  // Initializing socket
  int sock = 0, valread, client_fd;
  struct sockaddr_in serv_addr;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Error: failed to create socket." << std::endl;
    exit(1);
  }

  /* ---------- X-second timeout on recv() / read() ---------- */
  struct timeval tv;
  tv.tv_sec  = 3;          // seconds
  tv.tv_usec = 0;          // microseconds
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                 &tv, sizeof(tv)) < 0) {
    std::cerr << "Error: fiailed to set socket option: setsockopt(SO_RCVTIMEO)." << std::endl;
      exit(1);
  }

  /* ---------- X-second timeout on send() ---------- */
  if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,
                 &tv, sizeof(tv)) < 0) {
    std::cerr << "Error: fiailed to set socket option: setsockopt(SO_SNDTIMEO)." << std::endl;
    exit(1);
  }


  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(this->server_port);

  // Convert IPv4 and IPv6 addresses from text to binary
  if (inet_pton(AF_INET, this->server_addr.c_str(), &serv_addr.sin_addr) <= 0) {
    std::cerr << "Error: invalid address or unsupported address." << std::endl;
    exit(1);
  }

  if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
                           sizeof(serv_addr))) < 0) {
    std::cerr << "Error: connection failed." << std::endl;
    exit(1);
  }

  // --- Sending the data ---
  // Send the Handshake packet to the server
  if (send(sock, data, packet_length, 0) != packet_length) {
    std::cerr << "Error: failed to send the Handshake packet to the server." << std::endl;
    exit(1);
  }

  // Deallocate the buffer. I could've used a std::array, but oh well...
  free(data);

  // Sends the status request packet
  if (send(sock, status_request_packet, 2, 0) != 2) {
    std::cerr << "Error: failed to send the Status Request packet to the server." << std::endl;
    exit(1);
  }

  // ----- READING SERVER -----


  // Read packet length (VarInt)
  uint64_t srv_p_len;
  if (unpack_varint(sock, &srv_p_len) < 1) {
    std::cerr << "Error: failed to read the Status Response length" << std::endl;
    exit(1);
  }

  // Read packet ID (VarInt, but 0x00 is still 0x00)
  uint8_t srv_p_id;
  if (read(sock, &srv_p_id, 1) < 1) {
    std::cerr << "Error: failed to read the Status Response ID" << std::endl;
    exit(1);
  }
  if (srv_p_id != 0) {
    std::cerr << std::format("Error: received Status Response packet ID is incorrect; expected 0x00, got 0x{0:02X}", srv_p_id);
    exit(1);
  }

  uint64_t srv_str_len;
  if (unpack_varint(sock, &srv_str_len) < 1) {
    std::cerr << "Error: failed reading the Status Response string length" << std::endl;
    exit(1);
  }

  if (srv_str_len >= srv_p_len) {
    std::cerr << "Error: Status Response string length is greater than itself; nonsense." << std::endl;
    exit(1);
  }

  // Read the JSON string.
  std::vector<uint8_t> json_data;
  json_data.reserve(srv_str_len);
  read_json_string(sock, (size_t)srv_str_len, json_data);

  // Close the socket.
  close(client_fd);

  // Convert bytes into String.
  std::string json_string(json_data.begin(), json_data.end());

  return json_string;

}
