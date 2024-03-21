#include "archiver.h"
#include <iostream>

archiver::Archiver::Archiver(std::string archive_name, const std::vector<std::string> &files, int block_size)
                    : archive_name_(std::move(archive_name))
                    , files_(files)
                    , block_size_(block_size){
    if (archive_name_.size() < 5 || archive_name_.substr(archive_name_.size() - 4) != ".haf") {
        archive_name_ += ".haf";
    }
    switch (block_size){
        case 512:
        case 256:
        case 128:
        case 64:
        case 32:
            break;
        default:
            throw std::invalid_argument("Only 32, 64, 128, 256, 512 block size (in bits) are allowed.");
    }
}

archiver::Archiver &archiver::Archiver::Create() {
        std::remove(archive_name_.c_str());
        return AddFiles();
}
std::vector<std::pair<std::string, uint64_t>> archiver::Archiver::List() {
    std::vector<std::pair<std::string, uint64_t>> list;
    if (block_size_ == 512) {
        HammingReader512 archive(archive_name_);
        list = archive.GetFilesList();
        archive.Close();
    } else if (block_size_ == 256) {
        HammingReader256 archive(archive_name_);
        list = archive.GetFilesList();
        archive.Close();
    } else if (block_size_ == 128) {
        HammingReader128 archive(archive_name_);
        list = archive.GetFilesList();
        archive.Close();
    } else if (block_size_ == 64) {
        HammingReader64 archive(archive_name_);
        list = archive.GetFilesList();;
        archive.Close();
    } else if (block_size_ == 32) {
        HammingReader32 archive(archive_name_);
        list = archive.GetFilesList();
        archive.Close();
    }
    return list;
}
archiver::Archiver &archiver::Archiver::AddFiles() {
    if (block_size_ == 512) {
        HammingWriter512 archive(archive_name_, false);
        archive.AddFiles(files_);
        archive.Close();
    } else if (block_size_ == 256) {
        HammingWriter256 archive(archive_name_, false);
        archive.AddFiles(files_);
        archive.Close();
    } else if (block_size_ == 128) {
        HammingWriter128 archive(archive_name_, false);
        archive.AddFiles(files_);
        archive.Close();
    } else if (block_size_ == 64) {
        HammingWriter64 archive(archive_name_, false);
        archive.AddFiles(files_);
        archive.Close();
    } else if (block_size_ == 32) {
        HammingWriter32 archive(archive_name_, false);
        archive.AddFiles(files_);
        archive.Close();
    }
    return *this;
}
archiver::Archiver &archiver::Archiver::ExtractFiles() {
    if (block_size_ == 512) {
        HammingReader512 archive(archive_name_);
        if (files_.empty()) {
            archive.ExtractFiles();
        } else {
            archive.ExtractFiles(files_);
        }
        archive.Close();
    } else if (block_size_ == 256) {
        HammingReader256 archive(archive_name_);
        if (files_.empty()) {
            archive.ExtractFiles();
        } else {
            archive.ExtractFiles(files_);
        }
        archive.Close();
    } else if (block_size_ == 128) {
        HammingReader128 archive(archive_name_);
        if (files_.empty()) {
            archive.ExtractFiles();
        } else {
            archive.ExtractFiles(files_);
        }
        archive.Close();
    } else if (block_size_ == 64) {
        HammingReader128 archive(archive_name_);
        if (files_.empty()) {
            archive.ExtractFiles();
        } else {
            archive.ExtractFiles(files_);
        }
        archive.Close();
    } else if (block_size_ == 32) {
        HammingReader32 archive(archive_name_);
        if (files_.empty()) {
            archive.ExtractFiles();
        } else {
            archive.ExtractFiles(files_);
        }
        archive.Close();
    }
    return *this;
}

archiver::Archiver &archiver::Archiver::DeleteFiles() {
    if (block_size_ == 512) {
        HammingReader512 archive(archive_name_);
        archive.DeleteFiles(files_);
        archive.Close();
    } else if (block_size_ == 256) {
        HammingReader256 archive(archive_name_);
        archive.DeleteFiles(files_);
        archive.Close();
    } else if (block_size_ == 128) {
        HammingReader128 archive(archive_name_);
        archive.DeleteFiles(files_);
        archive.Close();
    } else if (block_size_ == 64) {
        HammingReader64 archive(archive_name_);
        archive.DeleteFiles(files_);
        archive.Close();
    } else if (block_size_ == 32) {
        HammingReader32 archive(archive_name_);
        archive.DeleteFiles(files_);
        archive.Close();
    }
    return *this;
}

archiver::Archiver &archiver::Archiver::Concatenate() {
    if (files_.size() < 2) {
        throw std::invalid_argument("Count of files to concatenate must be at least 2");
    }
    bool trunc = (archive_name_!= files_[0]) && (archive_name_ != files_[1]);
    if (!trunc) {
        throw std::invalid_argument("Please choose three different archives or use append option");
    }
    if (block_size_ == 512) {
        HammingWriter512 archive(archive_name_, trunc);
        archive.AddArchive(files_[0]);
        archive.AddArchive(files_[1]);
        archive.Close();
    } else if (block_size_ == 256) {
        HammingWriter256 archive(archive_name_, trunc);
        archive.AddArchive(files_[0]);
        archive.AddArchive(files_[1]);
        archive.Close();
    } else if (block_size_ == 128) {
        HammingWriter128 archive(archive_name_, trunc);
        archive.AddArchive(files_[0]);
        archive.AddArchive(files_[1]);
        archive.Close();
    } else if (block_size_ == 64) {
        HammingWriter64 archive(archive_name_, trunc);
        archive.AddArchive(files_[0]);
        archive.AddArchive(files_[1]);
        archive.Close();
    } else if (block_size_ == 32) {
        HammingWriter32 archive(archive_name_, trunc);
        archive.AddArchive(files_[0]);
        archive.AddArchive(files_[1]);
        archive.Close();
    }
    return *this;
}




