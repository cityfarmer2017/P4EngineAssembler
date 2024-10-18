/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <fstream>
#include <iostream>
#include <regex>  // NOLINT
#include "../../inc/pre_proc/preprocessor.h"
#include "../../inc/global_def.h"

constexpr auto IMPORT_LINE_P = R"(^\.import\s+\"((\w+\/)*\w+(\.p4[pdm])?)\"\s*(\/\/.*)?[\n\r]?$)";
constexpr auto INCLUDE_LINE_P = R"(^\.include\s+\"((\w+\/)*\w+\.p4h)\"\s*(\/\/.*)?[\n\r]?$)";
constexpr auto ASSIGN_LINE_P = R"(^\.assign\s+(\w+)\s+(\d+|0[xX][0-9A-F]+)\s*(\/\/.*)?[\n\r]?$)";
constexpr auto NORMAL_LINE_P = R"(^(.+[;:]|\.start|\.end)\s*(\/\/.*)?[\n\r]?$)";

std::string replace_all(std::string str, const std::string &from, const std::string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string preprocessor::replace_macros(std::string orignal) {
    for (const auto &token : token_to_val) {
        if (orignal.find(token.first)) {
            orignal = replace_all(orignal, token.first, token.second);
        }
    }
    return orignal;
}

int preprocessor::process_imported_file(const std::filesystem::path &path,
    const std::string &dst_fname, std::uint16_t &code_line_idx, std::ofstream &ot_fstrm) {
    if (!std::filesystem::exists(path)) {
        std::cout << "imported file - " << path.string() << " dose not exist." << std::endl;
        return -1;
    }

    auto src_fname = path.filename().string();

    auto in_fstrm = std::ifstream(path);
    auto line = std::string();
    auto file_line_idx = 0UL;
    while (std::getline(in_fstrm, line)) {
        ++file_line_idx;

        const auto re1 = std::regex(COMMENT_EMPTY_LINE_P);
        if (std::regex_match(line, re1)) {
            continue;
        }

        auto m = std::smatch();

        const auto re4 = std::regex(ASSIGN_LINE_P);
        if (std::regex_match(line, m, re4)) {
            token_to_val[m.str(1)] = m.str(2);
            continue;
        }

        const auto re5 = std::regex(NORMAL_LINE_P);
        if (!std::regex_match(line, m, re5)) {
            std::cout << "ERROR: line matching\n\t" << line;
            std::cout << "\n\tin file - " << src_fname << "\tline - #" << file_line_idx << std::endl;
            return -1;
        }

        ot_fstrm << replace_macros(m.str(1)) << "\n";

        ++code_line_idx;

        dst_cline_2_src_fline.emplace(
            std::make_pair(dst_fname + std::to_string(code_line_idx), std::make_pair(src_fname, file_line_idx)));
    }

    return 0;
}

int preprocessor::process_included_file(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        std::cout << "included file - " << path.string() << " dose not exist." << std::endl;
        return -1;
    }

    auto in_fstrm = std::ifstream(path);
    auto src_fname = path.filename().string();
    auto line = std::string();
    auto file_line_idx = 0UL;
    while (std::getline(in_fstrm, line)) {
        ++file_line_idx;

        const auto re1 = std::regex(COMMENT_EMPTY_LINE_P);
        if (std::regex_match(line, re1)) {
            continue;
        }

        auto m = std::smatch();

        const auto re4 = std::regex(ASSIGN_LINE_P);
        if (!std::regex_match(line, m, re4)) {
            std::cout << "ERROR: line matching\n\t" << line;
            std::cout << "\n\tin file - " << src_fname << "\tline - #" << file_line_idx << std::endl;
            return -1;
        }

        token_to_val[m.str(1)] = m.str(2);
    }

    return 0;
}

int preprocessor::process_single_entry(const std::filesystem::path &path) {
    auto src_fext = path.extension().string();
    if ((src_fext != ".p4p") && (src_fext != ".p4m") && (src_fext != ".p4d")) {
        return -1;
    }

    auto src_dir = (path.has_parent_path() ? path.parent_path() : std::filesystem::current_path()).string();
    if (src_dir.back() != '/') {
        src_dir += "/";
    }
    auto src_fname = path.filename().string();
    auto in_fstrm = std::ifstream(path);
    if (!in_fstrm.is_open()) {
        std::cout << "failed to open " << path.string() << std::endl;
        return -1;
    }

    auto dst_dir = src_dir + IR_PATH;
    if (!std::filesystem::exists(dst_dir)) {
        if (!std::filesystem::create_directories(dst_dir)) {
            std::cout << "failed to create directory: " << dst_dir << std::endl;
            return -1;
        }
    }

    auto dst_fname = src_fname + IR_STR;
    auto ot_fstrm = std::ofstream(dst_dir + dst_fname);
    if (!ot_fstrm.is_open()) {
        std::cout << "failed to open " << dst_dir + dst_fname << std::endl;
        return -1;
    }

    std::uint16_t file_line_idx = 0, code_line_idx = 0;

    auto line = std::string();
    while (std::getline(in_fstrm, line)) {
        ++file_line_idx;

        const auto re1 = std::regex(COMMENT_EMPTY_LINE_P);
        if (std::regex_match(line, re1)) {
            continue;
        }

        auto m = std::smatch();

        const auto re2 = std::regex(IMPORT_LINE_P);
        if (std::regex_match(line, m, re2)) {
            if (auto rc = process_imported_file(src_dir + m.str(1), dst_fname, code_line_idx, ot_fstrm)) {
                return rc;
            }
            continue;
        }

        const auto re3 = std::regex(INCLUDE_LINE_P);
        if (std::regex_match(line, m, re3)) {
            if (auto rc = process_included_file(src_dir + m.str(1))) {
                return rc;
            }
            continue;
        }

        const auto re4 = std::regex(ASSIGN_LINE_P);
        if (std::regex_match(line, m, re4)) {
            token_to_val[m.str(1)] = m.str(2);
            continue;
        }

        const auto re5 = std::regex(NORMAL_LINE_P);
        if (!std::regex_match(line, m, re5)) {
            std::cout << "ERROR: line matching\n\t" << line;
            std::cout << "\n\tin file - " << src_fname << "\tline - #" << file_line_idx << std::endl;
            return -1;
        }

        ot_fstrm << replace_macros(m.str(1)) << "\n";

        ++code_line_idx;

        dst_cline_2_src_fline.emplace(
            std::make_pair(dst_fname + std::to_string(code_line_idx), std::make_pair(src_fname, file_line_idx)));
    }

    in_fstrm.close();
    ot_fstrm << std::flush;
    ot_fstrm.close();

    return 0;
}

int preprocessor::process_multi_entries(const std::filesystem::path &path) {
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto ext = entry.path().extension().string();
        if ((ext != ".p4p") && (ext != ".p4m") && (ext != ".p4d")) {
            continue;
        }

        if (auto rc = process_single_entry(entry.path())) {
            return rc;
        }
    }
    return 0;
}

int preprocessor::handle(const std::filesystem::path &path) {
    if (std::filesystem::is_regular_file(path)) {
        return process_single_entry(path);
    } else {
        return process_multi_entries(path);
    }
    return 0;
}
