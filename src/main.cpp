/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <filesystem>
#include <iostream>
#include "../inc/assembler.h"

constexpr auto RESET =      "\033[0m";
constexpr auto BLACK =      "\033[30m";
constexpr auto RED =        "\033[31m";
constexpr auto GREEN =      "\033[32m";
constexpr auto YELLOW =     "\033[33m";
constexpr auto BLUE =       "\033[34m";
constexpr auto MAGENTA =    "\033[35m";
constexpr auto CYAN =       "\033[36m";
constexpr auto WHITE =      "\033[37m";
constexpr auto BBLACK =     "\033[1m\033[30m";
constexpr auto BRED =       "\033[1m\033[31m";
constexpr auto BGREEN =     "\033[1m\033[32m";
constexpr auto BYELLOW =    "\033[1m\033[33m";
constexpr auto BBLUE =      "\033[1m\033[34m";
constexpr auto BMAGENTA =   "\033[1m\033[35m";
constexpr auto BCYAN =      "\033[1m\033[36m";
constexpr auto BWHITE =     "\033[1m\033[37m";

inline void print_help_information() {
    std::cout << "Usage:\n\t";
    std::cout << BGREEN << "P4eAsm " << MAGENTA << "<path-to-source>" << RED << " <path-to-destination>\n\t";
    std::cout << BGREEN << "P4eAsm " << MAGENTA << "<path-to-source>\n\t";
    std::cout << BGREEN << "P4eAsm " << CYAN << "<-h/--help>\n";
    std::cout << RESET << "Examples:\n\t";
    std::cout << BGREEN << "P4eAsm " << MAGENTA << "./sample.p4p" << RED << " ./dst\n\t";
    std::cout << BGREEN << "P4eAsm " << MAGENTA << "./\n\t";
    std::cout << BGREEN << "P4eAsm " << CYAN << "-h\n";
    std::cout << RESET << "Info:\n\t";
    std::cout << BYELLOW << "source file extensions - \".p4p\" for parser, .p4d for deparser, \".p4m\" for mat.\n\t";
    std::cout << "destination file extensions - \".dat\" for binary format, \".txt\" for plain text.\n\t";
    std::cout << "if no \"path to destination\" provided, outcome will be in same place of source.\n\n";
    std::cout << std::flush;
}

int main(int argc, char* argv[]) {
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

    return assembler::handle(argv[1], argc == 3 ? argv[2] : "");
}
