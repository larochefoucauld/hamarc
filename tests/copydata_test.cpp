#include <gtest/gtest.h>

#include "hamarc/Copydata.hpp"
#include "FileComparator.hpp"

static const std::filesystem::path TestingDir{"./tests/data/copydata_test"};

static FileComparator fc(TestingDir);


class CopyingTestSuite 
    : public testing::TestWithParam<
        std::tuple<
            std::filesystem::path, // file
            size_t,                // from
            size_t,                // size
            std::filesystem::path  // copied
        >
    >
{};

TEST_P(CopyingTestSuite, CopyingTest) {
    Copydata cpy;
    std::ifstream in(TestingDir / std::get<0>(GetParam()), std::ios::binary);
    in.seekg(std::get<1>(GetParam()));
    std::ofstream out(TestingDir / "tmp.txt", std::ios::trunc);

    cpy.CopyData(in, out, std::get<2>(GetParam()));
    out.close();

    ASSERT_TRUE(fc.Equals(std::get<3>(GetParam()), "tmp.txt"));
    
    std::filesystem::remove(TestingDir / "tmp.txt");
}

INSTANTIATE_TEST_SUITE_P(
    Group,
    CopyingTestSuite,
    testing::Values(
        std::make_tuple(
            "in_1.txt",
            0,
            0,
            "out_1.txt"
        ),
        std::make_tuple(
            "in_2.txt",
            0,
            -1,
            "out_2.txt"
        ),
        std::make_tuple(
            "in_3.txt",
            5,
            10,
            "out_3.txt"
        ),
        std::make_tuple(
            "in_4.txt",
            3,
            6,
            "out_4.txt"
        )
    )
);
