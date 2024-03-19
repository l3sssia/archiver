#pragma once

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <vector>
#include <fstream>
#include "hamming_code.h"
#include <filesystem>

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
 *  3) static methods:
 *  3.1) Encode(char bytes[information_bits_ / 8], char* encoded)
 *  3.2) Decode(char encoded[total_bits / 8], char* decoded)
 *  e.g.    Coder<32, 15> coder;
 *
 *
 *
 *  After finishing using ArchiveReader/ArchiveWriter,
 */

const size_t kFileNameBytes = 32;
const size_t kFileSizeBytes = 8;

const std::string kTempName = "__temp_";

template<size_t total_block_bits,
    size_t information_bits,
    typename Coder>
class ArchiveWriter {
 public:
    static const size_t kInformationBytes = information_bits / 8;
    static const size_t kTotalBlockBytes = total_block_bits / 8;
    explicit ArchiveWriter(const std::string &filename, bool trunc);
    ArchiveWriter() = delete;
    ~ArchiveWriter();
    bool AddFile(const std::string &filename);
    bool AddFiles(const std::vector<std::string> &filenames);
    void DeleteArchive();
    void Close();
    void WriteFileName(const std::string& filename);
    void WriteFileSize(uint64_t file_size);
    bool AddArchive(const std::string& other_filename);
 private:
    std::ofstream out_{};
    std::string archive_filename_;

    char block_[kInformationBytes]{};
    //std::bitset<total_block_bits> encoded_block_;

    size_t last_written_byte_ = 0;
    bool EncodeBlock();
    bool WriteByte(char c);
};

template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveWriter<total_block_bits, information_bits, Coder>::AddArchive(const std::string &other_filename) {
    std::ifstream in = std::ifstream(other_filename, std::ios::binary | std::ios::in);
    if (!in.is_open()) {
        std::cerr << "Could not open archive with given name:" << other_filename << '\n';
        return false;
    }
    char byte;
    while(in.get(byte)){
        out_.put(byte);
    }
    in.close();
    return true;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::WriteFileSize(uint64_t file_size) {
    uint64_t file_size_temp_bytes = file_size;
    for (uint8_t i = 0; i < kFileSizeBytes; ++i) {
        WriteByte(file_size_temp_bytes & (0xFF));
        file_size_temp_bytes >>= 8;
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::WriteFileName(const std::string &filename) {
    for (size_t i = 0; i < kFileNameBytes; ++i) {
        if (i < filename.size()) {
            WriteByte(filename[i]);
        } else {
            WriteByte(0);
        }
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveWriter<total_block_bits, information_bits, Coder>::EncodeBlock() {
    //if (last_written_byte_ != 0) {
        char encoded[kTotalBlockBytes]{};
        Coder::Encode(block_, encoded);
        for (auto c : encoded) { //TODO
            out_ << c;
        }
    last_written_byte_ = 0;
    //}
    std::fill(block_, block_ + kInformationBytes, NULL);
    return true;
}


template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveWriter<total_block_bits, information_bits, Coder>::WriteByte(char c) {
    block_[last_written_byte_++] = c;
    last_written_byte_ %= kInformationBytes;
    if (last_written_byte_ == 0) {
        EncodeBlock();
    }
    return true;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::DeleteArchive() {
    this->Close();
    std::filesystem::remove(archive_filename_);
}

template<size_t total_block_bits,
    size_t information_bits,
    typename Coder>
class ArchiveReader {
 public:
    static const size_t kInformationBytes = information_bits / 8;
    static const size_t kTotalBlockBytes = total_block_bits / 8;
    ArchiveReader() = delete;
    ~ArchiveReader();
    explicit ArchiveReader(const std::string &filename);
    std::vector<std::pair<std::string, uint64_t>> GetFilesList();
    void ExtractFiles();
    void ExtractFiles(const std::vector<std::string> &filenames);
    void DeleteFiles(const std::vector<std::string> &filenames);
    void Close();
    void DeleteArchive();
 private:
    std::ifstream in_{};
    std::string archive_filename_;
    //std::bitset<total_block_bits> block_;
    char decoded_[kInformationBytes]{};
    size_t last_decoded_byte = kInformationBytes;

    char ReadByte();
    bool DecodeBlockFromStart();
    void ToBegin();
    void ToNextFileFromFileStart(size_t full_file_size);
    void ToBlockStart(size_t position);
    std::string ReadFileName();
    uint64_t ReadFileSize();
    bool ExtractCurrentFile();
    void WriteCurrentFileToTemporaryArchive(std::ofstream &temp,
                                            uint64_t file_size);
    void WriteRawBytes(std::ofstream &out, uint64_t bytes_count);
    void ReplaceWithTemporary(const std::string& temp_name, std::ofstream &temp);
};
template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::ReplaceWithTemporary(const std::string& temp_name,
                                                                                    std::ofstream &temp) {
    Close();
    std::error_code err_code;
    if(!std::filesystem::remove(archive_filename_)){
        std::cerr << "Could not rewrite new archive after extract. Archive is written to temporary file\n";
        return;
    }
    temp.close();
    std::filesystem::rename(temp_name, archive_filename_, err_code);
    if (err_code) {
        std::cerr << "Could not rewrite new archive after extract. Archive is written to temporary file\n";
        return;
    }
    this->in_ = std::ifstream(archive_filename_, std::ios::binary | std::ios::in);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::DeleteArchive() {
    Close();
    std::filesystem::remove(archive_filename_);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveReader<total_block_bits, information_bits, Coder>::ExtractCurrentFile() {
    std::ofstream out;
    try {
        std::string filename = ReadFileName();
        uint64_t file_size = ReadFileSize();
        out = std::ofstream(filename, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }
        for (size_t i = 0; i < file_size; ++i) {
            try {
                out << ReadByte();
            } catch (std::runtime_error &e) {
                std::cerr << e.what() << '\n';
                return false;
            }
        }
        while(last_decoded_byte!=kInformationBytes){
            ReadByte();
        }
        out.close();
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        out.close();
        Close();
    }
    return true;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
uint64_t ArchiveReader<total_block_bits, information_bits, Coder>::ReadFileSize() {
    uint64_t file_size = 0;
    for (int i = 0; i < kFileSizeBytes; ++i) {
        try {
            unsigned char current_byte = ReadByte();
            file_size ^= (current_byte << (8 * i));
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
    return file_size;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
std::string ArchiveReader<total_block_bits, information_bits, Coder>::ReadFileName() {
    std::string filename;
    filename.resize(kFileNameBytes);
    for (auto& i : filename) {
        try {
            i = ReadByte();
        } catch (std::runtime_error& e) {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
    while (filename.back() == 0) {
        filename.pop_back();
    }
    return filename;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::ToBlockStart(size_t position) {
    in_.seekg(position);
    std::fill(decoded_, decoded_ + kInformationBytes, 0);
    last_decoded_byte = kInformationBytes;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::ToNextFileFromFileStart(size_t full_file_size) {
    size_t current_pos = in_.tellg();
    size_t blocks = (full_file_size + kInformationBytes - 1) / kInformationBytes;
    ToBlockStart(current_pos + blocks * kTotalBlockBytes);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveReader<total_block_bits, information_bits, Coder>::DecodeBlockFromStart() {
    char encoded[kTotalBlockBytes]{};
    for (char& i : encoded) {
        if (in_.peek()==EOF) {
            return false;
            break;
        }
        in_.get(i);
    }
    if (!Coder::Decode(encoded, decoded_)){
        return false;
    }
    return true;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::ToBegin() {
    ToBlockStart(0);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
char ArchiveReader<total_block_bits, information_bits, Coder>::ReadByte() {
    if (last_decoded_byte == kInformationBytes) {
        if (!DecodeBlockFromStart()){
            Close();
            throw std::runtime_error("Could not decode file from archive:" + archive_filename_);
        }
        last_decoded_byte = 0;
    }
    return decoded_[last_decoded_byte++];
}


template<size_t total_block_bits, size_t information_bits, typename Coder>
ArchiveReader<total_block_bits, information_bits, Coder>::ArchiveReader(const std::string &filename) {
    if (total_block_bits % 8 != 0 || information_bits % 8 != 0) {
        throw std::logic_error("Could not open/create archive with given template parameters: " +
            std::to_string(total_block_bits) +
            std::to_string(information_bits) + '\n');
    }
    archive_filename_ = filename;
    this->in_ = std::ifstream(filename, std::ios::binary | std::ios::in);
    if (!in_.is_open()) {
        std::cerr << "Could not open archive with name: " + filename + '\n';
        throw std::invalid_argument("Could not open archive with name: " + filename + '\n');
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
std::vector<std::pair<std::string, uint64_t>> ArchiveReader<total_block_bits, information_bits, Coder>::GetFilesList() {
    std::vector<std::pair<std::string, uint64_t>> list;
    try {
        this->ToBegin();
        while (in_.peek()!=EOF) {
            size_t file_start = in_.tellg();
            std::string filename = ReadFileName();
            uint64_t file_size = ReadFileSize();
            list.emplace_back(filename, file_size);
            ToBlockStart(file_start);
            ToNextFileFromFileStart(file_size + kFileNameBytes + kFileSizeBytes);
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        Close();
        return {};
    }
    Close();
    return list;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::ExtractFiles() {
    this->ToBegin();
    while (in_.peek()!=EOF) {
        ExtractCurrentFile();
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::ExtractFiles(const std::vector<std::string> &filenames) {
    std::string temporary_name = archive_filename_ + kTempName;
    //ArchiveWriter<total_block_bits, information_bits, Coder> temporary_archive(temporary_name, true, true);
    std::ofstream temp(temporary_name, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!temp.is_open()) {
        //throw std::runtime_error("Could not open temporary archive");
        return;
    }
    try {
        this->ToBegin();
        while (in_.peek()!=EOF) {
            size_t file_start = in_.tellg();
            std::string filename = ReadFileName();
            uint64_t file_size = ReadFileSize();
            ToBlockStart(file_start);
            if (std::find(filenames.begin(), filenames.end(), filename) != filenames.end()) {
                ExtractCurrentFile();
            } else {
                WriteCurrentFileToTemporaryArchive(temp, file_size);
            }
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        Close();
        temp.close();
        return;
    }
    ReplaceWithTemporary(temporary_name, temp);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::DeleteFiles(const std::vector<std::string> &filenames) {
    std::string temporary_name = archive_filename_ + kTempName;
    std::ofstream temp(temporary_name, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!temp.is_open()) {
        //throw std::runtime_error("Could not open temporary archive");
        return;
    }
    try {
        this->ToBegin();
        while (in_.peek()!=EOF) {
            size_t file_start = in_.tellg();
            std::string filename = ReadFileName();
            uint64_t file_size = ReadFileSize();
            ToBlockStart(file_start);
            if (std::find(filenames.begin(), filenames.end(), filename) != filenames.end()) {
                ToNextFileFromFileStart(file_size + kFileNameBytes + kFileSizeBytes);
            } else {
                WriteCurrentFileToTemporaryArchive(temp, file_size);
            }
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << '\n';
        Close();
        temp.close();
        return;
    }
    ReplaceWithTemporary(temporary_name, temp);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::Close() {
    in_.close();
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
ArchiveWriter<total_block_bits, information_bits, Coder>::ArchiveWriter(const std::string &filename,
                                                                        bool trunc) {
    if (total_block_bits % 8 != 0 || information_bits % 8 != 0) {
        throw std::logic_error("Could not open/create archive with given template parameters: " +
            std::to_string(total_block_bits) +
            std::to_string(information_bits) + '\n');
    }
    archive_filename_ = filename;
    auto mode = ((trunc) ? std::ios::trunc : std::ios::app);
    this->out_ = std::ofstream(filename, std::ios::binary | std::ios::out | mode);
    if (!out_.is_open()) {
        std::cerr << "Could not open archive with name: " + filename + '\n';
        throw std::invalid_argument("Could not open archive with name: " + filename + '\n');
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveWriter<total_block_bits, information_bits, Coder>::AddFile(const std::string &filename) {
    uint64_t file_size = std::ifstream(filename, std::ios::ate).tellg();
    std::ifstream in = std::ifstream(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Could not open file for putting into archive: " + filename + '\n';
        return false;
    }
    // Write header
    WriteFileName(filename);
    WriteFileSize(file_size);
    char byte;
    while (in.get(byte)) {
        WriteByte(byte);
    }
    while(last_written_byte_!=0){
        WriteByte(0);
    }
    return true;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
bool ArchiveWriter<total_block_bits, information_bits, Coder>::AddFiles(const std::vector<std::string> &filenames) {
    bool flag = false;
    for (auto filename : filenames) {
        if(!AddFile(filename)){
            flag = true;
        }
    }
    return !flag;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::Close() {
    if (last_written_byte_ != 0) {
        EncodeBlock();
    }
    out_.close();
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
ArchiveReader<total_block_bits, information_bits, Coder>::~ArchiveReader() {
    this->Close();
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
ArchiveWriter<total_block_bits, information_bits, Coder>::~ArchiveWriter() {
    this->Close();
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::WriteRawBytes(std::ofstream& out,
                                                                             uint64_t bytes_count) {
//    if (bytes_count % kTotalBlockBytes !=0) {
//        throw std::invalid_argument("Count of bytes written to file must be divisible by block size");
//    }
//TODO
    for(uint64_t i = 0; i < bytes_count; ++i) {
        out.put(in_.get());
    }
    last_decoded_byte = kInformationBytes;
}


template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::WriteCurrentFileToTemporaryArchive(std::ofstream &temp,
                                                                                                  uint64_t file_size) {
    size_t blocks_count = (file_size + kFileNameBytes + kFileSizeBytes + kInformationBytes - 1) / kInformationBytes;
    size_t bytes_count = blocks_count * kTotalBlockBytes;
    WriteRawBytes(temp, bytes_count);
}


}

