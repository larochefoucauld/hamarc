#ifndef FILEOPERATOR_HPP
#define FILEOPERATOR_HPP

#include <filesystem>
#include <fstream>

class FileOperator {
public:
    FileOperator();
    FileOperator(std::filesystem::path init_dir);
    void SetDir(std::filesystem::path new_dir);

    bool FileExists(std::filesystem::path filename);
    size_t GetFileSize(std::filesystem::path filename);

    bool CreateFile(std::filesystem::path filename);
    bool CreateDir(std::filesystem::path name);
    bool DeleteFile(std::filesystem::path filename);
    size_t DeleteDir(std::filesystem::path name);
    void RenameFile(std::filesystem::path old_name, 
        std::filesystem::path new_name);

    bool OpenForReading(std::filesystem::path file, std::ifstream& stream, 
        std::ifstream::openmode openmode);
    bool OpenForWriting(std::filesystem::path file, std::ofstream& stream, 
        std::ofstream::openmode openmode);
    bool Open(std::filesystem::path file, std::fstream& stream, 
        std::fstream::openmode openmode);
    

private:
    std::filesystem::path dir_;
};

#endif  // FILEOPERATOR_HPP
