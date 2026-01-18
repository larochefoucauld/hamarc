#include <gtest/gtest.h>

#include "hamarc/Decoder.hpp"
#include "hamarc/Copydata.hpp"
#include "hamarc/BitOperator.hpp"
#include "FileComparator.hpp"

static const std::filesystem::path TestingDir{"./tests/data/decoder_test"};

static FileComparator fc(TestingDir);

static void MakeBitError(std::fstream& msg, std::streamoff error_bit_pos) {
    uint8_t* buf = new uint8_t[1];
    msg.read(reinterpret_cast<char*>(buf), 1);
    BitOperator::FlipBit(buf[0], error_bit_pos);
    msg.seekg(-1, std::fstream::cur);
    msg.write(reinterpret_cast<char*>(buf), 1);

    delete [] buf;
}

static void MakeCopy(std::filesystem::path file, std::streamoff start_pos, size_t data_size) {
    std::ifstream in(TestingDir / file, std::fstream::binary);
    in.seekg(start_pos, std::ifstream::beg);
    std::ofstream copy(TestingDir / "tmp.txt", std::fstream::trunc);
    Copydata::CopyData(in, copy, data_size);
    in.close();
    copy.close();
}

static void MakeErrors(std::filesystem::path file, size_t err_pos_1, size_t err_pos_2) {
    std::fstream stream(TestingDir / file, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (err_pos_1 != -1) {
        stream.seekg(err_pos_1 / 8, std::fstream::cur);
        MakeBitError(stream, err_pos_1 % 8);
    }
    stream.seekg(0, std::fstream::beg);
    if (err_pos_2 != -1) {
        stream.seekg(err_pos_2 / 8, std::fstream::cur);
        MakeBitError(stream, err_pos_2 % 8);
    }
    stream.close();
}



class ValidationTestSuite 
    : public testing::TestWithParam<
        std::tuple<
            std::filesystem::path,         // file
            size_t,                        // from
            size_t,                        // data size (byte)
            size_t,                        // code size (bit)
            size_t,                        // error 1
            size_t                         // error 2
        >
    >
{};

TEST_P(ValidationTestSuite, ValidationTest) {
    Decoder dec;
    MakeCopy
    (
        std::get<0>(GetParam()), std::get<1>(GetParam()), 
        std::get<2>(GetParam()) + (std::get<3>(GetParam()) - 1) / 8 + 1
    );

    size_t err_1 = std::get<4>(GetParam());
    size_t err_2 = std::get<5>(GetParam());
    MakeErrors("tmp.txt", err_1, err_2);
    std::fstream stream(TestingDir / "tmp.txt", std::fstream::in | std::fstream::out | std::fstream::binary);
    Decoder::ValidationResult exit_code = Decoder::Validate(stream, std::get<2>(GetParam()));
    stream.close();
    
    if (err_1 == -1 && err_2 == -1) {
        ASSERT_EQ(exit_code, Decoder::ValidationResult::kValid);
    } else if (err_1 != -1 && err_2 != -1) {
        ASSERT_EQ(exit_code, Decoder::ValidationResult::kDoubleError);
    } else {
        ASSERT_EQ(exit_code, Decoder::ValidationResult::kSingleErrorFixed);
    }

    if (err_1 == -1 || err_2 == -1) {
        ASSERT_TRUE(fc.Equals(std::get<0>(GetParam()), "tmp.txt"));
    }
    
    std::filesystem::remove(TestingDir / "tmp.txt");
}

INSTANTIATE_TEST_SUITE_P(
    Group,
    ValidationTestSuite,
    testing::Values(
        std::make_tuple(
            "in_1.txt",
            0,
            2,
            6,
            -1,
            -1
        ),
        std::make_tuple(
            "in_1.txt",
            0,
            2,
            6,
            1,
            -1
        ),
        std::make_tuple(
            "in_1.txt",     
            0,
            2,
            6,
            21,             
            -1
        ),
        std::make_tuple(
            "in_1.txt",
            0,
            2,
            6,
            10,             
            4 
        ),
        std::make_tuple(
            "in_1.txt",
            0,
            2,
            6,
            -1,             
            13 
        ),
        std::make_tuple(
            "in_2.txt",
            0,
            32,
            10,
            100,
            -1
        ),
        std::make_tuple(
            "in_2.txt",
            0,
            32,
            10,
            8 * 32 + 10 - 1,
            -1
        ),
        std::make_tuple(
            "in_2.txt",
            0,
            32,
            10,
            8 * 32 + 10 - 1,
            8 * 32 + 3 - 1
        ),
        std::make_tuple(
            "in_2.txt",
            0,
            32,
            10,
            8 * 32 + 4 - 1,
            8 * 32 + 3 - 1
        )
    )
);

/*
'in_1.txt':
10010000 01111111 00101100
- 2 data bytes, 5 + 1 control bits

'in_2.txt':
11001000 00111111 10101001 00100110 10101110 11011011 10100111 11100100
00000001 01000110 11000000 10010110 00111110 00101011 00011110 00101110
10001000 11111111 11101000 00100101 01010111 00100001 01001011 00001101
01110110 10110110 01000110 11111101 00000001 10110000 01010110 01000100
01111101 00000000
- 32 data bytes, 9 + 1 control bits
*/
