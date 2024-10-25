/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <filesystem>
#include <iostream>
#include "../../inc/mat_assembler.h"
#include "../../inc/mat_def.h"
#include "../../inc/table_proc/mat_link.h"
#include "../../inc/global_def.h"

constexpr auto TCAM_ENTRY_CNT = 512;
constexpr auto HASH_ENTRY_CNT = 1024;
constexpr auto CACHE_ENTRY_CNT = 2048;
constexpr auto CACHE_ENTRY_LEN = 1024;  // in bit
constexpr auto CACHE_AD_LEN = 320;  // in bit
constexpr auto H_MERGE_LMT = 4;  // horizontal merging limit
constexpr auto V_MERGE_LMT = 10;  // vertical merging limit
constexpr auto DEFAULT_ACTION_DIR = "default_action_ids/";
constexpr auto NORMAL_ACTION_DIR = "action_ids/";
constexpr auto TABLE_MAT_DIR = "tables_mat/";

typedef union {
    struct {
        std::uint32_t entry : 22;
        std::uint32_t table : 5;
        std::uint32_t cluster : 5;
    };
    std::uint32_t val;
} cache_entry_tuple;

inline std::uint16_t entry_cnt_per_table(char type) {
    return type == 'T' ? TCAM_ENTRY_CNT :  HASH_ENTRY_CNT;
}

inline bool cluster_range_not_valid(std::uint8_t cluster1, std::uint8_t cluster2) {
    return (cluster2 <= cluster1 || cluster2 - cluster1 >= H_MERGE_LMT);
}

inline bool table_range_not_valid(std::uint8_t table1, std::uint8_t table2) {
    return (table2 <= table1 || table2 - table1 >= V_MERGE_LMT);
}

template <typename T>
std::vector<T>& discrete_entries(const std::string &discete_str, std::vector<T> &entry_vec,  // NOLINT
    std::uint8_t table_cnt = 1, std::uint32_t entry_cnt = CACHE_ENTRY_CNT) {
    auto str = std::string();
    auto in_strm = std::istringstream(discete_str);
    while (std::getline(in_strm, str, ',')) {
        auto entry_id = std::stoul(str);
        if (entry_id / entry_cnt >= table_cnt) {
            std::cout << "table entry index exceeds limit." << std::endl;
            entry_vec.clear();
            return entry_vec;
        }
        entry_vec.emplace_back(entry_id);
    }
    return entry_vec;
}

std::vector<std::uint16_t> get_table_entries(const std::smatch &m, char table_type, std::uint8_t table_cnt) {
    auto entry_start = std::stoul(m.str(9));

    if (m.str(10).empty()) {
        return std::vector<std::uint16_t>{static_cast<std::uint16_t>(entry_start)};
    }

    auto cnt = entry_cnt_per_table(table_type);
    auto entries = std::vector<std::uint16_t>();

    if (!m.str(11).empty()) {
        auto entry_end = std::stoul(m.str(12));
        if (entry_start >= entry_end) {
            return std::vector<std::uint16_t>();
        }
        for (auto i = entry_start; i <= entry_end; ++i) {
            if (i / cnt >= table_cnt) {
                std::cout << "table entry index exceeds limit." << std::endl;
                return std::vector<std::uint16_t>();
            }
            entries.emplace_back(i);
        }
        return entries;
    }

    return discrete_entries(m.str(9) + m.str(13), entries, table_cnt, cnt);
}

int mat_link::generate_table_data(const std::shared_ptr<assembler> &p_asm) {
    auto in_path = input_path + TABLE_MAT_DIR;
    #if !NO_PRE_PROC
    in_path = input_path + "../" + TABLE_MAT_DIR;
    #endif
    if (!std::filesystem::exists(in_path)) {
        std::cout << in_path << " dose not exist." << std::endl;
        return -1;
    }

    auto file_name = p_asm->src_file_stem() + ".link";
    if (!std::filesystem::exists(in_path + file_name)) {
        std::cout << file_name << " does not exists in path: " << in_path << std::endl;
        return -1;
    }

    if (auto rc = generate_action_id(in_path + file_name, std::dynamic_pointer_cast<mat_assembler>(p_asm))) {
        return rc;
    }

    file_name = p_asm->src_file_stem() + ".ad";
    if (std::filesystem::exists(in_path + file_name)) {
        if (auto rc = output_normal_action_ids(in_path + file_name)) {
            return rc;
        }
    } else {
        if (auto rc = output_normal_action_ids()) {
            return rc;
        }
    }

    if (auto rc = output_default_action_ids()) {
        return rc;
    }

    if (auto rc = output_cache_default_action_ids()) {
        return rc;
    }

    if (auto rc = output_cache_normal_action_ids()) {
        return rc;
    }

    return 0;
}

std::string mat_link::get_cluter_table_string(std::uint32_t cluster, std::uint32_t table, bool is_def) {
    auto strm = std::ostringstream();
    auto key = std::string();
    if (linear_cluster_set.count(cluster) && !is_def) {
        if (table % 2) {
            table -= 1;
        } else {
            key = "_key_part";
        }
    }
    strm << "cluster" << cluster << "_table" << table << key;
    return strm.str();
}

int get_associate_data(const std::string &ad_path, std::ifstream &ad_ifstrm, std::vector<std::uint16_t> &data) {  // NOLINT
    auto line = std::string();
    if (!std::getline(ad_ifstrm, line)) {
        std::cout << "ERROR: get line from " << ad_path << " !!!" << std::endl;
        return -1;
    }
    if (trim_all(line).size() > 20) {
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
        for (auto i = 0UL, j = data.size() - 1; i < line.size() && j >= 2; i += 4, --j) {
            auto str = line.substr(i, 4);
            if (str != "0000") {
                data[j] = static_cast<std::uint16_t>(std::stoul(str, nullptr, 16));
            }
        }
    }
    return 0;
}

int mat_link::output_single_data_entry(std::uint8_t cluster_id, std::uint8_t table_id,
    std::string ot_path, std::uint16_t &prev_cluster_table, std::vector<std::uint16_t> &data) {  // NOLINT
    std::uint16_t cluster_table = cluster_id;
    cluster_table <<= 8;
    cluster_table |= table_id;

    auto ot_fstrm = std::ofstream();
    if (cluster_table != prev_cluster_table) {
        ot_fstrm.open(ot_path + ".txt", std::ios::trunc);
    } else {
        ot_fstrm.open(ot_path + ".txt", std::ios::app);
    }
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << ot_path + ".txt" << std::endl;
        return -1;
    }
    if (linear_cluster_set.count(cluster_id) && !(table_id % 2)) {
        ot_fstrm << std::bitset<48>(0);
    }
    for (auto it = data.crbegin(); it != data.crend(); ++it) {
        ot_fstrm << std::bitset<16>(*it);
    }
    ot_fstrm << "\n";
    ot_fstrm.close();

    if (cluster_table != prev_cluster_table) {
        ot_fstrm.open(ot_path + ".dat", std::ios::trunc);
    } else {
        ot_fstrm.open(ot_path + ".dat", std::ios::app);
    }
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << ot_path + ".dat" << std::endl;
        return -1;
    }
    if (linear_cluster_set.count(cluster_id) && !(table_id % 2)) {
        data.resize(data.size() + 3);
    }
    ot_fstrm.write(reinterpret_cast<const char*>(data.data()), sizeof(data[0]) * data.size());
    ot_fstrm.close();

    prev_cluster_table = cluster_table;

    return 0;
}

int mat_link::output_normal_action_ids(const std::string &ad_path) {
    auto ad_ifstrm = std::ifstream();
    if (!ad_path.empty()) {
        ad_ifstrm.open(ad_path);
        if (!ad_ifstrm.is_open()) {
            std::cout << "ERROR: cannot open " << ad_path << " !!!" << std::endl;
            return -1;
        }
    }

    std::uint16_t prev_cluster_table = 0xFFUL;
    for (const auto &tp : cluster_table_entry_2_ram_action) {
        auto ot_path = otput_path + NORMAL_ACTION_DIR;
        if (!std::filesystem::exists(ot_path)) {
            if (!std::filesystem::create_directory(ot_path)) {
                std::cout << "failed to create directory: " << ot_path << std::endl;
                return -1;
            }
        }

        auto ram_bitmap = tp.second.first;
        auto action_id = tp.second.second;
        std::vector<std::uint16_t> ad_aid{action_id, ram_bitmap, 0, 0, 0, 0, 0};

        if (ad_ifstrm.is_open()) {
            if (auto rc = get_associate_data(ad_path, ad_ifstrm, ad_aid)) {
                return rc;
            }
        }

        auto cluster_id = std::get<0>(tp.first);
        auto table_id = std::get<1>(tp.first);
        ot_path += get_cluter_table_string(cluster_id, table_id, false);

        if (auto rc = output_single_data_entry(cluster_id, table_id, ot_path, prev_cluster_table, ad_aid)) {
            return rc;
        }
    }

    ad_ifstrm.close();

    return 0;
}

int output_2_file(std::uint32_t aid_entry, const std::string &path) {
    auto ot_fstrm = std::ofstream(path + ".txt");
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << path + ".txt" << std::endl;
        return -1;
    }
    ot_fstrm << std::bitset<32>(aid_entry);
    ot_fstrm.close();

    ot_fstrm.open(path + ".dat");
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << path + ".dat" << std::endl;
        return -1;
    }
    ot_fstrm.write(reinterpret_cast<const char*>(&aid_entry), sizeof(aid_entry));
    ot_fstrm.close();
    return 0;
}

int mat_link::output_default_action_ids() {
    for (const auto &pp : cluster_table_2_ram_action) {
        auto ot_path = otput_path + DEFAULT_ACTION_DIR;
        if (!std::filesystem::exists(ot_path)) {
            if (!std::filesystem::create_directory(ot_path)) {
                std::cout << "failed to create directory: " << ot_path << std::endl;
                return -1;
            }
        }

        ot_path += get_cluter_table_string(pp.first.first, pp.first.second);
        std::uint32_t aid_entry = pp.second.first;
        aid_entry <<= 16;
        aid_entry |= pp.second.second;

        if (auto rc = output_2_file(aid_entry, ot_path)) {
            return rc;
        }
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
    auto r4 = std::regex(COMMENT_EMPTY_LINE_P);

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
                if (m.str(4).empty()) {
                    if (auto rc = process_cache_table(m)) {
                        std::cout << "line #" << file_line_idx << ": " << sub_str << "\n\t";
                        std::cout << "error processing cache table." << std::endl;
                        return rc;
                    }
                } else if (m.str(8).empty()) {
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
                    if ((m.str(9) == "C" || m.str(9) == "S")) {
                        std::cout << "line #" << file_line_idx << ": " << sub_str << "\n\t";
                        std::cout << "non-cache entry shall not use 'C' or 'S' in pattern." << std::endl;
                        return -1;
                    }
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

bool mat_link::is_using_ad(const std::shared_ptr<mat_assembler>& p_asm) {
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
    auto table_cnt = 1UL;

    if (!m.str(6).empty()) {
        table_idx2 = std::stoul(m.str(7));
        if (table_range_not_valid(table_idx, table_idx2)) {
            std::cout << "table range shall be ascending." << std::endl;
            return -1;
        }
        table_cnt = table_idx2 - table_idx + 1;
    }

    auto entries = get_table_entries(m, table_type, table_cnt);
    if (entries.empty()) {
        return -1;
    }

    if (!m.str(2).empty()) {
        auto cluster_idx2 = std::stoul(m.str(3));
        if (cluster_range_not_valid(cluster_idx, cluster_idx2)) {
            std::cout << "cluster range shall be ascending but not exceeding 4." << std::endl;
            return -1;
        }
        for (auto x = cluster_idx; x <= cluster_idx2; ++x) {
            insert_linear_cluster_set(table_type, x);
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
        insert_linear_cluster_set(table_type, cluster_idx);
        if (!m.str(6).empty()) {
            return assign_multi_table_action(cluster_idx, table_type, table_idx, table_idx2, entries);
        } else {
            return assign_single_table_action(cluster_idx, table_type, table_idx, table_idx, entries);
        }
    }

    return 0;
}

int mat_link::assign_single_table_action(std::uint8_t cluster, char type, std::uint8_t base, std::uint8_t table,
    const std::vector<std::uint16_t> &entries) {
    auto cnt = entry_cnt_per_table(type);
    for (const auto &entry : entries) {
        if (table != base + entry / cnt) {
            continue;
        }
        if (auto rc = assign_single_entry_action(cluster, table, entry)) {
            return rc;
        }
    }
    return 0;
}

int mat_link::assign_multi_table_action(std::uint8_t cluster, char type, std::uint8_t table, std::uint8_t table2,
    const std::vector<std::uint16_t> &entries) {
    for (auto y = table; y <= table2; ++y) {
        if (auto rc = assign_single_table_action(cluster, type, table, y, entries)) {
            return rc;
        }
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
        if (cluster_range_not_valid(cluster_idx, cluster_idx2)) {
            std::cout << "cluster range shall be ascending but not exceeding 4." << std::endl;
            return -1;
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
        if (ram_bitmap & cluster_table_2_ram_action[cluster_table].first) {
            std::cout << "action bitmap conflict." << std::endl;
            return -1;
        }
        ram_bitmap |= cluster_table_2_ram_action[cluster_table].first;
    }
    cluster_table_2_ram_action[cluster_table] = std::make_pair(ram_bitmap, cur_line_idx);
    return 0;
}

int mat_link::assign_multi_def_action(std::uint8_t cluster, std::uint8_t table, std::uint8_t table2) {
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

int mat_link::process_cache_table(const std::smatch &m) {
    if (!m.str(2).empty() || !m.str(6).empty() || m.str(8).empty()) {
        std::cout << "ERROR: no merge allowed for a cache table!" << std::endl;
        return -1;
    }
    if ((m.str(9) == "C" || m.str(9) == "S")) {
        if (!m.str(10).empty()) {
            std::cout << "ERROR: cache table default action id pattern!" << std::endl;
            return -1;
        }
        return assign_single_def_action(std::stoul(m.str(1)), std::stoul(m.str(5)), m.str(9));
    } else {
        return assign_cache_normal_action(m);
    }
}

int mat_link::assign_cache_normal_action(const std::smatch &m) {
    cache_entry_tuple cache_entry;
    cache_entry.cluster = std::stoul(m.str(1));
    cache_entry.table = std::stoul(m.str(5));
    if (m.str(10).empty()) {
        cache_entry.entry = std::stoul(m.str(9));
        return assign_single_entry_action(cache_entry.val);
    } else if (!m.str(11).empty()) {
        auto start_id = std::stoul(m.str(9));
        auto end_id = std::stoul(m.str(12));
        if (start_id >= end_id) {
            std::cout << "entries shall be ascending." << std::endl;
            return -1;
        }
        for (auto i = start_id; i <= end_id; ++i) {
            cache_entry.entry = i;
            if (auto rc = assign_single_entry_action(cache_entry.val)) {
                return rc;
            }
        }
    } else {
        auto entries = std::vector<std::uint32_t>();
        entries = discrete_entries(m.str(9) + m.str(13), entries);
        for (auto entry : entries) {
            cache_entry.entry = entry;
            if (auto rc = assign_single_entry_action(cache_entry.val)) {
                return rc;
            }
        }
    }
    return 0;
}

int mat_link::output_cache_normal_action_ids() {
    cache_entry_tuple cache_entry;
    std::uint16_t prev_cluster_table = 0xFFUL;
    for (const auto &tp : cache_entry_2_ram_action) {
        auto ot_path = otput_path + NORMAL_ACTION_DIR;
        if (!std::filesystem::exists(ot_path)) {
            if (!std::filesystem::create_directory(ot_path)) {
                std::cout << "failed to create directory: " << ot_path << std::endl;
                return -1;
            }
        }

        std::vector<std::uint16_t> data_entry(CACHE_ENTRY_LEN / 16);
        auto ram_bitmap_idx = data_entry.size() - 1 - CACHE_AD_LEN / 16;
        auto action_id_idx = ram_bitmap_idx - 1;
        data_entry[ram_bitmap_idx] = tp.second.first;
        data_entry[action_id_idx] = tp.second.second;

        cache_entry.val = tp.first;
        ot_path += get_cluter_table_string(cache_entry.cluster, cache_entry.table);

        if (auto rc = output_single_data_entry(
            cache_entry.cluster, cache_entry.table, ot_path, prev_cluster_table, data_entry)) {
            return rc;
        }
    }
    return 0;
}

int mat_link::assign_single_def_action(std::uint8_t cluster, std::uint8_t table, std::string type) {
    auto cluster_table_type = std::make_tuple(cluster, table, type.back());
    std::uint16_t ram_bitmap = 1 << cur_ram_idx;
    if (cluster_table_type_2_ram_action.count(cluster_table_type)) {
        if (cur_line_idx != cluster_table_type_2_ram_action[cluster_table_type].second) {
            std::cout << "action id conflict." << std::endl;
            return -1;
        }
        if (ram_bitmap & cluster_table_type_2_ram_action[cluster_table_type].first) {
            std::cout << "action bitmap conflict." << std::endl;
            return -1;
        }
        ram_bitmap |= cluster_table_type_2_ram_action[cluster_table_type].first;
    }
    cluster_table_type_2_ram_action[cluster_table_type] = std::make_pair(ram_bitmap, cur_line_idx);
    return 0;
}

int mat_link::output_cache_default_action_ids() {
    for (const auto &tp : cluster_table_type_2_ram_action) {
        auto ot_path1 = otput_path + DEFAULT_ACTION_DIR;
        if (!std::filesystem::exists(ot_path1)) {
            if (!std::filesystem::create_directory(ot_path1)) {
                std::cout << "failed to create directory: " << ot_path1 << std::endl;
                return -1;
            }
        }

        ot_path1 += get_cluter_table_string(std::get<0>(tp.first), std::get<1>(tp.first));
        ot_path1 += std::get<2>(tp.first) == 'C' ? "_cache_miss" : "_search_miss";

        std::uint32_t aid_entry = tp.second.first;
        aid_entry <<= 16;
        aid_entry |= tp.second.second;

        if (auto rc = output_2_file(aid_entry, ot_path1)) {
            return rc;
        }
    }

    return 0;
}

int mat_link::assign_single_entry_action(std::uint8_t cluster, std::uint8_t table, std::uint16_t entry) {
    auto cluster_table_entry = std::make_tuple(cluster, table, entry);
    std::uint16_t ram_bitmap = 1 << cur_ram_idx;
    if (cluster_table_entry_2_ram_action.count(cluster_table_entry)) {
        if (cur_line_idx != cluster_table_entry_2_ram_action[cluster_table_entry].second) {
            std::cout << "action id conflict." << std::endl;
            return -1;
        }
        if (ram_bitmap & cluster_table_entry_2_ram_action[cluster_table_entry].first) {
            std::cout << "action bitmap conflict." << std::endl;
            return -1;
        }
        ram_bitmap |= cluster_table_entry_2_ram_action[cluster_table_entry].first;
    }
    cluster_table_entry_2_ram_action[cluster_table_entry] = std::make_pair(ram_bitmap, cur_line_idx);
    return 0;
}

int mat_link::assign_single_entry_action(std::uint32_t cache_entry) {
    cache_entry_tuple cache_tuple;
    cache_tuple.val = cache_entry;
    if (cache_tuple.entry >= CACHE_ENTRY_CNT) {
        std::cout << "table entry index exceeds limit." << std::endl;
        return -1;
    }
    std::uint16_t ram_bitmap = 1 << cur_ram_idx;
    if (cache_entry_2_ram_action.count(cache_entry)) {
        if (cur_line_idx != cache_entry_2_ram_action[cache_entry].second) {
            std::cout << "action id conflict." << std::endl;
            return -1;
        }
        if (ram_bitmap & cache_entry_2_ram_action[cache_entry].first) {
            std::cout << "action bitmap conflict." << std::endl;
            return -1;
        }
        ram_bitmap |= cache_entry_2_ram_action[cache_entry].first;
    }
    cache_entry_2_ram_action[cache_entry] = std::make_pair(ram_bitmap, cur_line_idx);
    return 0;
}

const char* mat_link::start_end_line_pattern = R"(^\.(start|end)(\s+\/\/.*)?[\n\r]?$)";
const char* mat_link::nop_line_pattern = R"(^\s*nop\s*(\s+\/\/.*)?[\n\r]?$)";
const char* mat_link::cluster_table_entry_line_pattern =
    R"(^\s*\d+(-\d+)?_[THL]?\d(-\d)?(_([CS]|\d+)((-\d+)|(,\d+)+)?)?)"
    R"((\s*:\s*\d+(-\d+)?_[THL]?\d(-\d)?(_([CS]|\d+)((-\d+)|(,\d+)+)?)?)*\s*$)";
const char* mat_link::cluster_table_entry_field_pattern =
    R"(^\s*([0-9]|1[0-5])(-([0-9]|1[0-5]))?_([THL]?)(\d)(-(\d))?(_([CS]|\d{1,5})((-(\d{1,5}))|(,\d{1,5})+)?)?\s*$)";
