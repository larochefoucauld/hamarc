#include "Decoder.hpp"
#include "Encoder.hpp"
#include "BitOperator.hpp"


Decoder::ValidationResult Decoder::Validate(std::fstream& msg, size_t raw_msg_size) {
    size_t code_bit_size = Encoder::GetCodeBitSize(raw_msg_size * 8);
    size_t code_size = code_bit_size / 8 + 1;
    std::streampos start_pos = msg.tellg();

    uint8_t* code_1 = Encoder::GetCode(msg, raw_msg_size);
    uint8_t* code_2 = new uint8_t[code_size];
    msg.read(reinterpret_cast<char*>(code_2), code_size);
    msg.seekg(start_pos, std::fstream::beg);
    bool parity_bit_1 = Encoder::GetMsgParityBit(msg, raw_msg_size * 8 + code_bit_size);
    bool parity_bit_2 = BitOperator::GetBit(code_2[code_size - 1], code_bit_size % 8);
    msg.seekg(start_pos, std::fstream::beg);

    size_t error_bit_pos_ = 0;
    // Проверка контрольных бит без учёта общего бита чётности
    if (BitOperator::GetBit(code_1[code_size - 1], code_bit_size % 8) != parity_bit_2) {
        BitOperator::FlipBit(code_1[code_size - 1], code_bit_size % 8);
    }
    for (size_t i = 0; i < code_size; ++i) {
        error_bit_pos_ |= (BitOperator::Reflect(code_1[i] ^ code_2[i]) << (i * 8));
    }

    if (error_bit_pos_ == 0 && parity_bit_1 == parity_bit_2) {
        delete [] code_1;
        delete [] code_2;
        return ValidationResult::kValid;
    }
    if (error_bit_pos_ != 0 && parity_bit_1 == parity_bit_2) {
        delete [] code_1;
        delete [] code_2;
        return ValidationResult::kDoubleError;
    }
    if (error_bit_pos_ == 0 && parity_bit_1 != parity_bit_2) {
        FixBit(msg, raw_msg_size * 8 + code_bit_size);
        delete [] code_1;
        delete [] code_2;
        return ValidationResult::kSingleErrorFixed;
    }

    size_t log = 0;
    for (size_t kMax2Deg = 64; log < kMax2Deg; ++log) {
        if (error_bit_pos_ <= (1 << log)) {
            break;
        }
    }

    if (error_bit_pos_ == (1 << log)) {
        FixBit(msg, raw_msg_size * 8 + log);
        delete [] code_1;
        delete [] code_2;
        return ValidationResult::kSingleErrorFixed;
    }

    FixBit(msg, error_bit_pos_ - log - 1);
    delete [] code_1;
    delete [] code_2;
    return ValidationResult::kSingleErrorFixed;
}

void Decoder::FixBit(std::fstream& msg, std::streamoff error_bit_pos) {
    std::streampos start_pos = msg.tellg();
    uint8_t* buf = new uint8_t[1];

    msg.seekg(error_bit_pos / 8, std::fstream::cur);
    msg.read(reinterpret_cast<char*>(buf), 1);
    BitOperator::FlipBit(buf[0], error_bit_pos % 8);
    msg.seekg(-1, std::fstream::cur);
    msg.write(reinterpret_cast<char*>(buf), 1);
    msg.flush();
    msg.seekg(start_pos, std::fstream::beg);

    delete [] buf;
}
