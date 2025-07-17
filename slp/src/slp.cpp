//
// Created by Urpagin on 2023-12-01.
//

#include "../include/slp.h"
#include <arpa/inet.h>

#include <algorithm>
#include <array>

#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "DataTypesUtils.h"

// Constructor definition
slp::slp(const std::string_view server_addr, const uint16_t server_port,
         const int timeout)
    : server_addr(server_addr), server_port(server_port), timeout(timeout) {}

/// @brief Builds the Handshake packet.
std::vector<uint8_t>
slp::make_handshake_packet(const int protocol_version_num = -1) const {
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

  using dtu = DataTypesUtils;

  const auto packet_id = dtu::make_varint(0x00);
  const auto protocol_version = dtu::make_varint(protocol_version_num);
  const auto server_address = dtu::make_string(this->server_addr);
  // const uint16_t server_port = this->server_port;
  const std::array server_port = {
      static_cast<uint8_t>(this->server_port >> 8),   // High byte
      static_cast<uint8_t>(this->server_port & 0xFF), // Low byte
  };
  const auto next_state =
      dtu::make_varint(1); // 1 for status, 2 for login, 3 for transfer

  const size_t packet_size = packet_id.size() + protocol_version.size() +
                             server_address.size() + server_port.size() +
                             next_state.size();

  const auto packet_size_varint =
      dtu::make_varint(static_cast<int>(packet_size));

  std::vector<uint8_t> packet;
  packet.reserve(packet_size + packet_size_varint.size());

  packet.append_range(packet_size_varint);
  packet.append_range(packet_id);
  packet.append_range(protocol_version);
  packet.append_range(server_address);
  packet.append_range(server_port);
  packet.append_range(next_state);

  return packet;
}

/// @brief Builds the Status Request packet.
std::array<uint8_t, slp::STATUS_REQUEST_SIZE>
slp::make_status_request_packet() {
  return std::array<uint8_t, STATUS_REQUEST_SIZE>{1, 0};
}

/// @brief Read the socket for the Status Response packet and returns the bytes
/// of its JSON string.
/// @returns The bytes of the JSON.
std::string slp::read_json_status_response_packet(asio::ip::tcp::socket &sock) {
  using dtu = DataTypesUtils;
  int json_size{0};

  try {
    // Read the size in bytes of the JSON from sock.
    json_size = dtu::read_varint(sock);
  } catch (const asio::system_error &e) {
    if (e.code() == asio::error::eof) {
      std::cerr << "Error: failed to read Status Response packet JSON size: EOF"
                << std::endl;
    } else {
      std::cerr << "Error: failed to read Status Response packet JSON size: "
                << e.what() << std::endl;
    }
    throw std::runtime_error(
        "Failed to read the Status Response packet JSON size");
  }

  if (json_size <= 0) {
    throw std::runtime_error("Failed to read the Status Response packet JSON "
                             "size: JSON size is 0 or less");
  }

  // std::vector<uint8_t> buffer(json_size);
  std::string buffer(json_size, '\0');

  try {
    // Read the JSON string into memory.
    asio::read(sock, asio::buffer(buffer.data(), json_size));
  } catch (const asio::system_error &e) {
    if (e.code() == asio::error::eof) {
      std::cerr << "Error: failed to read Status Response packet JSON: EOF"
                << std::endl;
    } else {
      std::cerr << "Error: failed to read Status Response packet JSON size: "
                << e.what() << std::endl;
    }
    throw std::runtime_error(
        "Failed to read the Status Response packet JSON size");
  }
  std::cout << "size json: " << buffer.size() << std::endl;
  std::cout << "size json_size: " << json_size << std::endl;

  return buffer;
}

/// @brief Resolves the address, connect to the server and return the TCP
/// socket.
asio::ip::tcp::socket slp::get_conn_socket(asio::io_context &io_context) const {
  using asio::ip::tcp;

  // Resolve the domain for an actual IPv4 or IPv6 as the TCP protocol works
  // with IPs and not domain names.
  tcp::resolver resolver(io_context);
  tcp::socket socket(io_context);
  asio::connect(socket, resolver.resolve(this->server_addr,
                                         std::to_string(this->server_port)));

  // TODO: Set timeout
  // TODO: Set timeout
  // TODO: Set timeout
  // TODO: Set timeout
  // TODO: Set timeout
  // TODO: Set timeout
  // TODO: Set timeout

  return std::move(socket);
}

std::string slp::query_slp() {
  // 'Build Handshake' and 'Status Request' packets.
  auto handshake_packet = make_handshake_packet(-1);
  auto status_request_packet = make_status_request_packet();

  asio::io_context io_context{};
  try {
    // Init socket & connect to MC server.
    auto socket = get_conn_socket(io_context);

    // Send server-bound packets:
    asio::write(socket, asio::buffer(handshake_packet));
    asio::write(socket, asio::buffer(status_request_packet));

    // Receive client-bound packet JSON string.
    return read_json_status_response_packet(socket);

  } catch (const asio::system_error &e) {
    std::cerr << "Error: connection with the MC server failed: " << e.what()
              << std::endl;
    throw;
  }
}

// std::string slp::query_slp() {
//   // [Uncompressed Packet Format]:
//   // VarInt: Length of packet_id and data
//   // VarInt: packet_id
//   // ByteArray: data
//
//   // [String Format]: UTF-8 string prefixed with its size in bytes as a
//   VarInt.
//
//   // [Handshake Packet Format]:
//   // VarInt: request length in bytes   (example : 1 + 1 + 9 + 2 + 1 = 14)
//   // VarInt: packetID                  (example : 0)
//   // VarInt: protocol version          (example : pack_varint(760))
//   // VarInt: server address length     (example : 9)
//   // String: server address            (example : "127.0.0.1")
//   // uint16_t: server port             (example : 25565)
//   // VarInt: next state                (example : 0)
//
//   // Preparing the Handshake packet
//   uint8_t packet_id{0x00}; // 0x00 VarInt encoded remains 0x00.
//   // if the client doesn't know, default to -1.
//   uint64_t protocol_version =
//       DataTypesUtils::pack_varint(static_cast<uint32_t>(-1));
//   size_t server_address_length =
//       this->server_addr
//           .size(); // IPv4 address, max size is 15: (255.255.255.255)
//   uint64_t next_state =
//       DataTypesUtils::pack_varint(1); // 1 for status, 2 for login.
//
//   size_t packet_data_length =
//       static_cast<size_t>(1) // packet_id is 1 Byte
//       // Number of bytes used by the protocol version
//       + DataTypesUtils::bytes_used(static_cast<uint32_t>(protocol_version))
//       // Number of bytes of the String prefix
//       +
//       DataTypesUtils::bytes_used(static_cast<uint32_t>(server_address_length))
//       + server_address_length // Number of bytes in the actual String (UTF-8)
//                             // (we assume it's ASCII)
//       + 2                   // port uses 2 Bytes (Unsigned Short)
//       + 1;                  // next_state is either 1 or 2, so uses 1 Byte
//
//   // Total packet size. (remember that Packet: (VarInt)packet_length + data)
//   uint8_t packet_length = DataTypesUtils::bytes_used(
//                               DataTypesUtils::pack_varint(packet_data_length))
//                               +
//                           packet_data_length;
//   // std::cout << "Total packet length: " << (int)packet_length << std::endl;
//
//   // Building the Handshake packet
//
//   auto *data = new uint8_t[packet_length];
//   uint32_t data_offset_ptr{0};
//
//   DataTypesUtils::insert_bytes_in_data(
//       DataTypesUtils::pack_varint(packet_data_length), &data,
//       &data_offset_ptr);
//   DataTypesUtils::insert_bytes_in_data(packet_id, &data, &data_offset_ptr);
//   DataTypesUtils::insert_bytes_in_data(protocol_version, &data,
//                                        &data_offset_ptr);
//
//   // Server Address String
//   DataTypesUtils::insert_bytes_in_data(
//       DataTypesUtils::pack_varint(server_address_length), &data,
//       &data_offset_ptr);
//   DataTypesUtils::insert_string_in_data(this->server_addr, &data,
//                                         &data_offset_ptr);
//
//   DataTypesUtils::insert_bytes_in_data(this->server_port, &data,
//                                        &data_offset_ptr);
//   DataTypesUtils::insert_bytes_in_data(
//       next_state, &data,
//       &data_offset_ptr); // VarInt encodedNumbers under 127 included remain
//       the
//                          // same.
//
//   // -------- Networking --------
//   uint8_t status_request_packet[2] = {1, 0};
//
//   // Initializing socket
//   int sock = 0, valread, client_fd;
//   struct sockaddr_in serv_addr;
//
//   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//     std::cerr << "Error: failed to create socket." << std::endl;
//     exit(1);
//   }
//
//   /* ---------- X-second timeout on recv() / read() ---------- */
//   struct timeval tv;
//   tv.tv_sec = 3;  // seconds
//   tv.tv_usec = 0; // microseconds
//   if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
//     std::cerr << "Error: fiailed to set socket option:
//     setsockopt(SO_RCVTIMEO)."
//               << std::endl;
//     exit(1);
//   }
//
//   /* ---------- X-second timeout on send() ---------- */
//   if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
//     std::cerr << "Error: fiailed to set socket option:
//     setsockopt(SO_SNDTIMEO)."
//               << std::endl;
//     exit(1);
//   }
//
//   serv_addr.sin_family = AF_INET;
//   serv_addr.sin_port = htons(this->server_port);
//
//   // Convert IPv4 and IPv6 addresses from text to binary
//   if (inet_pton(AF_INET, this->ip.c_str(), &serv_addr.sin_addr) <= 0) {
//     std::cerr << "Error: invalid address or unsupported address." <<
//     std::endl; exit(1);
//   }
//
//   if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr,
//                            sizeof(serv_addr))) < 0) {
//     std::cerr << "Error: connection failed." << std::endl;
//     exit(1);
//   }
//
//   // --- Sending the data ---
//   // Send the Handshake packet to the server
//   if (send(sock, data, packet_length, 0) != packet_length) {
//     std::cerr << "Error: failed to send the Handshake packet to the server."
//               << std::endl;
//     exit(1);
//   }
//
//   // Deallocate the buffer. I could've used a std::array, but oh well...
//   free(data);
//
//   // Sends the status request packet
//   if (send(sock, status_request_packet, 2, 0) != 2) {
//     std::cerr
//         << "Error: failed to send the Status Request packet to the server."
//         << std::endl;
//     exit(1);
//   }
//
//   // ----- READING SERVER -----
//
//   // Read packet length (VarInt)
//   uint64_t srv_p_len;
//   if (read_varint(sock, &srv_p_len) < 1) {
//     std::cerr << "Error: failed to read the Status Response length"
//               << std::endl;
//     exit(1);
//   }
//
//   // Read packet ID (VarInt, but 0x00 is still 0x00)
//   uint8_t srv_p_id;
//   if (read(sock, &srv_p_id, 1) < 1) {
//     std::cerr << "Error: failed to read the Status Response ID" << std::endl;
//     exit(1);
//   }
//   if (srv_p_id != 0) {
//     std::cerr << std::format("Error: received Status Response packet ID is "
//                              "incorrect; expected 0x00, got 0x{0:02X}",
//                              srv_p_id);
//     exit(1);
//   }
//
//   uint64_t srv_str_len;
//   if (read_varint(sock, &srv_str_len) < 1) {
//     std::cerr << "Error: failed reading the Status Response string length"
//               << std::endl;
//     exit(1);
//   }
//
//   if (srv_str_len >= srv_p_len) {
//     std::cerr << "Error: Status Response string length is greater than
//     itself; "
//                  "nonsense."
//               << std::endl;
//     exit(1);
//   }
//
//   // Read the JSON string.
//   std::vector<uint8_t> json_data;
//   json_data.reserve(srv_str_len);
//   read_json_string(sock, (size_t)srv_str_len, json_data);
//
//   // Close the socket.
//   close(client_fd);
//
//   // Convert bytes into String.
//   std::string json_string(json_data.begin(), json_data.end());
//
//   return json_string;
// }
