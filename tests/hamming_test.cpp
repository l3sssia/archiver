#include <gtest/gtest.h>
#include "lib/hamming_code.h"
#include "lib/archiver.h"
#include <string>

#define block_size 128

std::string bin(const char* str, size_t size) {
    std::string res;
    for (int i = 0; i < size; ++i) {
        for(int j = 0; j < 8; ++j) {
            res += (((str[i] >> j) & 1)? "1" : "0");
        }
    }
    return res;
}

TEST(HammingCodeTestSuite, EncodeTest) {
    // 0b 1001 0010 1110 0010;
    // 0b 1111 0010 0010 1111 0001 0001;
    char text[2]{};
    text[0] = char(0x49);
    text[1] = char(0x47);

    char encoded[3]{};
    char need_encoded[3]{};
    need_encoded[0] = char(0x4F);
    need_encoded[1] = char(0xF4);
    need_encoded[2] = char(0x88);
    hamming_code::HammingCoder<24, 16>::Encode(text, encoded);
    std::string expected = bin(need_encoded, 3);
    std::string result = bin(encoded, 3);
    ASSERT_EQ(expected, result);
}

TEST(HammingCodeTestSuite, DecodeTest) {
    // 0b 1001 0010 1110 0010;
    // 0b 1111 0010 0010 1111 0001 0001;
    char need_text[2]{};
    need_text[0] = char(0x49);
    need_text[1] = char(0x47);
    char text[2]{};

    char encoded[3]{};
    char need_encoded[3]{};
    need_encoded[0] = char(0x4F);
    need_encoded[1] = char(0xF4);
    need_encoded[2] = char(0x88);
    hamming_code::HammingCoder<24, 16>::Decode(need_encoded, text);
    std::string expected = bin(need_text, 2);
    std::string result = bin(text, 2);
    ASSERT_EQ(expected, result);
}


TEST(HammingCodeTestSuite, EncodeDecodeTest) {
    char text[] = "tex";
    char encoded[4];
    encoded[0] = -85;
    encoded[1] = 87;
    encoded[2] = 12;
    encoded[3] = 15;
    char decoded[3]{};
    hamming_code::HammingCoder<32, 24>::Decode(encoded, decoded);
    std::string expected = bin(text, 3);
    std::string result = bin(decoded, 3);
    ASSERT_EQ(expected, result);
}

TEST(HammingCodeTestSuite, CorrectionTest) {
    char text[] = "tex";
    char encoded[4];
    encoded[0] = -85;
    encoded[1] = 87;
    encoded[2] = 12;
    encoded[3] = 15;
    encoded[2] ^= (1 << 2);
    char decoded[3]{};
    hamming_code::HammingCoder<32, 24>::Decode(encoded, decoded);
    std::string expected = bin(text, 3);
    std::string result = bin(decoded, 3);
    ASSERT_EQ(expected, result);
}

TEST(HammingArchiveTestSuite, CreateTest) {
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    archiver::Archiver archive("test_archive",
                               {filename1, filename2},
                               block_size);
    ASSERT_NO_THROW(archive.Create());
}

TEST(HammingArchiveTestSuite, ListTest) {
    std::string archive_filename = "test_archive";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    archiver::Archiver archive(archive_filename,
                               {filename1, filename2},
                               block_size);
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
                                    = {{"files/test_text", 87},
                                       {"files/test_image.jpg", 30664}};
    ASSERT_EQ(expected_list, list);
    std::filesystem::remove(archive_filename);
}

void BreakArchiveBit(const std::string& name, size_t bit_number) {
    std::fstream file(name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error while opening the file archive" << std::endl;
        throw std::runtime_error("Error while opening the file archive for testing");
    }
    file.seekp(bit_number / 8);
    file.seekg(bit_number / 8);
    char newValue = file.peek() ^ (1 << (bit_number % 8));
    file.put(newValue);
    file.close();
}

TEST(HammingArchiveTestSuite, BreakOneBitTest) {
    std::string archive_filename = "broken_1_bit_archive.haf";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    archiver::Archiver archive(archive_filename,
                               {filename1, filename2},
                               block_size);
    archive.Create();
    BreakArchiveBit(archive_filename, 287);
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{"files/test_text", 87},
           {"files/test_image.jpg", 30664}};
    ASSERT_EQ(expected_list, list);
    std::filesystem::remove(archive_filename);
}

TEST(HammingArchiveTestSuite, BreakTwoBitsTest) {
    std::string archive_filename = "broken_2_bit_archive.haf";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    archiver::Archiver archive(archive_filename,
                               {filename1, filename2},
                               block_size);
    archive.Create();
    BreakArchiveBit(archive_filename, block_size - 8 );
    BreakArchiveBit(archive_filename, block_size + 3);
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{"files/test_text", 87},
           {"files/test_image.jpg", 30664}};
    ASSERT_EQ(expected_list, list);
    std::filesystem::remove(archive_filename);
}

void WriteDataToFile(const std::string& filename, std::string& data) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Could not open file for testing: " + filename << '\n';
        throw std::runtime_error("Could not open file for testing: " + filename);
    }
    out << data;
    out.close();
}
void ReadDataFromFile(const std::string& filename, std::string& data) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Could not open file for testing: " + filename << '\n';
        throw std::runtime_error("Could not open file for testing: " + filename);
    }
    char c;
    while (in.get(c)) {
        data += c;
    }
    in.close();
}

TEST(HammingArchiveTestSuite, DeleteSingleFileTest) {
    std::string archive_filename = "test_archive.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    WriteDataToFile(test_file_name, test_file_text);
    archiver::Archiver archive(archive_filename,
                               {test_file_name, filename1, filename2},
                               block_size);
    archive.Create();
    std::filesystem::remove(test_file_name);
    archive = archiver::Archiver(archive_filename, {test_file_name}, block_size);
    archive.DeleteFiles();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{"files/test_text", 87},
           {"files/test_image.jpg", 30664}};
    std::string data;
    ASSERT_EQ(expected_list, list);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, ExtractSingleFileTest) {
    std::string archive_filename = "test_archive.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    WriteDataToFile(test_file_name, test_file_text);
    archiver::Archiver archive(archive_filename,
                               {test_file_name, filename1, filename2},
                               block_size);
    archive.Create();
    std::filesystem::remove(test_file_name);
    archive = archiver::Archiver(archive_filename, {test_file_name}, block_size);
    archive.ExtractFiles();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{"files/test_text", 87},
           {"files/test_image.jpg", 30664}};
    std::string data;
    ASSERT_EQ(expected_list, list);
    ASSERT_NO_THROW(ReadDataFromFile(test_file_name, data));
    ASSERT_EQ(test_file_text, data);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, ExtractAllFilesTest) {
    std::string archive_filename = "test_archive.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    WriteDataToFile(test_file_name, test_file_text);
    archiver::Archiver archive(archive_filename,
                               { filename1, filename2, test_file_name},
                               block_size);
    archive.Create();
    std::filesystem::remove(test_file_name);
    archive = archiver::Archiver(archive_filename, {test_file_name, filename1, filename2}, block_size);
    archive.ExtractFiles();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {};
    std::string data;
    ASSERT_EQ(expected_list, list);
    ASSERT_NO_THROW(ReadDataFromFile(test_file_name, data));
    ASSERT_EQ(test_file_text, data);

    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, MergeTest) {
    std::string archive_filename_1 = "test_archive_1.haf";
    std::string archive_filename_2 = "test_archive_2.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    WriteDataToFile(test_file_name, test_file_text);
    archiver::Archiver archive1(archive_filename_1,
                               {test_file_name},
                               block_size);
    archive1.Create();
    std::filesystem::remove(test_file_name);
    archiver::Archiver archive2(archive_filename_2, {filename1, filename2}, block_size);
    archive2.Create();
    std::string archive_filename = "test_archive_3.haf";
    archiver::Archiver archive(archive_filename, {archive_filename_1, archive_filename_2}, block_size);
    archive.Concatenate();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = { {test_file_name, test_file_text.size()},
            {"files/test_text", 87},
            {"files/test_image.jpg", 30664}};
    std::string data;
    ASSERT_EQ(expected_list, list);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(archive_filename_1);
    std::filesystem::remove(archive_filename_2);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, MergeSameTest) {
    std::string archive_filename_1 = "test_archive_1.haf";
    std::string archive_filename_2 = "test_archive_2.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    WriteDataToFile(test_file_name, test_file_text);
    archiver::Archiver archive1(archive_filename_1,
                                {test_file_name},
                                block_size);
    archive1.Create();
    std::filesystem::remove(test_file_name);
    archiver::Archiver archive2(archive_filename_2, {filename1, filename2}, block_size);
    archive2.Create();
    std::string archive_filename = "test_archive_2.haf";
    archiver::Archiver archive(archive_filename, {archive_filename_1, archive_filename_2}, block_size);
    ASSERT_THROW(archive.Concatenate(), std::invalid_argument);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(archive_filename_1);
    std::filesystem::remove(archive_filename_2);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, AppendTest) {
    //std::string archive_filename_1 = "test_archive_1.haf";
    //std::string archive_filename_2 = "test_archive_2.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = "files/test_image.jpg";
    WriteDataToFile(test_file_name, test_file_text);
    std::string archive_filename = "test_archive_2.haf";
    archiver::Archiver archive(archive_filename, {filename1, filename2}, block_size);
    archive.Create();
    archive = archiver::Archiver(archive_filename,
                                 {test_file_name},
                                 block_size);
    archive.AddFiles();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = { {"files/test_text", 87},
            {"files/test_image.jpg", 30664},
            {test_file_name, test_file_text.size()}};
    ASSERT_EQ(list, expected_list);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}
