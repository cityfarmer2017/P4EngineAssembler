#include <memory>
#include <filesystem>
#include "parser_assembler.h"
#include "deparser_assembler.h"

int process_one_entry(const std::filesystem::directory_entry &entry, const string &out_path)
{
    string src_dir(entry.path().parent_path());
    string src_fname(entry.path().filename());
    string src_fstem(entry.path().stem());
    string src_fext(entry.path().extension());

    string dst_fname(out_path);
    if (!out_path.empty()) {
        std::filesystem::path dst_dir(out_path);
        if (!std::filesystem::exists(dst_dir)) {
            if (!std::filesystem::create_directories(dst_dir)) {
                std::cout << "failed to create directory: " << dst_dir << std::endl;
                return -1;
            }
        }
    } else {
        dst_fname = src_dir;
    }

    if (dst_fname.back() != '/') {
        dst_fname += "/";
    }

    std::unique_ptr<assembler> p_asm;
    if (src_fext == ".p4p") {
        p_asm = std::make_unique<parser_assembler>();
        dst_fname += "parser_";
    } else if (src_fext == ".p4m") {
        std::cout << "MAT operation" << std::endl;
        dst_fname += "mat_";
    } else if(src_fext == ".p4d") {
        p_asm = std::make_unique<deparser_assembler>();
        dst_fname += "deparser_";
    } else {
        return 0;
    }

    dst_fname += src_fstem;

    #ifdef DEBUG
    std::cout << dst_fname << "\n";
    #endif

    return p_asm->execute(src_fname, dst_fname);
}

inline void print_help_information()
{
    std::cout << "Usage:\n\t";
    std::cout << "P4eAsm \"path to source (file / directory)\" [\"path to destination (directory)\"]\n\t";
    std::cout << "P4eAsm \"-h\" or \"--help\" for help.\n";
    std::cout << "Examples:\n\t";
    std::cout << "P4eAsm ./parser_sample.p4p ./dst\n\t";
    std::cout << "P4eAsm ./\n";
    std::cout << "Info:\n\t";
    std::cout << "source file posfix - .p4p for parser, .p4d for deparser, .p4m for mat\n\t";
    std::cout << "destination file posfix - .dat for binary format, .txt for plain text\n";
    std::cout << std::flush;
}

int main(int argc, char* argv[])
{
    if (argc == 2) {
        if (string(argv[1]) == "-h" || string(argv[1]) == "--help") {
            print_help_information();
            return 0;
        }
    } else if (argc != 3) {
        print_help_information();
        return -1;
    }

    if (!std::filesystem::exists(argv[1])) {
        std::cout << "the path to source file(s) dose not exists." << std::endl;
        print_help_information();
        return -1;
    }

    if (argc == 3 && std::filesystem::exists(argv[2]) && !std::filesystem::is_directory(argv[2])) {
        std::cout << "the 2nd argument must be a directory path to destination files, otherwise empty." << std::endl;
        print_help_information();
        return -1;
    }

    string out_path(argc == 3 ? argv[2] : "");

    if (std::filesystem::is_regular_file(argv[1])) {
        return process_one_entry(std::filesystem::directory_entry(argv[1]), out_path);
    }

    for (const auto &entry: std::filesystem::directory_iterator(argv[1])) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (auto rc = process_one_entry(entry, out_path)) {
            return rc;
        }
    }
}