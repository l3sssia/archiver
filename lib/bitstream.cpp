#include <algorithm>
#include "bitstream.h"

using namespace archive_stream;

template<size_t total_block_bits, size_t information_bits, typename Coder>
ArchiveReader<total_block_bits, information_bits, Coder>::ArchiveReader(const std::string &filename) {
    if (total_block_bits % 8 !=0 || information_bits % 8 != 0) {
        throw std::logic_error("Could not open/create archive with given template parameters: " +
                                    std::to_string(total_block_bits) +
                                    std::to_string(information_bits) + '\n');
    }
    this->in_ = std::ifstream(filename, std::ios::binary | std::ios::in);
    if (!in_.is_open()) {
        std::cerr << "Could not open archive with name: " + filename + '\n';
        throw std::invalid_argument("Could not open archive with name: " + filename + '\n');
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
std::vector<std::pair<std::string, int64_t>> ArchiveReader<total_block_bits, information_bits, Coder>::getFilesList() {
    std::vector<std::pair<std::string, int64_t>> list;
    in_.seekg(0);
    while (!in_.eof()){
        std::string filename = readFileName();
        int64_t size = readFileSize();
        list.emplace_back(filename, size);
        jumpToNextFile(); //TODO
    }
    return list;
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::extractFiles() {
    in_.seekg(0);
    while (!in_.eof()) {
        extractCurrentFile();
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::extractFiles(const std::vector<std::string>& filenames) {
    std::string temporary_name = createTemporaryName(archive_filename_);
    ArchiveWriter<total_block_bits, information_bits, Coder> temporary_archive(temporary_name);
    in_.seekg(0);
    while (!in_.eof()) {
        size_t file_start = in_.tellg();
        std::string filename = readFileName();
        in_.seekg(file_start); //TODO
        if (std::find(filenames.begin(), filenames.end(), filename) != filenames.end()) {
            extractCurrentFile();
        } else {
            writeCurrentFileToTemporaryArchive(temporary_archive);
        }
    }
    this->close();
    temporary_archive.deleteArchive();
    *this = ArchiveReader<total_block_bits, information_bits, Coder>(temporary_name);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::deleteFiles(const std::vector<std::string>& filenames) {
    std::string temporrary_name = createTemporaryName(archive_filename_);
    ArchiveWriter<total_block_bits, information_bits, Coder> temporary_archive(temporrary_name);
    in_.seekg(0);
    while (!in_.eof()) {
        size_t file_start = in_.tellg();
        std::string filename = readFileName();
        in_.seekg(file_start); //TODO
        if (std::find(filenames.begin(), filenames.end(), filename) != filenames.end()) {
            jumpToNextFile();
        } else {
            writeCurrentFileToTemporaryArchive(temporary_archive);
        }
    }
    this->close();
    temporary_archive.deleteArchive();
    *this = ArchiveReader<total_block_bits, information_bits, Coder>(temporrary_name);
}


template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveReader<total_block_bits, information_bits, Coder>::close() {
    in_.close();
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
ArchiveWriter<total_block_bits, information_bits, Coder>::ArchiveWriter(const std::string &filename) { // TODO
    if (total_block_bits % 8 !=0 || information_bits % 8 != 0) {
        throw std::logic_error("Could not open/create archive with given template parameters: " +
                               std::to_string(total_block_bits) +
                               std::to_string(information_bits) + '\n');
    }
    this->out_ = std::ofstream(filename, std::ios::binary | std::ios::out | std::ios::app);
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::addFile(const std::string &filename) {
    uint64_t file_size = std::ifstream(filename, std::ios::ate).tellg();
    std::ifstream in = std::ifstream(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Could not open file for putting into archive: " + filename + '\n';
        throw std::invalid_argument("Could not open archive with name: " + filename + '\n');
    }
    // Write header
    for (size_t i = 0; i < kFileName; ++i) {
        if (i < filename.size()) {
            writeByte(filename[i]);
        } else {
            writeByte(0);
        }
    }
    std::string file_size_bytes = std::to_string(file_size);
    for (uint8_t i = 0; i < kFileSize; ++i) {
        writeByte(file_size_bytes[i]);
    }
    char byte;
    while (in.get(byte)) {
        writeByte(byte);
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::addFiles(const std::vector<std::string> &filenames) {
    for (auto filename : filenames) {
        addFile(filename);
    }
}

template<size_t total_block_bits, size_t information_bits, typename Coder>
void ArchiveWriter<total_block_bits, information_bits, Coder>::close() {
    if (last_written_byte != 0) {
        encodeBlock();
    }
    out_.close();
}
