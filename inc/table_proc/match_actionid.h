/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_TABLE_PROC_MATCH_ACTIONID_H_
#define INC_TABLE_PROC_MATCH_ACTIONID_H_

#include <set>
#include <vector>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <memory>
#include "table.h"  // NOLINT [build/include_subdir]

using vec_of_str = std::vector<std::string>;

constexpr auto ALL_1_STR32{"11111111111111111111111111111111"};

class match_actionid : public table {
 public:
    match_actionid(const std::string &in, const std::string &ot) :
        table(in, ot), actionid_sram_vec(MAX_TCAM_ENTRY_CNT + 1, 0) {}
    match_actionid() : actionid_sram_vec(MAX_TCAM_ENTRY_CNT + 1, 0) {}

    match_actionid(const match_actionid&) = delete;
    match_actionid(match_actionid&&) = delete;
    match_actionid& operator=(const match_actionid&) = delete;
    match_actionid& operator=(match_actionid&&) = delete;

    ~match_actionid() = default;

    int generate_table_data(const std::shared_ptr<assembler>&) override;

 private:
    int generate_sram_data(const std::string&, const map_of_u16_map&);
    int output_sram_data(const std::string&);

    std::set<std::filesystem::path> tcam_file_paths;
    std::vector<vec_of_str> match_sram_chip_vec{TCAM_CHIP_CNT, vec_of_str(ENTRY_CNT_PER_CHIP, ALL_1_STR32)};
    std::vector<std::uint16_t> actionid_sram_vec;

    static const std::unordered_map<std::string, std::string> c2_c4_map;
};

#endif  // INC_TABLE_PROC_MATCH_ACTIONID_H_
