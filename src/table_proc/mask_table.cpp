/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <iostream>
#include <filesystem>
#include "../../inc/deparser_assembler.h"
#include "../../inc/table_proc/mask_table.h"

constexpr auto MASK_LINE_LEN = 512UL;

int mask_table::generate_table_data(const std::shared_ptr<assembler> &p_asm) {
    auto in_path = input_path + "tables_deparser/";
    if (!std::filesystem::exists(in_path)) {
        #if DEBUG
        std::cout << "no 'tables_deparser' sub directory under current sourc code path: " << input_path << std::endl;
        #endif
        return 0;
    }

    auto file_name = p_asm->get_cur_src_file_name() + ".msk";
    if (!std::filesystem::exists(in_path + file_name)) {
        #if DEBUG
        std::cout << file_name << "does not exists in path: " << in_path << std::endl;
        #endif
        return 0;
    }

    std::vector<std::string> mask_strings;
    auto in_file_strm = std::ifstream(in_path + file_name);
    auto line = std::string();
    const auto r1 = std::regex(comment_empty_line_p);
    const auto r2 = std::regex(mask_line_pattern);
    const auto r3 = std::regex(mask_off_len_pattern);
    while (std::getline(in_file_strm, line)) {
        if (regex_match(line, r1)) {
            continue;
        }

        if (!regex_match(line, r2)) {
            std::cout << "mask line pattern does not match 'offset:length'." << std::endl;
            return -1;
        }

        auto str_strm = std::istringstream(line);
        auto str = std::string();
        auto mask = std::string(MASK_LINE_LEN, '0');
        auto prev_off = -1;
        auto total_len = 0UL;
        while (std::getline(str_strm, str, '_')) {
            auto m = std::smatch();
            if (!regex_match(str, m, r3)) {
                std::cout << "the 'offset:length' values are not rational." << std::endl;
                return -1;
            }

            auto off = std::stoul(m.str(1));
            auto len = std::stoul(m.str(2));
            if (prev_off >= static_cast<int>(off)) {
                std::cout << "offset shall be ascending from left to right." << std::endl;
                return -1;
            }

            prev_off = off;
            total_len += len;

            if (total_len >= MASK_LINE_LEN || off + len >= MASK_LINE_LEN) {
                std::cout << "total length or 'offset+length' exceed rational value." << std::endl;
                return -1;
            }

            auto rep_str = std::string(len, '1');
            mask.replace(off, len, rep_str);
        }

        std::reverse(mask.begin(), mask.end());
        mask_strings.emplace_back(mask);
    }

    auto ot_path = otput_path + "deparser_tables/";
    if (!std::filesystem::exists(ot_path)) {
        if (!std::filesystem::create_directories(ot_path)) {
            std::cout << "failed to create directory: " << ot_path << std::endl;
            return -1;
        }
    }

    auto ot_file_name = ot_path + p_asm->get_cur_src_file_name();
    auto ot_file_strm = std::ofstream(ot_file_name + "_msk.txt");
    if (!ot_file_strm.is_open()) {
        std::cout << "cannot open dest file: " << ot_file_name + "_msk.txt" << std::endl;
        return -1;
    }

    std::vector<std::uint64_t> u64_vals;
    for (const auto &str : mask_strings) {
        ot_file_strm << str << "\n";

        for (auto i = str.size() - 64; static_cast<int>(i) >= 0; i -= 64) {
            u64_vals.emplace_back(std::stoull(str.substr(i, 64), nullptr, 2));
        }
    }
    ot_file_strm << std::flush;
    ot_file_strm.close();

    ot_file_strm.open(ot_file_name + "_msk.dat", std::ios::binary);
    if (!ot_file_strm.is_open()) {
        std::cout << "cannot open dest file: " << ot_file_name + "_msk.dat" << std::endl;
        return -1;
    }
    ot_file_strm.write(reinterpret_cast<const char*>(u64_vals.data()), sizeof(u64_vals[0]) * u64_vals.size());
    ot_file_strm.close();

    return 0;
}

const char* mask_table::mask_line_pattern = R"(^\d+:\d+(_\d+:\d+)*[\n\r]?$)";
const char* mask_table::mask_off_len_pattern =
    R"(^([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01]):([1-9][0-9]?|[1-4][0-9]{2}|50[0-9]|51[01])$)";
