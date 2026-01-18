#include "Encoder.hpp"

const size_t Encoder::kMaxBufferSize = 8192;

void Encoder::ShiftAndRead(
    std::istream& reader, std::streamoff offset, 
    size_t data_size, char* buf) {

    reader.seekg(offset, std::ios::cur);
    reader.read(buf, data_size);
    reader.seekg(-(offset + data_size), std::ios::cur);
}

bool Encoder::GetByteParityBit(uint8_t byte, size_t number_of_bits) {
    bool res = false;
    for (size_t i = 0; i < number_of_bits; ++i) {
        res ^= BitOperator::GetBit(byte, i);
    }

    return res;
}

size_t Encoder::GetCodeBitSize(size_t msg_bit_size) {
    if (msg_bit_size == 0) {
        return 0;
    }
    size_t code_bit_size = 1;
    while ((static_cast<size_t>(1) << code_bit_size) - code_bit_size - 1 < msg_bit_size) {
        ++code_bit_size;
    }

    return code_bit_size;
}

bool Encoder::GetMsgParityBit(std::istream& reader, size_t msg_bit_size) {
    size_t msg_size = msg_bit_size / 8;
    if (msg_size == 0) {
        uint8_t* buffer = new uint8_t[1];
        reader.read(reinterpret_cast<char*>(buffer), 1);
        bool res = GetByteParityBit(buffer[0], msg_bit_size);
        delete [] buffer;

        return res;
    }

    size_t buffer_size = std::min(msg_size, kMaxBufferSize);
    uint8_t* buffer = new uint8_t[buffer_size];
    size_t start_pos = reader.tellg();

    uint8_t xor_byte = 0;
    for (
        size_t pos = start_pos; 
        pos - start_pos < msg_size; 
        pos = reader.tellg()
        ) {
        
        size_t to_read = std::min(buffer_size, msg_size - (pos - start_pos));
        reader.read(reinterpret_cast<char*>(buffer), to_read);
        for (size_t i = 0; i < to_read; ++i) {
            xor_byte ^= buffer[i];
        }
    }

    bool res = GetByteParityBit(xor_byte, 8);
    if (msg_bit_size % 8 != 0) {
        reader.read(reinterpret_cast<char*>(buffer), 1);
        res ^= GetByteParityBit(buffer[0], msg_bit_size % 8);
    }

    delete [] buffer;

    return res;
}

bool Encoder::GetMsgParityBit(const uint8_t* msg, size_t msg_bit_size) {
    size_t msg_size = msg_bit_size / 8;
    if (msg_size == 0) {
        bool res = GetByteParityBit(msg[0], msg_bit_size);
        return res;
    }

    uint8_t xor_byte = 0;
    for (size_t i = 0; i < msg_size; ++i) {
        xor_byte ^= msg[i];
    }
    bool res = GetByteParityBit(xor_byte, 8);
    if (msg_bit_size % 8 != 0) {
        res ^= GetByteParityBit(msg[msg_size - 1], msg_bit_size % 8);
    }

    return res;
} 


uint8_t* Encoder::GetCode(std::istream& reader, size_t raw_msg_size) {
    size_t code_bit_size = GetCodeBitSize(raw_msg_size * 8);
    size_t code_size = code_bit_size / 8 + 1;
    uint8_t* control_bytes = new uint8_t[code_size]{};
    
    bool parity_bit = false;
    size_t encoded_msg_bit_size = code_bit_size + raw_msg_size * 8;
    size_t start_pos = reader.tellg();
    char* buf = new char[1];

    // i - индекс контрольного бита в блоке контрольных бит (формат <сообщение><контроль>)
    // i_ - индекс контрольного бита при вставке контрольных бит в сообщение
    // j и j_ - индексы информационных бит в тех же форматах
    // Индексация ind начинается с 0, ind_ - с 1
    // Соотношения: i_ = 2^i; j = j_ - round(log2(j_), up) - 1
    for (size_t i = 0; i < code_bit_size; ++i) {
        bool control_bit = 0;
        size_t i_ = (1 << i);
        for (
             size_t j_ = i_ + 1, log_j_ = i + 1; 
             j_ <= encoded_msg_bit_size; j_ += i_
            ) {
            
            while (j_ <= encoded_msg_bit_size && (j_ & i_) == i_) {
                while ((1 << log_j_) < j_) {
                    ++log_j_;
                }
                std::streamoff j = j_ - log_j_ - 1;
                ShiftAndRead(reader, j / 8, 1, buf);
                control_bit ^= BitOperator::GetBit(static_cast<uint8_t>(buf[0]), j % 8);
                ++j_;
            }
        }
        if (control_bit) {
            BitOperator::SetBit(control_bytes[i / 8], i % 8);
        }
        parity_bit ^= control_bit;
    }
    delete [] buf;

    parity_bit ^= GetMsgParityBit(reader, raw_msg_size * 8);
    if (parity_bit) {
        BitOperator::SetBit(control_bytes[code_size - 1], code_bit_size % 8);
    }

    return control_bytes;
}

uint8_t* Encoder::GetCode(const uint8_t* msg, size_t raw_msg_size) {
    size_t code_bit_size = GetCodeBitSize(raw_msg_size * 8);
    size_t code_size = code_bit_size / 8 + 1;
    uint8_t* control_bytes = new uint8_t[code_size]{};
    
    bool parity_bit = false;
    size_t encoded_msg_bit_size = code_bit_size + raw_msg_size * 8;

    uint8_t cur_byte = 0;
    // i - индекс контрольного бита в блоке контрольных бит (формат <сообщение><контроль>)
    // i_ - индекс контрольного бита при вставке контрольных бит в сообщение
    // j и j_ - индексы информационных бит в тех же форматах
    // Индексация ind начинается с 0, ind_ - с 1
    // Соотношения: i_ = 2^i; j = j_ - round(log2(j_), up) - 1
    for (size_t i = 0; i < code_bit_size; ++i) {
        bool control_bit = 0;
        size_t i_ = (1 << i);
        for (
             size_t j_ = i_ + 1, log_j_ = i + 1; 
             j_ <= encoded_msg_bit_size; j_ += i_
            ) {
            
            while (j_ <= encoded_msg_bit_size && (j_ & i_) == i_) {
                while ((1 << log_j_) < j_) {
                    ++log_j_;
                }
                size_t j = j_ - log_j_ - 1;
                control_bit ^= BitOperator::GetBit(static_cast<uint8_t>(msg[j / 8]), j % 8);
                ++j_;
            }
        }
        if (control_bit) {
            BitOperator::SetBit(control_bytes[i / 8], i % 8);
        }
        parity_bit ^= control_bit;
    }
    parity_bit ^= GetMsgParityBit(msg, raw_msg_size * 8);
    if (parity_bit) {
        BitOperator::SetBit(control_bytes[code_size - 1], code_bit_size % 8);
    }

    return control_bytes;
}


Encoder::EncodingResult Encoder::EncodeAndWrite(
    std::istream& reader, std::ostream& writer, size_t raw_msg_size) {
    
    if (!reader.good()) {
        return EncodingResult::kReaderCorrupted;
    }
    if (raw_msg_size == 0) {
        return EncodingResult::kSuccess;
    }
    
    EncodingResult exit_code = EncodingResult::kSuccess;
    size_t start_pos = reader.tellg();
    reader.seekg(0, std::ios::end);
    size_t tail_size = static_cast<size_t>(reader.tellg()) - start_pos;
    reader.seekg(start_pos, std::ios::beg);
    if (tail_size < raw_msg_size) {
        exit_code = EncodingResult::kReaderEOFReached;
        raw_msg_size = tail_size;
        if (raw_msg_size == 0) {
            return exit_code;
        }
    }

    uint8_t* control_bytes = GetCode(reader, raw_msg_size);
    writer.write
    (
        reinterpret_cast<char*>(control_bytes), 
        GetCodeBitSize(raw_msg_size * 8) / 8 + 1
    );
    delete [] control_bytes;

    return exit_code;
}

Encoder::EncodingResult Encoder::EncodeAndWrite(
    const uint8_t* msg, std::ostream& writer, size_t raw_msg_size) {
    
    if (raw_msg_size == 0) {
        return EncodingResult::kSuccess;
    }
    uint8_t* control_bytes = GetCode(msg, raw_msg_size);
    writer.write
    (
        reinterpret_cast<char*>(control_bytes), 
        GetCodeBitSize(raw_msg_size * 8) / 8 + 1
    );
    delete [] control_bytes;

    return EncodingResult::kSuccess;
}
