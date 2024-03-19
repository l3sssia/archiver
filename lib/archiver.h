#include <utility>
#pragma once
#include "bitstream.h"

namespace archiver {
using namespace archive_stream;
using namespace hamming_code;

class Archiver {
 public:
    Archiver(std::string archive_name, const std::vector<std::string> &files, int block_size = 32);
    Archiver& Create();
    std::vector<std::pair<std::string, uint64_t>> List();
    Archiver& AddFiles();
    Archiver& ExtractFiles();
    Archiver& DeleteFiles();
    Archiver& Concatenate();
 private:
    std::string archive_name_;
    std::vector<std::string> files_;
    int block_size_;
};
}
