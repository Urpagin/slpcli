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
    static std::vector<uint8_t> write_var_int(uint64_t value);

    static uint64_t pack_varint(uint32_t number);

    static uint8_t bytes_used(uint32_t num);

    static void insert_bytes_in_data(uint64_t dataByte, uint8_t **data, uint32_t *data_offset);

    static void insert_string_in_data(const std::string &ip, uint8_t **data, uint32_t *data_offset);

};

// test
#endif //MCPINGERCPP_DATATYPESUTILS_H
