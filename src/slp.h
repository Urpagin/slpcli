//
// Created by Urpagin on 01/12/2023.
// My first real C++ project
//

#ifndef SLP_H
#define SLP_H

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

/// Implementation of a simple Server List Ping protocol for Minecraft: Java Edition.
///
/// Source: https://minecraft.wiki/w/Java_Edition_protocol/Server_List_Ping
class slp {
private:
  std::string_view server_addr;
  uint16_t server_port;
  std::string ip; // Will tried to be resolved in constructor.
  int timeout;

  /// Resolves a domain name to an IPv4. E.g., google.com -> 83.19.138.40
  std::string resolve_address(std::string_view address);

  /// Takes Protocol Version (int -> VarInt)
  std::vector<uint8_t> make_handshake_packet(int);
  std::vector<uint8_t> make_status_request_packet();
  /// Takes sock (int)
  std::vector<uint8_t> read_status_response_packet(int);


public:

	// Constructor
	// why the explicit, I don't really know
	explicit slp(std::string_view server_addr, uint16_t server_port = 25565, int timeout = 3);

	/// Queries the Minecraft notchian server using the Server List Ping protocol and returns a JSON string.
	std::string query_slp();
};

#endif  // SLP_H
