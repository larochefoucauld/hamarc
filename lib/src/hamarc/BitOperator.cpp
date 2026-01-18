#include "BitOperator.hpp"

// Обратная нумерация бит: 01234567
bool BitOperator::GetBit(uint8_t byte, uint8_t pos) {
    pos = 0b00000111 & ~pos;
    return (byte >> pos) & 1;
}

// Обратная нумерация бит: 01234567
void BitOperator::SetBit(uint8_t& byte, uint8_t pos) {
    pos = 0b00000111 & ~pos;
    byte |= (1 << pos);
}

// Обратная нумерация бит: 01234567
void BitOperator::FlipBit(uint8_t& byte, uint8_t pos) {
    if (GetBit(byte, pos)) {
        byte = ~byte;
        SetBit(byte, pos);
        byte = ~byte;
    } else {
        SetBit(byte, pos);
    }
}

uint8_t BitOperator::Reflect(uint8_t byte) {
    return ((byte & 1) << 7) | (((byte >> 1) & 1) << 6) | (((byte >> 2) & 1) << 5) |
    (((byte >> 3) & 1) << 4) | (((byte >> 4) & 1) << 3) | (((byte >> 5) & 1) << 2) |
    (((byte >> 6) & 1) << 1) | ((byte >> 7) & 1);
}