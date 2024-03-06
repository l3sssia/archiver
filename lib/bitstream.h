#pragma once

#include <bitset>
#include <cstdint>
#include <iostream>
#include <vector>
#include <fstream>

namespace archive_stream {

    /*
     *  Archive:
     *  Block size - template parameter
     *  Block inf. bits - template parameter
     *
     *  Each file is stored as:
     *  - File name     32 bytes
     *  - File size     8 bytes
     *  - File bytes
     *
     *  - Information and total bits count must be convertable to bytes (divisible by 8)
     *  - Coder must be a template class that satisfies following conditions:
     *  1) total_block_bits parameter
     *  2) information_bits parameter
     *  3) static methods decode/encode, which decodes/encodes bitset, returns bitset
     *  e.g.    Coder<32, 15> coder;
     *          std::bitset<32> encoded = coder.encode(...)
     */
    const size_t kFileName = 32;
    const size_t kFileSize = 8;
    const std::string kTempName = "_temp";
    std::string createTemporaryName(std::string& filename){
        return filename + kTempName;
    }

    template<size_t total_block_bits,
            size_t information_bits,
            typename Coder>
    class ArchiveWriter {
    public:
        static const size_t kInformationBytes = information_bits / 8;
        explicit ArchiveWriter(const std::string& filename);
        void addFile(const std::string& filename);
        void addFiles(const std::vector<std::string>& filenames);
        void deleteArchive();
        void close();
    private:
        std::ofstream out_{};
        std::string archive_filename_;

        std::byte block_[kInformationBytes]{};
        //std::bitset<total_block_bits> encoded_block_;

        size_t last_written_byte = 0;

        void encodeBlock();
        bool writeByte(std::byte c);
        bool writeRawByte(std::byte c);
    };

    template<size_t total_block_bits,
            size_t information_bits,
            typename Coder>
    class ArchiveReader {
    public:
        static const size_t kInformationBytes = information_bits / 8;
        ArchiveReader() = delete;
        explicit ArchiveReader(const std::string& filename);
        std::vector<std::pair<std::string, int64_t>> getFilesList();
        void extractFiles();
        void extractFiles(const std::vector<std::string>& filenames);
        void deleteFiles(const std::vector<std::string>& filenames);
        void close();
        void deleteArchive();
    private:
        std::ifstream in_{};
        std::string archive_filename_;
        //TODO stream with decoded bytes
        //std::bitset<total_block_bits> block_;
        std::byte decoded_[kInformationBytes]{};
        size_t last_decoded_byte = 0;

        std::byte readByte();
        void decodeLastBlock();
        bool jumpToNextFile();
        bool readNextBlock();

        std::string readFileName();
        int64_t readFileSize();
        void extractCurrentFile();
        void writeCurrentFileToTemporaryArchive(const ArchiveWriter<total_block_bits, information_bits, Coder>& temp);
    };
}

