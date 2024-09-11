/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <filesystem>
#include <iostream>
#include "../../inc/mat_assembler.h"
#include "../../inc/mat_def.h"
#include "../../inc/table_proc/mat_link.h"

constexpr auto tcam_table_entry_cnt = 512;
constexpr auto hash_table_entry_cnt = 1024;

int mat_link::generate_table_data(const std::shared_ptr<assembler> &p_asm) {
    auto in_path = input_path + "tables_mat/";
    if (!std::filesystem::exists(in_path)) {
        std::cout << in_path << " dose not exist." << std::endl;
        return -1;
    }

    auto file_name = p_asm->src_file_name() + ".link";
    if (!std::filesystem::exists(in_path + file_name)) {
        std::cout << file_name << " does not exists in path: " << in_path << std::endl;
        return -1;
    }

    if (auto rc = generate_action_id(in_path + file_name, std::dynamic_pointer_cast<mat_assembler>(p_asm))) {
        return rc;
    }

    file_name = p_asm->src_file_name() + ".ad";
    if (std::filesystem::exists(in_path + file_name)) {
        if (auto rc = output_normal_action_ids(otput_path, in_path + file_name)) {
            return rc;
        }
    } else {
        if (auto rc = output_normal_action_ids(otput_path)) {
            return rc;
        }
    }

    if (auto rc = output_default_action_ids(otput_path)) {
        return rc;
    }

    return 0;
}

inline std::string get_cluter_table_string(std::uint32_t cluster, std::uint32_t table) {
    auto strm = std::ostringstream();
    strm << "cluster" << cluster << "_table" << table;
    return strm.str();
}

int mat_link::output_normal_action_ids(const std::string &ot_path, const std::string &ad_path) {
    auto ad_ifstrm = std::ifstream();
    if (!ad_path.empty()) {
        ad_ifstrm.open(ad_path);
        if (!ad_ifstrm.is_open()) {
            std::cout << "ERROR: cannot open " << ad_path << " !!!" << std::endl;
            return -1;
        }
    }

    auto prev_cluster_table = 0xFFUL;
    for (const auto &tp : cluster_table_entry_2_ram_action) {
        auto ot_path1 = ot_path + "action_ids/";
        if (!std::filesystem::exists(ot_path1)) {
            if (!std::filesystem::create_directory(ot_path1)) {
                std::cout << "failed to create directory: " << ot_path1 << std::endl;
                return -1;
            }
        }

        auto ram_bitmap = tp.second.first;
        auto action_id = tp.second.second;
        std::vector<std::uint16_t> ad_aid{action_id, ram_bitmap, 0, 0, 0, 0, 0};

        if (ad_ifstrm.is_open()) {
            auto line = std::string();
            if (!std::getline(ad_ifstrm, line)) {
                std::cout << "ERROR: get line from " << ad_path << " !!!" << std::endl;
                return -1;
            }
            line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
            if (line.size() > 20) {
                std::cout << "ERROR: line is too long.\n\t" << line << std::endl;
                return -1;
            }
            if (line != "00000000000000000000") {
                for (auto &c : line) {
                    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                        std::cout << "ERROR: lines of ad file shall only include heximal digits.\n\t";
                        std::cout << line << std::endl;
                        return -1;
                    }
                }
                for (auto i = 0UL, j = ad_aid.size() - 1; i < line.size() && j >= 2; i += 4, --j) {
                    auto str = line.substr(i, 4);
                    if (str != "0000") {
                        ad_aid[j] = static_cast<std::uint16_t>(std::stoul(str, nullptr, 16));
                    }
                }
            }
        }

        auto cluster_id = std::get<0>(tp.first);
        auto table_id = std::get<1>(tp.first);
        ot_path1 += get_cluter_table_string(cluster_id, table_id);

        std::uint16_t cluster_table = cluster_id;
        cluster_table <<= 8;
        cluster_table |= table_id;

        auto ot_fstrm = std::ofstream();
        if (cluster_table != prev_cluster_table) {
            ot_fstrm.open(ot_path1 + ".txt", std::ios::trunc);
        } else {
            ot_fstrm.open(ot_path1 + ".txt", std::ios::app);
        }
        if (!ot_fstrm.is_open()) {
            std::cout << "cannot open file: " << ot_path1 + ".txt" << std::endl;
            return -1;
        }
        for (auto it = ad_aid.crbegin(); it != ad_aid.crend(); ++it) {
            ot_fstrm << std::bitset<16>(*it);
        }
        ot_fstrm << "\n";
        ot_fstrm.close();

        if (cluster_table != prev_cluster_table) {
            ot_fstrm.open(ot_path1 + ".dat", std::ios::trunc);
        } else {
            ot_fstrm.open(ot_path1 + ".dat", std::ios::app);
        }
        if (!ot_fstrm.is_open()) {
            std::cout << "cannot open file: " << ot_path1 + ".dat" << std::endl;
            return -1;
        }
        ot_fstrm.write(reinterpret_cast<const char*>(ad_aid.data()), sizeof(ad_aid[0]) * ad_aid.size());
        ot_fstrm.close();

        prev_cluster_table = cluster_table;
    }

    ad_ifstrm.close();

    return 0;
}

int mat_link::output_default_action_ids(const std::string &ot_path) {
    for (const auto &pp : cluster_table_2_ram_action) {
        auto ot_path1 = ot_path + "default_action_ids/";
        if (!std::filesystem::exists(ot_path1)) {
            if (!std::filesystem::create_directory(ot_path1)) {
                std::cout << "failed to create directory: " << ot_path1 << std::endl;
                return -1;
            }
        }

        ot_path1 += get_cluter_table_string(pp.first.first, pp.first.second);
        std::uint32_t aid_entry = pp.second.first;
        aid_entry <<= 16;
        aid_entry |= pp.second.second;

        auto ot_fstrm = std::ofstream(ot_path1 + ".txt");
        if (!ot_fstrm.is_open()) {
            std::cout << "cannot open file: " << ot_path1 + ".txt" << std::endl;
            return -1;
        }
        ot_fstrm << std::bitset<32>(aid_entry);
        ot_fstrm.close();

        ot_fstrm.open(ot_path1 + ".dat");
        if (!ot_fstrm.is_open()) {
            std::cout << "cannot open file: " << ot_path1 + ".dat" << std::endl;
            return -1;
        }
        ot_fstrm.write(reinterpret_cast<const char*>(&aid_entry), sizeof(aid_entry));
        ot_fstrm.close();
    }

    return 0;
}

int mat_link::generate_action_id(const std::string &in_path, const std::shared_ptr<mat_assembler> &p_asm) {
    auto in_fstrm = std::ifstream(in_path);
    auto line = std::string();
    auto file_line_idx = 1UL;
    std::vector<std::string> start_end_stk;
    auto r1 = std::regex(start_end_line_pattern);
    auto r2 = std::regex(cluster_table_entry_line_pattern);
    auto r3 = std::regex(nop_line_pattern);
    auto r4 = std::regex(comment_empty_line_p);

    while (std::getline(in_fstrm, line)) {
        auto m = std::smatch();
        if (std::regex_match(line, m, r1)) {
            if (m.str(1) == "start") {
                if (cur_line_idx != 0 || (!start_end_stk.empty() && start_end_stk.back() != "end")) {
                    std::cout << "line #" << file_line_idx << ": \n\t";
                    std::cout << "a '.start' line shall be the initial line or succeeding a '.end' line." << std::endl;
                    return -1;
                }
                start_end_stk.emplace_back("start");
                if (cur_ram_idx > 7) {
                    std::cout << "test" << std::endl;
                    return -1;
                }
            } else if (m.str(1) == "end") {
                cur_line_idx = 0;
                if (start_end_stk.empty() || start_end_stk.back() != "start") {
                    std::cout << "line #" << file_line_idx << ": \n\t";
                    std::cout << "a '.end' line shall be coupled with a '.start' line." << std::endl;
                    return -1;
                }
                start_end_stk.emplace_back("end");
                ++cur_ram_idx;
            }
        } else if (std::regex_match(line, r2)) {
            if (!(start_end_stk.size() % 2)) {
                std::cout << "line #" << file_line_idx << ": " << line << "\n\t";
                std::cout << "any table linking line shall be located between a '.start' and a '.end'" << std::endl;
                return -1;
            }

            auto line_strm = std::istringstream(line);
            auto sub_str = std::string();
            auto re = std::regex(cluster_table_entry_field_pattern);
            while (std::getline(line_strm, sub_str, ':')) {
                if (!std::regex_match(sub_str, m, re)) {
                    std::cout << "line #" << file_line_idx << ": " << sub_str << "\n\t";
                    std::cout << "does not match cluster_table_entry format." << std::endl;
                    return -1;
                }

                if (m.str(8).empty()) {
                    if (is_using_ad(p_asm)) {
                        std::cout << "line #" << file_line_idx << ": " << sub_str << "\n\t";
                        std::cout << "default action is not allowed to use AD." << std::endl;
                        return -1;
                    }
                    if (auto rc = process_default_action(m)) {
                        std::cout << "line #" << file_line_idx << ": " << sub_str << "\n\t";
                        std::cout << "error processing default action." << std::endl;
                        return rc;
                    }
                } else {
                    if (auto rc = process_normal_action(m)) {
                        std::cout << "line #" << file_line_idx << ": " << sub_str << "\n\t";
                        std::cout << "error processing normal action." << std::endl;
                        return rc;
                    }
                }
            }

            ++cur_line_idx;
        } else if (std::regex_match(line, r3)) {
            ++cur_line_idx;
        } else if (!std::regex_match(line, r4)) {
            std::cout << "line #" << file_line_idx << ": " << line << "\n\t";
            std::cout << "does not match." << std::endl;
            return -1;
        }

        ++file_line_idx;
    }

    return 0;
}

inline bool mat_link::is_using_ad(const std::shared_ptr<mat_assembler>& p_asm) {
    machine_code mcode;
    mcode.val64 = p_asm->ram_mcode_vec[cur_ram_idx][cur_line_idx];
    switch (mcode.op_00001.opcode) {
    case 0b00001:
    case 0b00010:
    case 0b10100:
    case 0b10111:
    case 0b11000:
        if (mcode.op_00001.mode == 1) {
            return true;
        }
        break;
    case 0b00100:
    case 0b00101:
    case 0b00110:
    case 0b00111:
        if (mcode.op_00001.mode == 2) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

int mat_link::process_normal_action(const std::smatch &m) {
    auto cluster_idx = std::stoul(m.str(1));
    auto table_idx = std::stoul(m.str(5));
    auto table_idx2 = 0UL;
    auto table_type = m.str(4).back();

    if (!m.str(6).empty()) {
        table_idx2 = std::stoul(m.str(7));
    }

    auto entries = get_table_entries(m);

    if (!m.str(2).empty()) {
        auto cluster_idx2 = std::stoul(m.str(3));
        if (auto rc = check_cluster_range(cluster_idx, cluster_idx2)) {
            return rc;
        }
        for (auto x = cluster_idx; x <= cluster_idx2; ++x) {
            if (!m.str(6).empty()) {
                if (auto rc = assign_multi_table_action(x, table_type, table_idx, table_idx2, entries)) {
                    return rc;
                }
            } else {
                if (auto rc = assign_single_table_action(x, table_type, table_idx, table_idx, entries)) {
                    return rc;
                }
            }
        }
    } else {
        if (!m.str(6).empty()) {
            return assign_multi_table_action(cluster_idx, table_type, table_idx, table_idx2, entries);
        } else {
            return assign_single_table_action(cluster_idx, table_type, table_idx, table_idx, entries);
        }
    }

    return 0;
}

std::vector<std::uint16_t> mat_link::get_table_entries(const std::smatch &m) {
    auto entry_start = std::stoul(m.str(9));

    if (m.str(10).empty()) {
        return std::vector<std::uint16_t>{static_cast<std::uint16_t>(entry_start)};
    }

    if (!m.str(11).empty()) {
        auto entry_end = std::stoul(m.str(12));
        if (entry_start >= entry_end) {
            return std::vector<std::uint16_t>();
        }
        std::vector<std::uint16_t> entries;
        for (auto i = entry_start; i <= entry_end; ++i) {
            entries.emplace_back(i);
        }
        return entries;
    }

    std::vector<std::uint16_t> entries;
    auto in_strm = std::istringstream(m.str(9) + m.str(13));
    auto str = std::string();
    while (std::getline(in_strm, str, ',')) {
        entries.emplace_back(std::stoul(str));
    }
    return entries;
}

int mat_link::assign_single_table_action(std::uint8_t cluster, char type, std::uint8_t base, std::uint8_t table,
    const std::vector<std::uint16_t> &entries) {
    auto cnt = type == 'T' ? tcam_table_entry_cnt : hash_table_entry_cnt;
    for (const auto &entry : entries) {
        auto idx = entry / cnt;
        if (table != base + idx) {
            continue;
        }
        auto cluster_table_entry = std::make_tuple(cluster, table, entry);
        std::uint16_t ram_bitmap = 1 << cur_ram_idx;
        if (cluster_table_entry_2_ram_action.count(cluster_table_entry)) {
            if (cur_line_idx != cluster_table_entry_2_ram_action[cluster_table_entry].second) {
                std::cout << "action id conflict." << std::endl;
                return -1;
            }
            ram_bitmap = cluster_table_entry_2_ram_action[cluster_table_entry].first;
            if (ram_bitmap & (1 << cur_ram_idx)) {
                std::cout << "action bitmap conflict." << std::endl;
                return -1;
            }
            ram_bitmap |= 1 << cur_ram_idx;
        }
        cluster_table_entry_2_ram_action[cluster_table_entry] = std::make_pair(ram_bitmap, cur_line_idx);
    }
    return 0;
}

int mat_link::assign_multi_table_action(std::uint8_t cluster, char type, std::uint8_t table, std::uint8_t table2,
    const std::vector<std::uint16_t> &entries) {
    if (table2 <= table) {
        std::cout << "table range shall be ascending." << std::endl;
        return -1;
    }
    for (auto y = table; y <= table2; ++y) {
        if (auto rc = assign_single_table_action(cluster, type, table, y, entries)) {
            return rc;
        }
    }
    return 0;
}

inline int mat_link::check_cluster_range(std::uint8_t cluster1, std::uint8_t cluster2) {
    if (cluster2 <= cluster1 || cluster2 - cluster1 >= 4) {
        std::cout << "cluster range shall be ascending but not exceeding 4." << std::endl;
        return -1;
    }
    return 0;
}

int mat_link::process_default_action(const std::smatch &m) {
    auto cluster_idx = std::stoul(m.str(1));
    auto table_idx = std::stoul(m.str(5));
    auto table_idx2 = 0UL;

    if (!m.str(6).empty()) {
        table_idx2 = std::stoul(m.str(7));
    }

    if (!m.str(2).empty()) {
        auto cluster_idx2 = std::stoul(m.str(3));
        if (auto rc = check_cluster_range(cluster_idx, cluster_idx2)) {
            return rc;
        }
        for (auto x = cluster_idx; x <= cluster_idx2; ++x) {
            if (!m.str(6).empty()) {
                if (auto rc = assign_multi_def_action(x, table_idx, table_idx2)) {
                    return rc;
                }
            } else {
                if (auto rc = assign_single_def_action(x, table_idx)) {
                    return rc;
                }
            }
        }
    } else {
        if (!m.str(6).empty()) {
            return assign_multi_def_action(cluster_idx, table_idx, table_idx2);
        } else {
            return assign_single_def_action(cluster_idx, table_idx);
        }
    }

    return 0;
}

int mat_link::assign_single_def_action(std::uint8_t cluster, std::uint8_t table) {
    auto cluster_table = std::make_pair(cluster, table);
    std::uint16_t ram_bitmap = 1 << cur_ram_idx;
    if (cluster_table_2_ram_action.count(cluster_table)) {
        if (cur_line_idx != cluster_table_2_ram_action[cluster_table].second) {
            std::cout << "action id conflict." << std::endl;
            return -1;
        }
        ram_bitmap = cluster_table_2_ram_action[cluster_table].first;
        if (ram_bitmap & (1 << cur_ram_idx)) {
            std::cout << "action bitmap conflict." << std::endl;
            return -1;
        }
        ram_bitmap |= 1 << cur_ram_idx;
    }
    cluster_table_2_ram_action[cluster_table] = std::make_pair(ram_bitmap, cur_line_idx);
    return 0;
}

inline int mat_link::assign_multi_def_action(std::uint8_t cluster, std::uint8_t table, std::uint8_t table2) {
    if (table2 <= table) {
        std::cout << "table range shall be ascending." << std::endl;
        return -1;
    }
    for (auto y = table; y <= table2; ++y) {
        if (auto rc = assign_single_def_action(cluster, y)) {
            return rc;
        }
    }
    return 0;
}

const char* mat_link::start_end_line_pattern = R"(^\.(start|end)(\s+\/\/.*)?[\n\r]?$)";
const char* mat_link::nop_line_pattern = R"(^\s*nop\s*(\s+\/\/.*)?[\n\r]?$)";
const char* mat_link::cluster_table_entry_line_pattern =
    R"(^\s*\d+(-\d+)?_[TH]\d+(-\d+)?(_\d+((-\d+)|(,\d+)+)?)?)"
    R"((\s*:\s*\d+(-\d+)?_[TH]\d+(-\d+)?(_\d+((-\d+)|(,\d+)+)?)?)*\s*$)";
const char* mat_link::cluster_table_entry_field_pattern =
    R"(^\s*([0-9]|1[0-5])(-([0-9]|1[0-5]))?_([TH])([0-9])(-([0-9]))?(_(\d{1,5})((-(\d{1,5}))|(,\d{1,5})+)?)?\s*$)";
