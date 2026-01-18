#ifndef BITOPERATOR_HPP
#define BITOPERATOR_HPP

#include <cstdint>

class BitOperator {
public:
    static bool GetBit(uint8_t byte, uint8_t pos);
    static void SetBit(uint8_t& byte, uint8_t pos);
    static void FlipBit(uint8_t& byte, uint8_t pos);
    static uint8_t Reflect(uint8_t byte);
};

#endif  // BITOPERATOR_HPP
