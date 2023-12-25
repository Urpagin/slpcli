//
// Created by urpagin on 24/12/2023.
//

#ifndef MCPINGERCPP_DATATYPESUTILS_H
#define MCPINGERCPP_DATATYPESUTILS_H

#include <cstdint>
#include <vector>


class DataTypesUtils {
private:
    // Private constructor to prevent instantiation. It's a utility class
    DataTypesUtils() = default;

    static constexpr int SEGMENT_BITS = 0x7F; // 127 in decimal
    static constexpr int CONTINUE_BIT = 0x80; // 128 in decimal

public:
    static std::vector<uint8_t> write_var_int(int value);

    // function taken from https://github.com/LhAlant/MinecraftSLP/blob/main/MinecraftSLP.c
    static uint64_t pack_varint(uint32_t number);

    static uint8_t bytes_used(uint32_t number);
};


#endif //MCPINGERCPP_DATATYPESUTILS_H
