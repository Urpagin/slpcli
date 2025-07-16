//
// Created by Urpagin on 2023-12-24.
//

#include "DataTypesUtils.h"

#include <cstdint>
#include <iostream>
#include <string>


/// https://minecraft.wiki/w/Java_Edition_protocol/Data_types#VarInt_and_VarLong
/// @brief Makes a VarInt from an int.
/// @return A vector of bytes ranging from 1 to 5 bytes inclusive.
std::vector<uint8_t> DataTypesUtils::make_varint(const int value) {
  auto val = static_cast<uint32_t>(value); // Don't bother with negative and such.
  std::vector<uint8_t> varint{}; // Byte by Byte vector
  varint.reserve(MAX_VARINT_SIZE); // Max 5 bytes for VarInt.

  while (val & (~DATA_BITS)) {
    // From the LSB's side.
    varint.push_back((val & DATA_BITS) | CONTINUE_BIT);
    val >>= 7;
  }
  varint.push_back(val & DATA_BITS);
  return varint;
}


/// https://minecraft.wiki/w/Java_Edition_protocol/Data_types#Type:String
/// @brief Makes an MC Protocol String.
/// @details Format: [VarInt of the size in bytes of the string, string bytes]
std::vector<uint8_t> DataTypesUtils::make_string(const std::string_view s) {
  const auto str = std::string(s);
  const auto varint = DataTypesUtils::make_varint(static_cast<int>(str.size()));
  std::vector<uint8_t> data;
  data.reserve(varint.size() + str.size());
  data.insert(data.end(), varint.begin(), varint.end());
  data.insert(data.end(), str.begin(), str.end());
  return data;
}


/// @brief Returns the highest byte that is non-zero. Zero counts as 1.
///
/// E.g., 0x000FF -> 1, 0x000100 -> 2, 0x0000 -> 1
template<std::integral T>
size_t DataTypesUtils::highest_byte(T value) {
  using U = std::make_unsigned_t<T>;
  // Get the unsigned version to not have weird two's complement shenanigans.
  U val = static_cast<U>(value);

  size_t highest{0};
  do {
    ++highest;
    val >>= 8;
  } while (val);

  return highest;
}
