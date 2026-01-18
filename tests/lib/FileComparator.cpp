#include <iostream>

#include "FileComparator.hpp"

FileComparator::FileComparator(std::filesystem::path dir) 
: dir_{dir} {}

size_t FileComparator::GetFileSize(std::filesystem::path filename) const {
    std::ifstream file(dir_ / filename, std::ios::binary);
    file.seekg(0, std::ios::end);

    return static_cast<size_t>(file.tellg());
}

bool FileComparator::Equals(
    std::filesystem::path filename_1, std::filesystem::path filename_2) const {

    std::ifstream reader_1(dir_ / filename_1, std::ios::binary);
    std::ifstream reader_2(dir_ / filename_2, std::ios::binary);
    if (!reader_1.is_open() || !reader_2.is_open()) {
        // std::cout << "Invalid filename\n";
        return false;
    }
    if (GetFileSize(filename_1) != GetFileSize(filename_2)) {
        // std::cout << "Different sizes\n";
        return false;
    }
    char c_1;
    char c_2;
    while (reader_1.get(c_1) && reader_2.get(c_2)) {
        if (c_1 != c_2) {
            // std::cout << "Different bytes\n";
            return false;
        }
    }
    return true;
}