#ifndef FILECOMPARATOR_HPP
#define FILECOMPARATOR_HPP

#include "fstream"
#include "filesystem"


class FileComparator {
public:
    FileComparator(std::filesystem::path dir);
    bool Equals(std::filesystem::path filename_1, std::filesystem::path filename_2) const;

private:
    std::filesystem::path dir_;

    size_t GetFileSize(std::filesystem::path filename) const;
};

#endif  // FILECOMPARATOR_HPP
