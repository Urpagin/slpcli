//
// Created by urpagin on 01/12/2023.
// My first real C++ project
//

#ifndef MCPINGERCPP_MCPING_H
#define MCPINGERCPP_MCPING_H

#include <iostream>
#include <cstdint>

// TODO: UnitTesting
class Mcping {
private:
    static constexpr int TCP_TIMEOUT = 5; // in seconds

    std::string server_addr;
    u_int16_t server_port;
    int timeout;


public:
    // Constructor
    explicit Mcping(std::string server_addr, uint16_t server_port = 25565,
                    int timeout = TCP_TIMEOUT); // why the explicit, I don't really know

    void ping();

};


#endif //MCPINGERCPP_MCPING_H
