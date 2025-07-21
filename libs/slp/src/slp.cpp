//
// Created by Urpagin on 2023-12-01.
//

#include "../include/slp.h"
#include <arpa/inet.h>

#include <algorithm>
#include <array>

#include <chrono>
#include <future>
#include <iostream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "DataTypesUtils.h"

// Constructor definition
slp::slp(const std::string_view server_addr, const uint16_t server_port,
         const int timeout, const int handshake_protocol_version)
    : server_addr(server_addr), server_port(server_port), timeout(timeout),
      protocol_version(handshake_protocol_version) {}

/// @brief Builds the Handshake packet.
std::vector<uint8_t> slp::make_handshake_packet() const {
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
  const auto protocol_version = dtu::make_varint(this->protocol_version);
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
  packet.insert(packet.end(), packet_size_varint.begin(),
                packet_size_varint.end());
  packet.insert(packet.end(), packet_id.begin(), packet_id.end());
  packet.insert(packet.end(), protocol_version.begin(), protocol_version.end());
  packet.insert(packet.end(), server_address.begin(), server_address.end());
  packet.insert(packet.end(), server_port.begin(), server_port.end());
  packet.insert(packet.end(), next_state.begin(), next_state.end());

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
    // Read the packet size.
    dtu::read_varint(sock);
    // Read the packet ID
    if (const int packet_id = dtu::read_varint(sock); packet_id != 0x00) {
      throw std::runtime_error("Error: Status Response packet ID is not 0x00");
    }
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

  return std::move(socket);
}

/// @brief Queries the Minecraft server using the Status List Ping protocol and
/// returns the JSON response.
/// @returns The JSON string of the Status Response packet.
/// @throws asio::system_error For any errors encountered.
std::string slp::_query_slp() const {
  // 'Build Handshake' and 'Status Request' packets.
  auto handshake_packet = make_handshake_packet();
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

/// @brief Queries the Minecraft server using the Status List Ping protocol and
/// returns the JSON response.
/// @returns The JSON string of the Status Response packet.
/// @throws asio::system_error For any errors encountered.
/// @details Small wrapper over the real function to add a timeout.
/// Source: https://stackoverflow.com/a/51850018
std::string slp::query_slp() const {
  auto fut =
      std::async(std::launch::async, [&]() { return this->_query_slp(); });

  switch (fut.wait_for(std::chrono::seconds{this->timeout})) {
  case std::future_status::deferred:
    //... should never happen with std::launch::async
    break;
  case std::future_status::ready:
    return fut.get();
    break;
  case std::future_status::timeout:
    std::cerr << "Error: timeout of " << this->timeout << "s exceeded."
              << std::endl;
    throw;
  }

  return {""};
}
