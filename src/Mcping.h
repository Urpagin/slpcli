//
// Created by urpagin on 01/12/2023.
// My first real C++ project
//

#ifndef MCPINGERCPP_MCPING_H
#define MCPINGERCPP_MCPING_H

#include <cstdint>
#include <iostream>
#include <string>

// TODO: UnitTesting
class Mcping {
 private:
  std::string server_addr;
  u_int16_t server_port;
  std::string ip; // Will tried to be resolved in constructor.
  int timeout;

 public:

	// Constructor
	// why the explicit, I don't really know
	explicit Mcping(std::string server_addr, uint16_t server_port = 25565);


	/// Queries the Minecraft notchian server using the Server List Ping protocol and returns a JSON string.
	std::string query_slp();

	/// Resolves a domain name to an IPv4. E.g., google.com -> 83.19.138.40
	std::string resolve_address(const std::string &address);

};

#endif  // MCPINGERCPP_MCPING_H
