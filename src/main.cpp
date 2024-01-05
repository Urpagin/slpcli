#include <boost/asio.hpp>
#include <iostream>
#include "Mcping.h"
#include "DataTypesUtils.h"

using boost::asio::ip::tcp;

int main() {

    Mcping serv("116.202.106.62", 25565, 3);

    serv.ping();
  // std::cout << "HEY" << std::endl;
  // std::vector<uint8_t> a = DataTypesUtils::write_var_int(999999);

  // for (uint8_t vec : a) {
  //     std::cout << static_cast<int>(vec) << std::endl;
  // }



    return 0;
}
