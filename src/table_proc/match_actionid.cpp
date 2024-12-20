/**
 * Copyright [2024] <wangdc1111@gmail.com>
 */
#include <iostream>
#include <regex>  // NOLINT [build/c++11]
#include "../../inc/parser_assembler.h"
#include "../../inc/table_proc/match_actionid.h"

constexpr auto ALL_X_STR40 = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

int match_actionid::generate_table_data(const std::shared_ptr<assembler> &p_asm) {
    auto in_path = input_path + "tables_parser/";
    #if !NO_PRE_PROC
    in_path = input_path + "../tables_parser/";
    #endif
    if (!std::filesystem::exists(in_path)) {
        std::cout << "no 'tables_parser' sub directory under current sourc code path: " << input_path << std::endl;
        return -1;
    }

    if (auto rc = generate_sram_data(in_path, p_asm)) {
        return rc;
    }

    if (!std::filesystem::exists(otput_path)) {
        if (!std::filesystem::create_directories(otput_path)) {
            std::cout << "failed to create directory: " << otput_path << std::endl;
            return -1;
        }
    }

    if (auto rc = output_sram_data(otput_path)) {
        return rc;
    }

    return 0;
}

int match_actionid::generate_sram_data(const std::string &src_dir, const std::shared_ptr<assembler> &p_asm) {
    auto p_parser_asm = std::dynamic_pointer_cast<parser_assembler>(p_asm);

    for (const auto &entry : std::filesystem::recursive_directory_iterator(src_dir)) {
        const std::regex r(R"(^(\w+)_(\d{3}).tcam$)");
        std::smatch m;
        auto str = entry.path().filename().string();
        if (!regex_match(str, m, r)) {
            std::cout << "tcam file pattern does not match." << std::endl;
            return -1;
        } else if (m.str(1) != p_parser_asm->src_file_stem()) {
            continue;
        } else if (!p_parser_asm->state_line_sub_map.count(std::stoul(m.str(2)))) {
            std::cout << "tcam file does not match state no." << std::endl;
            return -1;
        } else {
            tcam_file_paths.emplace(entry.path());
        }
    }

    if (tcam_file_paths.size() > MAX_STATE_NO + 1) {
        std::cout << "the number of tcam files shall not exceed " << MAX_STATE_NO + 1 << std::endl;
        return -1;
    }

    if (tcam_file_paths.size() != p_parser_asm->state_line_sub_map.size()) {
        std::cout << "the number of tcam files does not match sate chart generated from source code." << std::endl;
        return -1;
    }

    auto it = p_parser_asm->state_line_sub_map.begin();

    for (const auto &tcam_file_path : tcam_file_paths) {
        std::ifstream in_file_strm(tcam_file_path);
        if (!in_file_strm.is_open()) {
            std::cout << "cannot open: " << tcam_file_path.string() << std::endl;
            return -1;
        }

        std::vector<vec_of_str> str_vec_vec{20, vec_of_str{4, ALL_1_STR32}};

        std::string line;
        std::size_t sz = 0;
        while (getline(in_file_strm, line)) {
            if (trim_all(line).size() > 40) {
                std::cout << "ERROR: line is too long.\n\t" << line << std::endl;
                return -1;
            }
            for (const auto &c : line) {
                if (c != '0' && c != '1' && c != 'x') {
                    std::cout << "tcam entry shall only include '0', '1' or 'x'." << std::endl;
                    return -1;
                }
            }

            ++sz;

            if (line == ALL_X_STR40) {
                continue;
            }

            vec_of_str tcam_entry_vec;
            for (auto i = 0UL; i < line.size(); ++i, ++i) {
                tcam_entry_vec.emplace_back(line.substr(i, 2));
            }

            std::reverse(tcam_entry_vec.begin(), tcam_entry_vec.end());

            auto i = 0UL;
            for (const auto &c2 : tcam_entry_vec) {
                auto c4 = c2_c4_map.at(c2);
                str_vec_vec[i][0][32-sz] = c4[0];
                str_vec_vec[i][1][32-sz] = c4[1];
                str_vec_vec[i][2][32-sz] = c4[2];
                str_vec_vec[i][3][32-sz] = c4[3];
                ++i;
            }
        }

        if (sz != 32) {
            std::cout << "a tcam table must be exactly 32 entries." << std::endl;
            return -1;
        }

        for (auto i = 0UL; i < line.size() / 2; ++i) {
            match_sram_chip_vec[i][it->first * 4 + 0] = str_vec_vec[i][0];
            match_sram_chip_vec[i][it->first * 4 + 1] = str_vec_vec[i][1];
            match_sram_chip_vec[i][it->first * 4 + 2] = str_vec_vec[i][2];
            match_sram_chip_vec[i][it->first * 4 + 3] = str_vec_vec[i][3];
        }

        auto it_inner = (it->second).cbegin();
        for (auto i = 0UL; i < it->second.size(); ++i) {
            actionid_sram_vec[it->first * MATCH_ENTRY_CNT_PER_STATE + i] = it_inner->first;
            ++it_inner;
        }

        ++it;
    }

    return 0;
}

int match_actionid::output_sram_data(const std::string &dst_file_stem) {
    std::ofstream ot_file_strm(dst_file_stem + "match.txt");
    if (!ot_file_strm.is_open()) {
        std::cout << "cannot open dest file: " << dst_file_stem + "match.txt" << std::endl;
        return -1;
    }

    auto i = 0UL;
    std::vector<std::uint32_t> u32_vec;
    for (const auto &sram_chip_str_vec : match_sram_chip_vec) {
        ot_file_strm << "//sram chip #" << std::setw(2) << std::setfill('0') << i++ << "\n";
        for (const auto &entry_str : sram_chip_str_vec) {
            ot_file_strm << entry_str << "\n";
            u32_vec.emplace_back(std::stoul(entry_str, nullptr, 2));
        }
    }
    ot_file_strm << std::flush;
    ot_file_strm.close();

    ot_file_strm.open(dst_file_stem + "match.dat", std::ios::binary);
    if (!ot_file_strm.is_open()) {
        std::cout << "cannot open dest file: " << dst_file_stem + "match.dat" << std::endl;
        return -1;
    }
    ot_file_strm.write(reinterpret_cast<const char*>(u32_vec.data()), sizeof(u32_vec[0]) * u32_vec.size());
    ot_file_strm.close();

    ot_file_strm.open(dst_file_stem + "action_id.dat", std::ios::binary);
    if (!ot_file_strm.is_open()) {
        std::cout << "cannot open dest file: " << dst_file_stem + "action_id.dat" << std::endl;
        return -1;
    }
    ot_file_strm.write(reinterpret_cast<const char*>(actionid_sram_vec.data()),
                                                            sizeof(actionid_sram_vec[0]) * actionid_sram_vec.size());
    ot_file_strm.close();

    ot_file_strm.open(dst_file_stem + "action_id.txt");
    if (!ot_file_strm.is_open()) {
        std::cout << "cannot open dest file: " << dst_file_stem + "action_id.dat" << std::endl;
        return -1;
    }
    for (const auto &val : actionid_sram_vec) {
        ot_file_strm << std::bitset<16>(val) << "\n";
    }
    ot_file_strm << std::flush;
    ot_file_strm.close();

    return 0;
}

const std::unordered_map<std::string, std::string> match_actionid::c2_c4_map = {
    {"00", "1000"},
    {"01", "0100"},
    {"10", "0010"},
    {"11", "0001"},
    {"0x", "1100"},
    {"x0", "1010"},
    {"1x", "0011"},
    {"x1", "0101"},
    {"xx", "1111"}
};
