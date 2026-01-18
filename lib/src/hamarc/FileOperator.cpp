#include "FileOperator.hpp"

FileOperator::FileOperator() : dir_(std::filesystem::current_path()) {}

FileOperator::FileOperator(std::filesystem::path init_dir) 
              : dir_(init_dir) 
              {}

void FileOperator::SetDir(std::filesystem::path new_dir) {
    dir_ = std::filesystem::path{new_dir};
    std::filesystem::create_directories(new_dir);
}

bool FileOperator::FileExists(std::filesystem::path filename) {
    std::ifstream checker(dir_ / filename, std::ios::binary);
    return checker.is_open();
}

size_t FileOperator::GetFileSize(std::filesystem::path filename) {
    std::ifstream file(dir_ / filename, std::ios::binary);
    file.seekg(0, std::ios::end);

    return static_cast<size_t>(file.tellg());
}

// Создаёт новый, либо очищает уже существующий файл
bool FileOperator::CreateFile(std::filesystem::path filename) {
    std::ofstream creator(dir_ / filename, std::ios::trunc);
    return creator.is_open();
}

bool FileOperator::CreateDir(std::filesystem::path name) {
    return std::filesystem::create_directories(dir_ / name);
}

bool FileOperator::DeleteFile(std::filesystem::path filename) {
    return std::filesystem::remove(dir_ / filename);
}

size_t FileOperator::DeleteDir(std::filesystem::path name) {
    return std::filesystem::remove_all(dir_ / name);
}

void FileOperator::RenameFile(std::filesystem::path old_name, 
        std::filesystem::path new_name) {
    
    std::filesystem::rename(dir_ / old_name, dir_ / new_name);
}

bool FileOperator::OpenForReading(std::filesystem::path file, 
    std::ifstream& stream, std::ifstream::openmode openmode) {
    stream.open(dir_ / file, openmode);
    return stream.is_open();
}

bool FileOperator::OpenForWriting(std::filesystem::path file, 
    std::ofstream& stream, std::ofstream::openmode openmode) {
    stream.open(dir_ / file, openmode);
    return stream.is_open();
}

bool FileOperator::Open(std::filesystem::path file, 
    std::fstream& stream, std::fstream::openmode openmode) {

    stream.open(dir_ / file, openmode);
    return stream.is_open();
}
