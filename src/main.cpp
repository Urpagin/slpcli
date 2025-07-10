#include <boost/asio.hpp>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>
#include "Mcping.h"
#include "DataTypesUtils.h"

using boost::asio::ip::tcp;

#include <iostream>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>

std::string domain_to_ipv4(const std::string& domain);

std::pair<std::string, uint16_t> read_server_address(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    std::pair<std::string, uint16_t> address = read_server_address(argc, argv);
    std::string ip = domain_to_ipv4(address.first);

    std::cout << "Pinging '" << address.first << ":" << address.second << "' (" << ip << ")" << std::endl;

    Mcping serv(ip.c_str(), address.second, 3);

    serv.ping();

    return 0;
}


std::string domain_to_ipv4(const std::string& domain) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipStr[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET for IPv4
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(domain.c_str(), nullptr, &hints, &res)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return "";
    }

    for(p = res; p != nullptr; p = p->ai_next) {
        void *addr;
        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipStr, sizeof ipStr);
        break; // if we just want the first IP
    }

    freeaddrinfo(res); // free the linked listj

    return std::string(ipStr);
}


std::pair<std::string, uint16_t> read_server_address(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Invalid usage: " << argv[0] << " <address>:<port> (port default is 25565)";
        std::exit(-1);
    }

    const std::string address{argv[1]};
    uint16_t port{25565};
    if (argc > 2) {
        port = static_cast<uint16_t>(std::stoi(argv[2]));
    }

    return std::make_pair(address, port);
}
