#include "archiver.h"
#include "ArgParser.h"

using namespace ArgumentParser;

int main(int argc, char** argv) {
    std::vector<std::string> files{};
    ArgParser parser("Hamming Archiver");
    parser.AddStringArgument('f', "file", "archive file name");
    parser.AddFlag('c', "create", "create new archive with given files");
    parser.AddFlag('a', "append", "add one and more files to archive");
    parser.AddFlag('x', "extract", "extract files from archive");
    parser.AddFlag('d', "delete", "delete given files from archive");
    parser.AddFlag('A', "concatenate", "merge two archives");
    parser.AddFlag('l', "list", "show list of archives");
    parser.AddStringArgument('n', "name").MultiValue().Positional().StoreValues(files);
    parser.Parse(argc, argv);
    std::string archive_name = parser.GetStringValue('f');
    archiver::Archiver archive(archive_name, files);
    if (parser.GetFlag('c')) {
        archive.Create();
    } else if (parser.GetFlag('a')) {
        archive.AddFiles();
    } else if (parser.GetFlag('x')) {
        archive.ExtractFiles();
    } else if (parser.GetFlag('d')) {
        archive.DeleteFiles();
    } else if (parser.GetFlag('A')) {
        if (files.size() != 2) {
            std::cerr << "Only 2 archives can be concatenated\n";
            return 0;
        }
        archive.Concatenate();
    } else if (parser.GetFlag('l')) {
        std::vector<std::pair<std::string, uint64_t>> file_list = archive.List();
        std::cout << "Archive contains " << file_list.size() << " files:\n";
        for (size_t i = 0; i < file_list.size(); ++i) {
            std::cout << i+1 << ") " << file_list[i].first <<
            "    " << file_list[i].second << " bytes\n";
        }
    }
}