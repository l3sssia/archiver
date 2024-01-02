#include <gtest/gtest.h>
#include "lib/hamming_code.h"
#include "lib/archiver.h"
#include <string>

#define block_size 128

const std::string EMPTY_FILE_NAME = "files/empty_file";
const std::string EMPTY_FILE_REF_NAME = "files/empty_file_ref";

const std::string IMAGE_FILE_NAME = "files/test_image.jpg";
const std::string IMAGE_FILE_REF_NAME = "files/test_image_ref.jpg";
const size_t IMAGE_FILE_SIZE = 30664;

const std::string TEXT_FILE_NAME = "files/test_text";
const std::string TEXT_FILE_REF_NAME = "files/test_text_ref";
const size_t TEXT_FILE_SIZE = 87;


const std::string ARCHIVE_FILENAME = "test_archive";
const std::string ARCHIVE_FILENAME_HAF = "test_archive.haf";

bool AreFilesEqual(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);

    if (!f1.is_open() || !f2.is_open()) {
        throw std::runtime_error("Could not open one or both files for comparison.");
    }

    char byte1, byte2;
    while (true) {
        f1.get(byte1);
        f2.get(byte2);
        if (f1.eof() != f2.eof()) {
            return false;
        }
        if (byte1 != byte2) {
            return false;
        }
        if (f1.eof() || f2.eof()) {
            break;
        }
    }
    return f1.eof() && f2.eof();
}

void CopyFile(const std::string& source, const std::string& destination) {
    std::ifstream src(source, std::ios::binary);
    if (!src.is_open()) {
        throw std::runtime_error("Could not open source file for copying: " + source);
    }

    std::ofstream dest(destination, std::ios::binary);
    if (!dest.is_open()) {
        throw std::runtime_error("Could not open destination file for copying: " + destination);
    }

    dest << src.rdbuf();

    src.close();
    dest.close();
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

void ClearArchive() {
    std::filesystem::remove(ARCHIVE_FILENAME);
    std::filesystem::remove(ARCHIVE_FILENAME_HAF);
}

std::string bin(const char* str, size_t size) {
    std::string res;
    for (int i = 0; i < size; ++i) {
        for(int j = 0; j < 8; ++j) {
            res += (((str[i] >> j) & 1)? "1" : "0");
        }
    }
    return res;
}

TEST(HammingCodeTestSuite, EncodeSimpleTest) {
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
    EXPECT_EQ(expected, result);
}

TEST(HammingCodeTestSuite, DecodeSimpleTest) {
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
    EXPECT_EQ(expected, result);
}


TEST(HammingCodeTestSuite, EncodeDecodeStringTest) {
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
    EXPECT_EQ(expected, result);
}

TEST(HammingCodeTestSuite, CorrectionStringTest) {
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
    EXPECT_EQ(expected, result);
}

TEST(HammingArchiveTestSuite, CreateTest) {
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
    archiver::Archiver archive(ARCHIVE_FILENAME,
                               {filename1, filename2},
                               block_size);
    EXPECT_NO_THROW(archive.Create());
    ClearArchive();
}

TEST(HammingArchiveTestSuite, ListTest) {
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
    archiver::Archiver archive(ARCHIVE_FILENAME,
                               {filename1, filename2},
                               block_size);
    archive.Create();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
                                    = {{TEXT_FILE_NAME, TEXT_FILE_SIZE},
                                       {IMAGE_FILE_NAME, IMAGE_FILE_SIZE}};
    EXPECT_EQ(expected_list, list);
    ClearArchive();
}

TEST(HammingArchiveTestSuite, BreakOneBitTest) {
    std::string archive_filename = "broken_1_bit_archive.haf";
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
    archiver::Archiver archive(ARCHIVE_FILENAME,
                               {filename1, filename2},
                               block_size);
    archive.Create();
    BreakArchiveBit(ARCHIVE_FILENAME_HAF, 287);
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{filename1, TEXT_FILE_SIZE},
           {filename2, IMAGE_FILE_SIZE}};
    EXPECT_EQ(expected_list, list);
    ClearArchive();
}

TEST(HammingArchiveTestSuite, BreakTwoBitsTest) {
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
    archiver::Archiver archive(ARCHIVE_FILENAME,
                               {filename1, filename2},
                               block_size);
    archive.Create();
    BreakArchiveBit(ARCHIVE_FILENAME_HAF, block_size - 8 );
    BreakArchiveBit(ARCHIVE_FILENAME_HAF, block_size + 3);
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{TEXT_FILE_NAME, TEXT_FILE_SIZE},
           {IMAGE_FILE_NAME, IMAGE_FILE_SIZE}};
    EXPECT_EQ(expected_list, list);
    ClearArchive();
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
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = IMAGE_FILE_NAME;
    WriteDataToFile(test_file_name, test_file_text);
    archiver::Archiver archive(ARCHIVE_FILENAME,
                               {test_file_name, filename1, filename2},
                               block_size);
    archive.Create();
    std::filesystem::remove(test_file_name);
    archive = archiver::Archiver(ARCHIVE_FILENAME, {test_file_name}, block_size);
    archive.DeleteFiles();
    std::vector<std::pair<std::string, uint64_t>> list = archive.List();
    std::vector<std::pair<std::string, uint64_t>> expected_list
        = {{TEXT_FILE_NAME, TEXT_FILE_SIZE},
           {IMAGE_FILE_NAME, IMAGE_FILE_SIZE}};
    std::string data;
    EXPECT_EQ(expected_list, list);
    std::filesystem::remove(ARCHIVE_FILENAME);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, ExtractSingleFileTest) {
    std::string archive_filename = ARCHIVE_FILENAME_HAF;
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
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
        = {{TEXT_FILE_NAME, TEXT_FILE_SIZE},
           {IMAGE_FILE_NAME, IMAGE_FILE_SIZE}};
    std::string data;
    EXPECT_EQ(expected_list, list);
    EXPECT_NO_THROW(ReadDataFromFile(test_file_name, data));
    EXPECT_EQ(test_file_text, data);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, ExtractAllFilesTest) {
    std::string archive_filename = "test_archive.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = "files/test_text";
    std::string filename2 = IMAGE_FILE_NAME;
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
    EXPECT_EQ(expected_list, list);
    EXPECT_NO_THROW(ReadDataFromFile(test_file_name, data));
    EXPECT_EQ(test_file_text, data);

    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, MergeTest) {
    std::string archive_filename_1 = "test_archive_1.haf";
    std::string archive_filename_2 = "test_archive_2.haf";
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
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
            {TEXT_FILE_NAME, TEXT_FILE_SIZE},
            {IMAGE_FILE_NAME, IMAGE_FILE_SIZE}};
    std::string data;
    EXPECT_EQ(expected_list, list);
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
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
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
    EXPECT_THROW(archive.Concatenate(), std::invalid_argument);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(archive_filename_1);
    std::filesystem::remove(archive_filename_2);
    std::filesystem::remove(test_file_name);
}

TEST(HammingArchiveTestSuite, AppendTest) {
    std::string test_file_name = "break_file";
    std::string test_file_text = "1234567890. is there all digits from 0 to 9?";
    std::string filename1 = TEXT_FILE_NAME;
    std::string filename2 = IMAGE_FILE_NAME;
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
        = { {TEXT_FILE_NAME, TEXT_FILE_SIZE},
            {IMAGE_FILE_NAME, IMAGE_FILE_SIZE},
            {test_file_name, test_file_text.size()}};
    EXPECT_EQ(list, expected_list);
    std::filesystem::remove(archive_filename);
    std::filesystem::remove(test_file_name);
}

TEST(HammingCodeTestSuite, EmptyFileTest) {
    std::string input = EMPTY_FILE_NAME;
    std::string ref = EMPTY_FILE_REF_NAME;
    CopyFile(input, ref);
    archiver::Archiver archiver("test_archive", {input}, block_size);
    archiver.Create();
    EXPECT_NO_THROW(archiver.AddFiles());
    archiver.ExtractFiles();
    EXPECT_TRUE(AreFilesEqual(input, ref));
    std::filesystem::remove(ARCHIVE_FILENAME);
}

TEST(HammingCodeTestSuite, TextTest) {
    std::string input = TEXT_FILE_NAME;
    std::string ref = TEXT_FILE_REF_NAME;
    CopyFile(input, ref);
    archiver::Archiver archiver(ARCHIVE_FILENAME, {input}, block_size);
    archiver.Create();
    EXPECT_NO_THROW(archiver.AddFiles());
    archiver.ExtractFiles();
    EXPECT_TRUE(AreFilesEqual(input, ref));
    std::filesystem::remove(ARCHIVE_FILENAME);
}

TEST(HammingCodeTestSuite, ImageTest) {
    std::string input = IMAGE_FILE_NAME;
    std::string ref = IMAGE_FILE_REF_NAME;
    CopyFile(input, ref);
    archiver::Archiver archiver(ARCHIVE_FILENAME, {input}, block_size);
    archiver.Create();
    EXPECT_NO_THROW(archiver.AddFiles());
    archiver.ExtractFiles();
    EXPECT_TRUE(AreFilesEqual(input, ref));
    std::filesystem::remove(ARCHIVE_FILENAME);
}


TEST(HammingCodeTestSuite, BrokenTextTest) {
    std::string input = TEXT_FILE_NAME;
    std::string ref = TEXT_FILE_REF_NAME;
    CopyFile(input, ref);

    archiver::Archiver archiver(ARCHIVE_FILENAME, {input}, block_size);

    EXPECT_NO_THROW(archiver.Create());
    EXPECT_NO_THROW(archiver.AddFiles());

    BreakArchiveBit(ARCHIVE_FILENAME_HAF, 239);

    EXPECT_NO_THROW(archiver.ExtractFiles());

    EXPECT_TRUE(AreFilesEqual(input, ref));

    std::filesystem::remove(ARCHIVE_FILENAME);
}



TEST(HammingCodeTestSuite, BrokenImageTest) {
    std::string input = IMAGE_FILE_NAME;
    std::string ref = IMAGE_FILE_REF_NAME;
    CopyFile(input, ref);

    archiver::Archiver archiver(ARCHIVE_FILENAME, {input}, block_size);

    EXPECT_NO_THROW(archiver.Create());
    EXPECT_NO_THROW(archiver.AddFiles());

    BreakArchiveBit(ARCHIVE_FILENAME_HAF, 239);
    BreakArchiveBit(ARCHIVE_FILENAME_HAF, 1010101);

    EXPECT_NO_THROW(archiver.ExtractFiles());

    EXPECT_TRUE(AreFilesEqual(input, ref));

    std::filesystem::remove(ARCHIVE_FILENAME);
}
