#include <gtest/gtest.h>
#include <filesystem>
#include <cstdint>

#include "hamarc/HamArchiver.hpp"
#include "hamarc/Copydata.hpp"
#include "hamarc/FileOperator.hpp"
#include "FileComparator.hpp"

static const std::filesystem::path TestingDir{"./tests/data/hamarchiver_test"};
static FileOperator fo(TestingDir);
static FileComparator fc(TestingDir);

static void MakeBitError(std::fstream& msg, std::streamoff error_bit_pos) {
    msg.seekg(error_bit_pos / 8, std::fstream::beg);
    uint8_t* buf = new uint8_t[1];
    msg.read(reinterpret_cast<char*>(buf), 1);
    BitOperator::FlipBit(buf[0], error_bit_pos % 8);
    msg.seekg(-1, std::fstream::cur);
    msg.write(reinterpret_cast<char*>(buf), 1);
    msg.flush();
    msg.seekg(0, std::fstream::beg);

    delete [] buf;
}


static void MakeErrors(std::filesystem::path file, const std::vector<size_t> errors) {
    std::fstream stream(TestingDir / file, std::fstream::in | std::fstream::out | std::fstream::binary);
    for (size_t i = 0; i < errors.size(); ++i) {
        // ошибка в случайном бите указанного байта
        MakeBitError(stream, errors[i] * 8 + (rand() % 8));
    }
}

class PackingTestSuite 
    : public testing::TestWithParam<
        std::tuple<
            std::vector<HamArchiver::FileMetadata>,     // raw (name, _, encoding_block_size)
            std::vector<size_t>,                        // errors (raw file)
            std::vector<HamArchiver::ExtractionResult>  // exit codes
        >
    >
{};

TEST_P(PackingTestSuite, PackingTest) {
    HamArchiver harchiver(TestingDir);
    const std::vector<HamArchiver::FileMetadata> file_list = std::get<0>(GetParam());
    fo.CreateDir("tmp");
    harchiver.Create("tmp/testarc.haf", file_list);
    MakeErrors("tmp/testarc.haf", std::get<1>(GetParam()));

    harchiver.SetDir(TestingDir / "tmp");
    auto exit_codes = harchiver.ExtractFiles("testarc.haf");
    const std::vector<HamArchiver::ExtractionResult> expected = std::get<2>(GetParam());

    ASSERT_EQ(exit_codes.size(), expected.size());
    for (size_t i = 0; i < exit_codes.size(); ++i) {
        ASSERT_EQ
        (
            static_cast<int>(exit_codes[i]), 
            static_cast<int>(expected[i])
        );
    }

    for (size_t i = 0; i < file_list.size(); ++i) {
        if (expected[0] == HamArchiver::ExtractionResult::kArcCorrupted 
        || expected[i] == HamArchiver::ExtractionResult::kFileCorrupted) {
            continue;
        }
        ASSERT_TRUE(fo.FileExists("tmp" / file_list[i].path));
        ASSERT_TRUE(fc.Equals(file_list[i].path, "tmp" / file_list[i].path));
    }
    fo.DeleteDir("tmp");
}

INSTANTIATE_TEST_SUITE_P(
    Group,
    PackingTestSuite,
    testing::Values(
        std::make_tuple(
            std::vector<HamArchiver::FileMetadata>
            {
                {"file_1.txt", 0, 43}, 
                {"file_2.txt", 0, 22}
            },
            std::vector<size_t>{},
            std::vector<HamArchiver::ExtractionResult>
            {
                HamArchiver::ExtractionResult::kSuccess, 
                HamArchiver::ExtractionResult::kSuccess
            }
        ),
        std::make_tuple(
            std::vector<HamArchiver::FileMetadata>
            {
                {"file_1.txt", 0, 10}, 
                {"file_2.txt", 0, 37},
                {"file_3.txt", 0, 1000}
            },
            std::vector<size_t>{0, 23, 45, 103, 158, 200, 243, 300, 359, 400, 452, 550, 551},
            std::vector<HamArchiver::ExtractionResult>
            {
                HamArchiver::ExtractionResult::kSuccess, 
                HamArchiver::ExtractionResult::kSuccess,
                HamArchiver::ExtractionResult::kFileCorrupted
            }
        )
        // This test is quite long...
        // ,
        // std::make_tuple(
        //     std::vector<HamArchiver::FileMetadata>
        //     {
        //         {"Лев_Толстой._Война_и_мир._Том_I.txt", 0, 200000},
        //     },
        //     std::vector<size_t>{15832, 567303, 705235},
        //     std::vector<HamArchiver::ExtractionResult>
        //     {
        //         HamArchiver::ExtractionResult::kSuccess,
        //     }
        // )
    )
);
