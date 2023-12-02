#include <boost/asio.hpp>
#include <iostream>
#include "Mcping.h"

using boost::asio::ip::tcp;

int main() {

    Mcping serv("e", 4, 3);


    return 0;
}
