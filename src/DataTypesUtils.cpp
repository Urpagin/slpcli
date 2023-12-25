//
// Created by urpagin on 24/12/2023.
//

#include "DataTypesUtils.h"


std::vector<uint8_t> DataTypesUtils::write_var_int(int value) {
    // Code logic from https://wiki.vg/Protocol#VarInt_and_VarLong

    std::vector<uint8_t> var_int{}; // Byte by Byte vector

    while (true) {
        if ((value & ~SEGMENT_BITS) == 0) {
            var_int.emplace_back(static_cast<uint8_t>(value));
            return var_int;
        }

        var_int.emplace_back((value & SEGMENT_BITS) | CONTINUE_BIT);
        // Ensure logical right shift for negative numbers
        value = static_cast<unsigned int>(value) >> 7;
    }
}
