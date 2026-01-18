#include <gtest/gtest.h>

#include "hamarc/Encoder.hpp"
#include "FileComparator.hpp"

static const std::filesystem::path TestingDir{"./tests/data/encoder_test"};

static FileComparator fc(TestingDir);


class EncodingTestSuite 
    : public testing::TestWithParam<
        std::tuple<
            std::filesystem::path,  // file
            size_t,                 // from
            size_t,                 // size
            std::filesystem::path,  // encoded
            Encoder::EncodingResult // exit code
        >
    >
{};

TEST_P(EncodingTestSuite, EncodingTest) {
    Encoder enc;
    std::ifstream in(TestingDir / std::get<0>(GetParam()), std::ios::binary);
    in.seekg(std::get<1>(GetParam()));
    std::ofstream out(TestingDir / "tmp.txt", std::ios::trunc);

    Encoder::EncodingResult exit_code = enc.EncodeAndWrite(in, out, std::get<2>(GetParam()));
    out.close();

    ASSERT_TRUE(fc.Equals(std::get<3>(GetParam()), "tmp.txt"));
    ASSERT_EQ(exit_code, std::get<4>(GetParam()));
    
    std::filesystem::remove(TestingDir / "tmp.txt");
}

INSTANTIATE_TEST_SUITE_P(
    Group,
    EncodingTestSuite,
    testing::Values(
        std::make_tuple(
            "in_1.txt",     // 10010000 01111111
            0,
            2,
            "out_1.txt",    // 00101100
            Encoder::EncodingResult::kSuccess
        ),
        std::make_tuple(
            "in_2.txt",     // 01010111 00100001 01001011 00001101
            0,
            4,
            "out_2.txt",    // 00011110
            Encoder::EncodingResult::kSuccess
        ),
        std::make_tuple(
            "in_3.txt",     // 11000111 11000101 01100011
            0,
            3,
            "out_3.txt",    // 00000100
            Encoder::EncodingResult::kSuccess
        ),
        std::make_tuple(
            "in_4.txt",     // 00000001 10111011 10110001 10000010
            2,
            1,
            "out_4.txt",    // 01111000
            Encoder::EncodingResult::kSuccess
        ),
        std::make_tuple(
            "in_5.txt",     // 10101101 00100010 00011100 01001111 00110001
            1,
            10,
            "out_5.txt",    // 11110100
            Encoder::EncodingResult::kReaderEOFReached
        ),
        std::make_tuple(
            "in_6.txt",     // 10100011 11101100 00110100 00001001 
                            // 11111111 00110010 10111100 01000001 
                            // 00011000 01000101 11110010 10010001 
                            // 01011000 10100010 10001110 10101000
            0,
            16,
            "out_6.txt",    // 11110111 10000000
            Encoder::EncodingResult::kSuccess
        )
    )
);
