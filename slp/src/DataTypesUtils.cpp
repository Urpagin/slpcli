//
// Created by Urpagin on 2023-12-24.
//

#include "DataTypesUtils.h"

#include <asio.hpp>
#include <string>

/// https://minecraft.wiki/w/Java_Edition_protocol/Data_types#VarInt_and_VarLong
/// @brief Makes a VarInt from an int.
/// @return A vector of bytes ranging from 1 to 5 bytes inclusive.
std::vector<uint8_t> DataTypesUtils::make_varint(const int value) {
  auto val =
      static_cast<uint32_t>(value); // Don't bother with negative and such.
  std::vector<uint8_t> varint{};    // Byte by Byte vector
  varint.reserve(MAX_VARINT_SIZE);  // Max 5 bytes for VarInt.

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

// TODO: Make read_varint() throw exception instead of returning an int!
// TODO: We're in C++, not C!

/// @brief Tries to read the first VarInt from `sock`. Which corresponds to the
/// Status Response's JSON string size in bytes.
/// @returns The VarInt read from `sock`.
/// @param sock The socket to read from.
/// @throws asio::system_error of code type asio::error::eof if EOF, some other
/// type otherwise.
int DataTypesUtils::read_varint(asio::ip::tcp::socket &sock) {
  static constexpr uint8_t DATA_BITS{0x7F};
  static constexpr uint8_t CONTINUATION_BIT{0x80};
  // A container with at least 5 bytes to hold the VarInt data.
  uint64_t result = 0;
  // The buffer, one byte at a time.
  uint8_t byte = 0;

  for (size_t i{0}; i < 5; ++i) {
    // Throws exception if error or EOF.
    asio::read(sock, asio::buffer(&byte, 1));

    // Must be before the check for the zero-continuation bit.
    // Add the data bits to result.
    result |= (byte & DATA_BITS) << (7 * i);

    // If the continuation bit on the byte is 0, we return.
    if (!(byte & CONTINUATION_BIT)) {
      return static_cast<int>(result);
    }
  }

  // The 5 bytes have continuation bits set to 1: error.
  throw std::runtime_error("Status Request packet's JSON size's bytes have all "
                           "their continuation bit set to 1. Invalid VarInt.");
}



// TODO: Remove?
/// @brief Returns the highest byte that is non-zero. Zero counts as 1.
///
/// E.g., 0x000FF -> 1, 0x000100 -> 2, 0x0000 -> 1
template <std::integral T> size_t DataTypesUtils::highest_byte(T value) {
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
