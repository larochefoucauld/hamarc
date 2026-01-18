#include <iostream>

#include "hamarc/HamArchiver.hpp"
#include "argparser/ArgParser.hpp"

std::string working_dir;
std::string arcfile;
std::vector<std::string> files;
HamArchiver harchiver{};

bool exec_create = false;
bool exec_list = false;
bool exec_extract = false;
bool exec_append = false;
bool exec_delete = false;
bool exec_merge = false;

void InitArgs(ArgumentParser::ArgParser& arg_parser) {
    arg_parser.AddStringArgument('D', "directory", "Override working directory").StoreValue(working_dir);
    arg_parser.AddStringArgument('f', "file", "An archive file").StoreValue(arcfile);
    arg_parser.AddStringArgument(0, "_files", "Files to process").MultiValue(0).Positional().StoreValues(files);
    arg_parser.AddFlag('c', "create", "Create an archive").StoreValue(exec_create);
    arg_parser.AddFlag('l', "list", "List files in archive").StoreValue(exec_list);
    arg_parser.AddFlag('x', "extract", "Extract specified files (all, if no files specified)").StoreValue(exec_extract);
    arg_parser.AddFlag('a', "append", "Append files to an archive").StoreValue(exec_append);
    arg_parser.AddFlag('d', "delete", "Delete files from an archive").StoreValue(exec_delete);
    arg_parser.AddFlag('A', "concatenate", "Merge archives").StoreValue(exec_merge);
    arg_parser.AddHelp('h', "help", "Hamming-based archiver");
}

std::vector<HamArchiver::FileMetadata> BuildFileList() {
    std::vector<HamArchiver::FileMetadata> file_list(files.size());
    std::cout << "Enter block sizes for encoding\n";
    for (size_t i = 0; i < files.size(); ++i) {
        file_list[i].path = files[i];
        std::cin >> file_list[i].encoding_block_size;
    }

    return file_list;
}

void ExecuteCreate() {
    auto exit_codes = harchiver.Create(arcfile, BuildFileList());

    switch (exit_codes[0]) {
        case HamArchiver::CreationResult::kArcAlreadyExists:
            std::cout << "\"" << arcfile << "\" already exists\n";
            return;
        case HamArchiver::CreationResult::kEmptyFileList:
            std::cout << "Empty file list\n";
            return;
    }

    for (size_t i = 0; i < exit_codes.size(); ++i) {
        std::cout << "\"" << files[i] << "\" - ";
        switch (exit_codes[i]) {
            case HamArchiver::CreationResult::kSuccess:
                std::cout << "added\n";
                continue;
            case HamArchiver::CreationResult::kFileNotFound:
                std::cout << "not found\n";
                continue;
            case HamArchiver::CreationResult::kFileNotAccessible:
                std::cout << "not accessible\n";
        }
    }
}

void ExecuteList() {
    auto list = harchiver.GetFileList(arcfile);

    if (list.empty()) {
        std::cout << "\"" << arcfile << "\" not found\n";
        return;
    }
    if (list.back().size == -1) {
        std::cout << "Archive corrupted. Reparable files:\n";
    }

    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i].size == -1) {
            break;
        }
        std::cout << list[i].path << ", size: " <<
        list[i].size << " bytes, encoding block size: " <<
        list[i].encoding_block_size << '\n';
    }
}

void BuildExtractionList() {
    auto file_list = harchiver.GetFileList(arcfile);
    for (size_t i = 0; i < file_list.size(); ++i) {
        if (file_list[i].size == -1) {
            break;
        }
        files.push_back(file_list[i].path.string());
    }
}

void ExecuteExtract() {
    if (files.empty()) {
        BuildExtractionList();
    }
    auto exit_codes = harchiver.ExtractFiles(arcfile, files);

    switch (exit_codes[0]) {
        case HamArchiver::ExtractionResult::kArcNotFound:
            std::cout << "\"" << arcfile << "\" not found\n";
            return;
        case HamArchiver::ExtractionResult::kEmptyFileList:
            std::cout << "Archive corrupted. No files can be extracted\n";
            return;
    }

    if (exit_codes.back() == HamArchiver::ExtractionResult::kArcCorrupted) {
        std::cout << "Archive corrupted. File extraction states:\n";
    }
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "\"" << files[i] << "\" - ";
        switch (exit_codes[i]) {
            case HamArchiver::ExtractionResult::kSuccess:
                std::cout << "extracted\n";
                continue;
            case HamArchiver::ExtractionResult::kFileCorrupted:
                std::cout << "corrupted\n";
                continue;
            case HamArchiver::ExtractionResult::kFileNotFound:
                std::cout << "not found\n";
        } 
    }
}

void ExecuteAppend() {
    auto exit_codes = harchiver.AppendFiles(arcfile, BuildFileList());

    switch (exit_codes[0]) {
        case HamArchiver::AdditionResult::kArcNotFound:
            std::cout << "\"" << arcfile << "\" not found\n";
            return;
        case HamArchiver::AdditionResult::kEmptyFileList:
            std::cout << "Empty file list\n";
            return;
    }

    for (size_t i = 0; i < exit_codes.size(); ++i) {
        std::cout << "\"" << files[i] << "\" - ";
        switch (exit_codes[i]) {
            case HamArchiver::AdditionResult::kSuccess:
                std::cout << "added\n";
                continue;
            case HamArchiver::AdditionResult::kFileNotFound:
                std::cout << "not found\n";
                continue;
            case HamArchiver::AdditionResult::kFileNotAccessible:
                std::cout << "not accessible\n";
        }
    }
}

void ExecuteDelete() {
    auto exit_codes = harchiver.DeleteFiles(arcfile, files);

    switch (exit_codes[0]) {
        case HamArchiver::ExtractionResult::kArcNotFound:
            std::cout << "\"" << arcfile << "\" not found\n";
            return;
        case HamArchiver::ExtractionResult::kEmptyFileList:
            std::cout << "Empty file list\n";
            return;
    }

    if (exit_codes.back() == HamArchiver::ExtractionResult::kArcCorrupted) {
        std::cout << "Archive corrupted. File extraction states:\n";
    }
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "\"" << files[i] << "\" - ";
        switch (exit_codes[i]) {
            case HamArchiver::ExtractionResult::kSuccess:
                std::cout << "deleted\n";
                continue;
            case HamArchiver::ExtractionResult::kFileCorrupted:
                std::cout << "corrupted\n";
                continue;
            case HamArchiver::ExtractionResult::kFileNotFound:
                std::cout << "not found\n";
        } 
    }
}

void ExecuteMerge() {
    auto exit_codes = harchiver.Merge(arcfile, files);

    switch (exit_codes[0]) {
        case HamArchiver::ConcatenationResult::kArcAlreadyExists:
            std::cout << "\"" << arcfile << "\" already exists\n";
            return;
        case HamArchiver::ConcatenationResult::kEmptyFileList:
            std::cout << "Empty file list\n";
            return;
    }

    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "\"" << files[i] << "\" - ";
        switch (exit_codes[i]) {
            case HamArchiver::ConcatenationResult::kSuccess:
                std::cout << "merged\n";
                continue;
            case HamArchiver::ConcatenationResult::kFileNotFound:
                std::cout << "not found\n";
        } 
    }
}

bool ExecuteCommands() {
    if (!working_dir.empty()) {
        harchiver.SetDir(working_dir);
    }

    if (arcfile.empty()) {
        std::cerr << "Error: arcfile name not set\n";
        return false;
    }

    if (exec_create) {
        ExecuteCreate();
        return true;
    }
    if (exec_list) {
        ExecuteList();
        return true;
    }
    if (exec_extract) {
        ExecuteExtract();
        return true;
    }
    if (exec_append) {
        ExecuteAppend();
        return true;
    }
    if (exec_delete) {
        ExecuteDelete();
        return true;
    }
    if (exec_merge) {
        ExecuteMerge();
        return true;
    }

    std::cerr << "Error: No known command specified\n";
    return false;
}

int main(int argc, char** argv) {
    ArgumentParser::ArgParser arg_parser("hamarc");
    InitArgs(arg_parser);
    arg_parser.Parse(argc, argv);
    if (arg_parser.Help()) {
        std::cout << arg_parser.HelpDescription();
        return 0;
    }
    if (!ExecuteCommands()) return 1;

    return 0;
}
