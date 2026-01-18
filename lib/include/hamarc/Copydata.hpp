#ifndef COPYDATA_HPP
#define COPYDATA_HPP

#include <fstream>

class Copydata {
public:
    enum class CopyingResult{
        kSuccess,
        kReaderEOFReached,
        kReaderCorrupted
    };

    static CopyingResult CopyData(std::istream& read_from, std::ostream& write_to, size_t data_size);
    
private:
    static const size_t kMaxBufferSize;
};

#endif  // COPYDATA_HPP
