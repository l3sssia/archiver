#pragma once
#include <bitset>
#include <iostream>

namespace hamming_code {
template<size_t total_block_bits, size_t information_bits>
class HammingCoder {
 public:
    static void Encode(const char *bytes, char *encoded);
    static bool Decode(const char *encoded, char *decoded);

    static void WriteBit(char* buf, size_t bit_index, bool bit);
    static bool ReadBit(const char* buf, size_t bit_index);
};

template<size_t total_block_bits, size_t information_bits>
bool HammingCoder<total_block_bits, information_bits>::ReadBit(const char *buf, size_t bit_index) {
    size_t byte_index = bit_index / 8;
    size_t offset = bit_index % 8;
    return buf[byte_index] & (1 << offset);
}

template<size_t total_block_bits, size_t information_bits>
void HammingCoder<total_block_bits, information_bits>::WriteBit(char *buf, size_t bit_index, bool bit) {
    size_t byte_index = bit_index / 8;
    size_t offset = bit_index % 8;
    buf[byte_index] &= ~(1 << offset);
    if (bit) {
        buf[byte_index] |= (1 << offset);
    }
}

template<size_t total_block_bits, size_t information_bits>
void HammingCoder<total_block_bits, information_bits>::Encode(const char *bytes, char *encoded) {
    std::fill(encoded, encoded + total_block_bits / 8, 0);
    size_t index = 0;
    for (int i = 0; i < total_block_bits; ++i) {
        if (index < information_bits) {
            if (__builtin_popcount(i + 1) != 1) {
                WriteBit(encoded, i, ReadBit(bytes, index++));
            }
        }
    }
    for (size_t i = 0; i < total_block_bits - 1; i++) {
        index = i + 1;
        size_t check_bit_number = 1;
        while (check_bit_number <= index) {
            if ((check_bit_number & index) != 0) {
                if (ReadBit(encoded, i)) {
                    bool b = ReadBit(encoded, check_bit_number - 1);
                    WriteBit(encoded, check_bit_number - 1, !b);
                }
            }
            check_bit_number <<= 1;
        }
    }
    bool total_parity = false;
    for (size_t i = 0; i < total_block_bits - 1; i++) {
        if (ReadBit(encoded, i)) {
            total_parity = !total_parity;
        }
    }
    WriteBit(encoded, total_block_bits - 1, total_parity);
}

template<size_t total_block_bits, size_t information_bits>
bool HammingCoder<total_block_bits, information_bits>::Decode(const char *encoded, char *decoded) {
    std::fill(decoded, decoded + information_bits / 8, 0);
    bool total_parity_bit = false;
    int error_index = 0;
    for (size_t i = 0; i < total_block_bits - 1; ++i) {
        size_t index = i + 1;
        if (!ReadBit(encoded, i)) {
            continue;
        }
        total_parity_bit ^= 1;
        int mask = 1;
        while (mask <= index) {
            if ((mask & index) != 0) {
                error_index ^= mask;
            }
            mask <<= 1;
        }
    }
    if (total_parity_bit == ReadBit(encoded, total_block_bits -1) && error_index) {
        return false;
    }
    size_t index = 0;
    for (int i = 0; i < total_block_bits - 1; ++i) {
        if (__builtin_popcount(i + 1) == 1) {
            continue;
        }
        if (index < information_bits) {
            bool b = (((i + 1) == error_index) ^ ReadBit(encoded, i));
            WriteBit(decoded, index++, b);
        }
    }
    return true;
}
}

