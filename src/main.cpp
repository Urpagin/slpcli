#include <boost/asio.hpp>
#include <iostream>
#include "Mcping.h"
#include "DataTypesUtils.h"

using boost::asio::ip::tcp;


int main(int argc, char *argv[]) {

    std::cout << "Server ip: ";
    std::string ip;
    std::cin >> ip;
    std::cout << "Port (default is 25565) lqdksjfqkljm juste met in truc random j'ai lea dflellmmme de mettre ça now: ";
    uint16_t port;
    std::cin >> port;
    Mcping serv(ip.c_str(), 25565, 3);

    serv.ping();


    return 0;
}
