#include <utility>
#pragma once
#include "bitstream.h"

namespace archiver {
using namespace archive_stream;
using namespace hamming_code;

class Archiver {
 public:
    typedef ArchiveReader<512, 496, HammingCoder<512, 496>> HammingReader512;
    typedef ArchiveWriter<512, 496, HammingCoder<512, 496>> HammingWriter512;
    typedef ArchiveReader<256, 240, HammingCoder<256, 240>> HammingReader256;
    typedef ArchiveWriter<256, 240, HammingCoder<256, 240>> HammingWriter256;
    typedef ArchiveReader<128, 120, HammingCoder<128, 120>> HammingReader128;
    typedef ArchiveWriter<128, 120, HammingCoder<128, 120>> HammingWriter128;
    typedef ArchiveReader<64, 56, HammingCoder<64, 56>> HammingReader64;
    typedef ArchiveWriter<64, 56, HammingCoder<64, 56>> HammingWriter64;
    typedef ArchiveReader<32, 24, HammingCoder<32, 24>> HammingReader32;
    typedef ArchiveWriter<32, 24, HammingCoder<32, 24>> HammingWriter32;
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
