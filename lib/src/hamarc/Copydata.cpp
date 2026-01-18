#include "Copydata.hpp"

const size_t Copydata::kMaxBufferSize = 8192;


Copydata::CopyingResult Copydata::CopyData(std::istream& reader, std::ostream& writer, 
                             size_t data_size) {
    if (!reader.good()) {
        return CopyingResult::kReaderCorrupted;
    }
    if (data_size == 0) {
        return CopyingResult::kSuccess;
    }
    size_t buffer_size = std::min(data_size, kMaxBufferSize);
    char* buffer = new char[buffer_size];

    CopyingResult cur_state = CopyingResult::kSuccess;
    size_t start_pos = reader.tellg();
    for (
        size_t pos = start_pos; 
        cur_state != CopyingResult::kReaderEOFReached && 
        pos - start_pos < data_size; pos = reader.tellg()
        ) {
        
        size_t to_read = std::min(buffer_size, data_size - (pos - start_pos));
        reader.read(buffer, to_read);
        if (reader.gcount() != to_read) {
            cur_state = CopyingResult::kReaderEOFReached;
        }
        writer.write(buffer, reader.gcount());
    }
    delete [] buffer;

    return cur_state;
}
