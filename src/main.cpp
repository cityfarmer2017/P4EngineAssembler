/**
 * Copyright [2024] <wangdc1111@gmail.com>
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
    std::cout << "Usage:\n";
    std::cout << BGREEN << "\tP4eAsm " << CYAN << "<-h/--help>\n";
    std::cout << BGREEN << "\tP4eAsm " << MAGENTA << "<path-to-source>" << BLUE << " <path-to-destination>\n";
    std::cout << BGREEN << "\tP4eAsm " << MAGENTA << "<path-to-source>\n";
    #if !NO_PRE_PROC
    std::cout << BGREEN << "\tP4eAsm " << BWHITE << "<-i>" << MAGENTA << " <path-to-source>\n";
    #endif
    std::cout << RESET << "Examples:\n";
    std::cout << BGREEN << "\tP4eAsm " << CYAN << "-h\n";
    std::cout << BGREEN << "\tP4eAsm " << MAGENTA << "./sample.p4p" << BLUE << " ./dst\n";
    std::cout << BGREEN << "\tP4eAsm " << MAGENTA << "./\n";
    #if !NO_PRE_PROC
    std::cout << BGREEN << "\tP4eAsm " << BWHITE << "-i" << MAGENTA << " ./\n";
    #endif
    std::cout << RESET << "Info:\n";
    std::cout << BYELLOW << "\t\"path-to-source\" is always required, \"path-to-destination\" is optional.\n";
    std::cout << "\tif no \"path to destination\" provided, outcome will be in same place of source.\n";
    #if !NO_PRE_PROC
    std::cout << "\twith \"-i\" option, generate only intermediate files in same place of source.\n";
    #endif
    std::cout << std::flush;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        if (string(argv[1]) == "-h" || string(argv[1]) == "--help") {
            print_help_information();
            return 0;
        }
    } else if (argc != 3) {
        std::cout << BRED << "ERROR: argument count!\n" << RESET;
        print_help_information();
        return -1;
    }

    #if !NO_PRE_PROC
    if (string(argv[1]) == "-i") {
        if (!std::filesystem::exists(argv[2])) {
            std::cout << BRED << "ERROR: the path to source file(s) dose not exist.\n" << RESET;
            print_help_information();
            return -1;
        }
        if (nullptr == assembler::execute_pre_process(argv[2])) {
            return -1;
        }
        return 0;
    }
    #endif

    if (!std::filesystem::exists(argv[1])) {
        std::cout << BRED << "ERROR: the path to source file(s) dose not exist.\n" << RESET;
        print_help_information();
        return -1;
    }

    if (argc == 3 && std::filesystem::exists(argv[2]) && !std::filesystem::is_directory(argv[2])) {
        std::cout << BRED << "ERROR: the path to destination must be a directory, otherwise empty.\n" << RESET;
        print_help_information();
        return -1;
    }

    return assembler::handle(argv[1], argc == 3 ? argv[2] : "");
}
