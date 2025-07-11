#include <arpa/inet.h>
#include <netdb.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <string>
#include <utility>

#include <string_view>
#include "DataTypesUtils.h"
#include "Mcping.h"

std::string domain_to_ipv4(const std::string &domain);

std::pair<std::string, uint16_t> read_server_address(int argc, char *argv[]);


/// Silences ALL writes to the stdout and stderr streams.
void silence_stdout_stderr() {
  std::cout.setstate(std::ios::failbit);
  std::cerr.setstate(std::ios::failbit);
}

/// Unsilences ALL writes to the stdout and stderr streams.
void unsilence_stdout_stderr() {
    std::cout.clear(std::cout.rdstate() & ~std::ios::failbit);
    std::cerr.clear(std::cerr.rdstate() & ~std::ios::failbit);
}

/// Returns true if the program's been called with the --quiet|-q flag.
bool is_quiet(int argc, char* argv[]) {
    for (int i{1}; i < argc; ++i) {
        std::string_view arg{argv[i]};
        if (arg == "-q" || arg == "--quiet") return true;
    }
    return false;
}


int main(int argc, char *argv[]) {
  if (is_quiet(argc, argv)) silence_stdout_stderr();

  std::pair<std::string, uint16_t> address = read_server_address(argc, argv);
  std::string ip = domain_to_ipv4(address.first);
  if (ip.empty()) {
    std::cerr << "Error: failed to resolve address" << std::endl;
    exit(1);
  }

  std::cout << "Querying '" << address.first << ":" << address.second
            << "'... (" << ip << ")\n\n" << std::endl;

  Mcping serv(ip.c_str(), address.second);

  std::string slp_response{serv.query_slp()};

  unsilence_stdout_stderr();
  std::cout << slp_response << std::endl;

  return 0;
}

std::string domain_to_ipv4(const std::string &domain) {
  struct addrinfo hints, *res, *p;
  int status;
  char ipStr[INET_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;  // AF_INET for IPv4
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(domain.c_str(), nullptr, &hints, &res)) != 0) {
    std::cerr << "Error: getaddrinfo: " << gai_strerror(status) << std::endl;
    return "";
  }

  for (p = res; p != nullptr; p = p->ai_next) {
    void *addr;
    // get the pointer to the address itself,
    // different fields in IPv4 and IPv6:
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    addr = &(ipv4->sin_addr);

    // convert the IP to a string and print it:
    inet_ntop(p->ai_family, addr, ipStr, sizeof ipStr);
    break;  // if we just want the first IP
  }

  freeaddrinfo(res);  // free the linked listj

  return std::string(ipStr);
}



// `argc`,`argv` come from `main(int argc, char* argv[])`
std::vector<char*> make_argv_without_quiet(int argc, char* argv[])
{
    std::vector<char*> filtered;
    filtered.reserve(argc);           // at most `argc` entries

    filtered.push_back(argv[0]);      // keep the program name

    for (int i = 1; i < argc; ++i)
    {
        std::string_view a{argv[i]};
        if (a == "-q" || a == "--quiet")   // skip the “quiet” flags
            continue;
        filtered.push_back(argv[i]);
    }

    filtered.push_back(nullptr);      // for Unix-style `argv` termination
    return filtered;                  // `filtered.size()-1` is the new argc
}

std::pair<std::string, uint16_t> read_server_address(int argc_, char *argv_[]) {
  auto new_argv_vec = make_argv_without_quiet(argc_, argv_);
  int  argc     = static_cast<int>(new_argv_vec.size()) - 1;
  char** argv   = new_argv_vec.data();

  std::string usage_str = std::format(
      "Error: incorrect usage.\n\nUsages:\n\t{} "
      "<address>:<port>\n\n\tor\n\n\t{} <address> <port>\n\nInfo:\n\tDefault "
      "port is 25565.",
      argv[0], argv[0]);

  if (argc < 2) {
    std::cerr << usage_str << std::endl;
    std::exit(-1);
  }

  // Delimiter for case "<addr>:<port>"
  const char *DELIMITER = ":";
  // Default port
  uint16_t port{25565};



  try {
    if (argc < 3) {
      // We assume: ./program ip:port
      std::string arg1 = std::string(argv[1]);

      std::string addr = arg1.substr(0, arg1.find(DELIMITER));
      std::string port_s = arg1.substr(arg1.find(DELIMITER) + 1, arg1.size());

      // If "./program addr" with no port whatsoever.
      if (port_s == addr) port_s.clear();

      if (port_s.size()) port = static_cast<uint16_t>(std::stoi(port_s));
      return std::make_pair(addr, port);

    } else {
      // We assume: ./program ip port garbage args
      const std::string addr{argv[1]};
      port = static_cast<uint16_t>(std::stoi(argv[2]));

      return std::make_pair(addr, port);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: failed to parse input arguments.\nDetail: " << e.what()
              << std::endl;
    exit(1);
  }
}
