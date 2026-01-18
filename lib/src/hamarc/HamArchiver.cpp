#include <unordered_map>

#include "HamArchiver.hpp"
#include "Copydata.hpp"

const size_t HamArchiver::kNumericMetadataSize = 4 + 8 + 8;

HamArchiver::HamArchiver() : file_operator() {}

HamArchiver::HamArchiver(std::filesystem::path working_dir) 
: file_operator(working_dir)
 {}

void HamArchiver::SetDir(std::filesystem::path new_dir) {
    file_operator.SetDir(new_dir);
}

size_t HamArchiver::GetMsgCodeSize(size_t raw_msg_size) {
    if (raw_msg_size == 0) {
        return 0;
    }
    return Encoder::GetCodeBitSize(raw_msg_size * 8) / 8 + 1;
}

size_t HamArchiver::GetEncodedMsgSize(size_t raw_msg_size, size_t encoding_block_size) {
    return raw_msg_size 
    + (raw_msg_size / encoding_block_size) 
    * GetMsgCodeSize(encoding_block_size) 
    + GetMsgCodeSize(raw_msg_size % encoding_block_size);
};

size_t HamArchiver::GetEncodedMsgSize(size_t raw_msg_size) {
    return raw_msg_size + GetMsgCodeSize(raw_msg_size);
}

std::vector<HamArchiver::CreationResult> HamArchiver::Create(std::string_view arcname, 
    const std::vector<FileMetadata>& files) {

    std::vector<CreationResult> creation_result;
    if (file_operator.FileExists(arcname)) {
        creation_result.push_back(CreationResult::kArcAlreadyExists);
        return creation_result;
    }
    if (files.empty()) {
        creation_result.push_back(CreationResult::kEmptyFileList);
        return creation_result;
    }
    file_operator.CreateFile(arcname);
    auto addition_result = AppendFiles(arcname, files);
    
    creation_result.resize(addition_result.size());
    for (size_t i = 0; i < addition_result.size(); ++i) {
        switch (addition_result[i]) {
            case AdditionResult::kSuccess:
                creation_result[i] = CreationResult::kSuccess;
                break;
            case AdditionResult::kFileNotFound:
                creation_result[i] = CreationResult::kFileNotFound;
                break;
            case AdditionResult::kFileNotAccessible:
                creation_result[i] = CreationResult::kFileNotAccessible;
        }
    }

    return creation_result;
}

std::vector<HamArchiver::FileMetadata> HamArchiver::GetFileList(std::filesystem::path arcfile) {
    if (!file_operator.FileExists(arcfile)) {
        return std::vector<FileMetadata>{};
    }
    
    size_t arc_size = file_operator.GetFileSize(arcfile);
    std::fstream stream;
    file_operator.Open(arcfile, stream, std::fstream::in | std::fstream::out | std::fstream::binary);
    std::vector<FileMetadata> files;
    while (stream.tellg() != arc_size) {
        files.push_back(GetMetadata(stream));
        if (files.back().size == -1) {
            break;
        }
        stream.seekg(
            GetEncodedMsgSize(
                files.back().size, 
                files.back().encoding_block_size
            ), 
            std::fstream::cur
        );
    }

    return files;
}

std::vector<HamArchiver::ExtractionResult> HamArchiver::ExtractFiles(std::filesystem::path arcfile) {

    if (!file_operator.FileExists(arcfile)) {
        return {ExtractionResult::kArcNotFound};
    }
    std::vector<FileMetadata> file_list = GetFileList(arcfile);
    std::vector<std::string> filenames(file_list.size());
    for (size_t i = 0; i < file_list.size(); ++i) {
        if (file_list[i].size == -1) {
            break;
        } 
        filenames[i] = file_list[i].path.string();
    }
    return RebuildArc(arcfile, filenames, true);    
}

std::vector<HamArchiver::ExtractionResult> HamArchiver::ExtractFiles(std::filesystem::path arcfile, 
        const std::vector<std::string>& filenames) {

    return RebuildArc(arcfile, filenames, true);    
}

std::vector<HamArchiver::ExtractionResult> HamArchiver::DeleteFiles(std::filesystem::path arcfile, 
        const std::vector<std::string>& filenames) {

    return RebuildArc(arcfile, filenames, false);
}


std::vector<HamArchiver::AdditionResult> HamArchiver::AppendFiles(std::filesystem::path arcfile, 
        const std::vector<FileMetadata>& files) {

    std::vector<AdditionResult> addition_result;
    if (!file_operator.FileExists(arcfile)) {
        addition_result.push_back(AdditionResult::kArcNotFound);
        return addition_result;
    }
    if (files.empty()) {
        addition_result.push_back(AdditionResult::kEmptyFileList);
        return addition_result;
    }
    
    std::ofstream writer;
    file_operator.OpenForWriting(arcfile, writer, std::ofstream::app | std::ofstream::binary);
    addition_result.resize(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        addition_result[i] = WriteEncodedFile(files[i], writer);
    }

    return addition_result;
}

std::vector<HamArchiver::ConcatenationResult> HamArchiver::Merge(std::string_view arcname,
        const std::vector<std::string>& arcfiles) {
    
    if (arcfiles.empty()) {
        return {ConcatenationResult::kEmptyFileList};
    }
    if (file_operator.FileExists(arcname)) {
        return {ConcatenationResult::kArcAlreadyExists};
    }
    file_operator.CreateFile(arcname);
    std::ofstream writer;
    file_operator.OpenForWriting(arcname, writer, std::ofstream::app | std::ofstream::binary);
    std::vector<ConcatenationResult> res(arcfiles.size(), ConcatenationResult::kFileNotFound);
    for (size_t i = 0; i < arcfiles.size(); ++i) {
        if (!file_operator.FileExists(arcfiles[i])) {
            continue;
        }
        std::ifstream reader;
        file_operator.OpenForReading(arcfiles[i], reader, std::ifstream::binary);
        Copydata::CopyData(reader, writer, file_operator.GetFileSize(arcfiles[i]));
        res[i] = ConcatenationResult::kSuccess;
    }

    return res;
}


std::vector<HamArchiver::ExtractionResult> HamArchiver::RebuildArc(std::filesystem::path arcfile,
    const std::vector<std::string>& skip_list, bool extract) {

    if (!file_operator.FileExists(arcfile)) {
        return {ExtractionResult::kArcNotFound};
    }
    if (skip_list.empty()) {
        return {ExtractionResult::kEmptyFileList};
    }

    std::unordered_map<std::string_view, ExtractionResult> file_states;
    for (size_t i = 0; i < skip_list.size(); ++i) {
        file_states[skip_list[i]] = ExtractionResult::kFileNotFound;
    }

    std::fstream stream;
    file_operator.Open(arcfile, stream, std::fstream::in | std::fstream::out | std::fstream::binary);
    std::filesystem::path tmp{"__arctmp__.haf"};
    std::ofstream writer;
    file_operator.OpenForWriting(tmp, writer, std::ofstream::trunc | std::ofstream::binary);

    size_t arcfile_size = file_operator.GetFileSize(arcfile);
    bool arc_corrupted = false;
    while (stream.tellg() < arcfile_size) {
        std::streampos metadata_beg = stream.tellg();
        FileMetadata cur_metadata = GetMetadata(stream);
        if (cur_metadata.size == -1) {
            arc_corrupted = true;
            break;
        }
        
        std::string cur_filename = cur_metadata.path.filename().string();
        if (!cur_filename.empty() && file_states.find(cur_filename) != file_states.end()) {
            if (extract) {
                file_states[cur_filename] = ExtractFile(cur_metadata, stream, false);
                continue;
            } 
            file_states[cur_filename] = ExtractionResult::kSuccess;
            stream.seekg(
                GetEncodedMsgSize(
                    cur_metadata.size, 
                    cur_metadata.encoding_block_size
                ), std::fstream::cur);
            continue;
        }

        // Возврат на первый байт метаданных
        stream.seekg(metadata_beg, std::fstream::beg);

        Copydata::CopyData(stream, writer, 
            GetEncodedMsgSize(kNumericMetadataSize) 
            + GetEncodedMsgSize(cur_filename.size())
            + GetEncodedMsgSize(
                cur_metadata.size, cur_metadata.encoding_block_size
            )
        );
    }
    stream.close();
    writer.close();

    file_operator.DeleteFile(arcfile);
    file_operator.RenameFile(tmp, arcfile);

    if (file_operator.GetFileSize(arcfile) == 0) {
        file_operator.DeleteFile(arcfile);
    }

    std::vector<ExtractionResult> res(skip_list.size());
    for (size_t i = 0; i < skip_list.size(); ++i) {
        res[i] = file_states[skip_list[i]];
    }
    if (arc_corrupted) {
        res.push_back(ExtractionResult::kArcCorrupted);
    }

    return res;
}

HamArchiver::ExtractionResult HamArchiver::ExtractFile(
    FileMetadata metadata, std::fstream& stream, bool forced) {
    
    std::streampos start_pos = stream.tellg();
    size_t full_blocks = metadata.size / metadata.encoding_block_size;
    size_t encoded_block_size = GetEncodedMsgSize(metadata.encoding_block_size);
    ExtractionResult exit_code = ExtractionResult::kSuccess;
    for (size_t i = 0; i <= full_blocks; ++i) {
        size_t cur_block_size = metadata.encoding_block_size;
        if (i == full_blocks) {
            size_t last_block_size = metadata.size % metadata.encoding_block_size;
            if (last_block_size == 0) {
                break;
            }
            cur_block_size = last_block_size;
            encoded_block_size = GetEncodedMsgSize(cur_block_size);
        }

        Decoder::ValidationResult cur_block_state = Decoder::Validate(
            stream, cur_block_size);
        if (cur_block_state == Decoder::ValidationResult::kDoubleError) {
            exit_code = ExtractionResult::kFileCorrupted;
            if (!forced) {
                stream.seekg(start_pos + static_cast<std::streampos>(
                    GetEncodedMsgSize
                (
                    metadata.size, metadata.encoding_block_size
                )
                ), std::fstream::beg);
                return exit_code;
            }
        }
        stream.seekg(encoded_block_size, std::fstream::cur);
    }

    file_operator.CreateFile(metadata.path.filename());
    std::ofstream writer;
    file_operator.OpenForWriting(metadata.path.filename(), 
        writer, std::fstream::binary);
    stream.seekg(start_pos, std::fstream::beg);

    for (size_t i = 0; i <= full_blocks; ++i) {
        size_t cur_block_size = metadata.encoding_block_size;
        if (i == full_blocks) {
            size_t last_block_size = metadata.size % metadata.encoding_block_size;
            if (last_block_size == 0) {
                break;
            }
            cur_block_size = last_block_size;
        }

        Copydata::CopyData(stream, writer, cur_block_size);
        stream.seekg(GetMsgCodeSize(cur_block_size), std::fstream::cur);
    }

    return exit_code;
}

HamArchiver::FileMetadata HamArchiver::GetMetadata(std::fstream& stream) {
    if (Decoder::Validate(stream, kNumericMetadataSize) == Decoder::ValidationResult::kDoubleError) {
        return FileMetadata{
            std::filesystem::path{}, 
            static_cast<size_t>(-1), 
            static_cast<size_t>(-1)
        };
    }
    uint8_t* numeric_metadata_buf = new uint8_t[kNumericMetadataSize];
    stream.read(reinterpret_cast<char*>(numeric_metadata_buf), kNumericMetadataSize);
    HamArchiver::FileMetadata file{std::filesystem::path{}, 0, 0};
    size_t filename_size = 0;
    for (size_t i = 0; i < 4; ++i) {
        filename_size |= (static_cast<size_t>(numeric_metadata_buf[i]) << i * 8);
    }
    for (size_t i = 0; i < 8; ++i) {
        file.size |= (static_cast<size_t>(numeric_metadata_buf[i + 4]) << i * 8);
    }
    for (size_t i = 0; i < 8; ++i) {
        file.encoding_block_size |= (static_cast<size_t>(numeric_metadata_buf[i + 12]) << i * 8);
    }
    delete [] numeric_metadata_buf;
    stream.seekg(GetMsgCodeSize(kNumericMetadataSize), std::fstream::cur);
    if (Decoder::Validate(stream, filename_size) == Decoder::ValidationResult::kDoubleError) {
        stream.seekg(GetEncodedMsgSize(filename_size), std::fstream::cur);
        return file;
    }
    char* filename_buf = new char[filename_size];
    stream.read(filename_buf, filename_size);
    stream.seekg(GetMsgCodeSize(filename_size), std::fstream::cur);
    file.path = std::string{filename_buf, filename_size};
    delete [] filename_buf;
    
    return file;
}

void HamArchiver::WriteEncodedMetadata(FileMetadata file, std::ostream& writer) {
    std::string filename = file.path.filename().string();
    uint8_t* numeric_metadata_buf = new uint8_t[kNumericMetadataSize];

    for (size_t i = 0; i < 4; ++i) {
        numeric_metadata_buf[i] = (filename.size() >> (8 * i)) & 0b11111111;
    }
    for (size_t i = 0; i < 8; ++i) {
        numeric_metadata_buf[i + 4] = (file.size >> (8 * i)) & 0b11111111;
    }
    for (size_t i = 0; i < 8; ++i) {
        numeric_metadata_buf[i + 12] = (file.encoding_block_size >> (8 * i)) & 0b11111111;
    }
    
    writer.write(reinterpret_cast<char*>(numeric_metadata_buf), kNumericMetadataSize);
    Encoder::EncodeAndWrite(numeric_metadata_buf, writer, kNumericMetadataSize);
    delete [] numeric_metadata_buf;
    writer.write(filename.data(), filename.size());
    Encoder::EncodeAndWrite(reinterpret_cast<uint8_t*>(filename.data()), writer, filename.size());    

}

HamArchiver::AdditionResult HamArchiver::WriteEncodedFile(FileMetadata file, std::ostream& writer) {
    AdditionResult exit_code;
    if (!file_operator.FileExists(file.path)) {
        return AdditionResult::kFileNotFound;
    }
    file.size = file_operator.GetFileSize(file.path);
    file.encoding_block_size = std::min(file.size, file.encoding_block_size);
    std::ifstream raw_file_reader;
    if (!file_operator.OpenForReading(file.path, raw_file_reader, std::ifstream::binary)) {
        return AdditionResult::kFileNotAccessible;
    }
    WriteEncodedMetadata(file, writer);

    for (size_t i = 0; i < file.size; i += file.encoding_block_size) {
        size_t cur_block_size = std::min(file.encoding_block_size, file.size - i);
        Copydata::CopyData(raw_file_reader, writer, cur_block_size);
        raw_file_reader.seekg(-static_cast<std::streamoff>(cur_block_size), std::ifstream::cur);
        Encoder::EncodeAndWrite(raw_file_reader, writer, cur_block_size);
    }
    
    return AdditionResult::kSuccess;
}
