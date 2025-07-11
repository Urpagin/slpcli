//
// Created by urpagin on 2023-12-24.
//

#include "DataTypesUtils.h"
#include <string>

// inspiration from
// https://github.com/LhAlant/MinecraftSLP/blob/main/MinecraftSLP.c

std::vector<uint8_t> DataTypesUtils::write_var_int(uint64_t value) {
  // Code logic from https://wiki.vg/Protocol#VarInt_and_VarLong

  std::vector<uint8_t> var_int{}; // Byte by Byte vector

  while (true) {
    if ((value & ~SEGMENT_BITS) == 0) {
      var_int.emplace_back(static_cast<uint8_t>(value));
      return var_int;
    }

    var_int.emplace_back((value & SEGMENT_BITS) | CONTINUE_BIT);
    // Ensure logical right shift for negative numbers
    value = value >> 7;
  }
}

// TODO: read and thoroughly check the func
uint64_t DataTypesUtils::pack_varint(uint32_t number) {
  /* Function to transform integers into variable size integer. Maximum size in
  effective bytes is 5. More details :
  https://wiki.vg/Protocol#VarInt_and_VarLong */

  uint64_t varint = 0;
  while (true) {
    varint <<= 8;

    uint8_t tmp = number & 0x7F; // 0b01111111
    number >>= 7;

    varint += tmp;
    if (number != 0) {
      varint |= 0x80; // 0b10000000
    } else
      break;
  }
  return varint;
}

uint8_t DataTypesUtils::bytes_used(uint32_t num) {
  // Counts how many non-zero bytes are needed to represent a number.
  // Example: 0x000000FF → 1, 0x00000100 → 2
  uint8_t bytes = 0;
  while (num != 0) {
    ++bytes;
    num >>= 8;
  }
  return bytes == 0 ? 1 : bytes; // Ensures 0 still counts as 1 byte.
}

void DataTypesUtils::insert_bytes_in_data(uint64_t dataByte, uint8_t **data,
                                          uint32_t *data_offset) {
  /* Inserts bytes one at a time in the data variable, and begins by the MSB */
  for (int64_t i = DataTypesUtils::bytes_used(dataByte) - 1; i >= 0; i--) {
    // Similar to data[*data_offset] in high-level structures like std::vector,
    // but using pointer arithmetic.
    *(*data + *data_offset) =
        (dataByte >> (i * 8)) & (0xFF); // Selects the right byte to put in data
    (*data_offset)++;
  }
}

void DataTypesUtils::insert_string_in_data(const std::string &ip,
                                           uint8_t **data,
                                           uint32_t *data_offset) {
  // Inserts a string into the data variable
  for (const char &c : ip) {
    *(*data + *data_offset) = c;
    ++(*data_offset);
  }
}
