/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_TABLE_PROC_MAT_LINK_H_
#define INC_TABLE_PROC_MAT_LINK_H_

#include <string>
#include <memory>
#include <tuple>
#include <utility>
#include <map>
#include <cstdint>
#include <vector>
#include "table.h"  // NOLINT

using cluster_table_entry = std::tuple<std::uint8_t, std::uint8_t, std::uint16_t>;
using cluster_table = std::pair<std::uint8_t, std::uint8_t>;
using ram_line = std::pair<std::uint16_t, std::uint16_t>;
using map_of_cluster_table_entry_2_action = std::map<cluster_table_entry, ram_line>;
using map_of_cluster_table_2_action = std::map<cluster_table, ram_line>;

class mat_link : public table {
 public:
    mat_link(const std::string &in, const std::string &ot) : table(in, ot) {}
    mat_link() = default;
    ~mat_link() = default;

    mat_link(const mat_link&) = delete;
    mat_link(mat_link&&) = delete;
    mat_link& operator=(const mat_link&) = delete;
    mat_link& operator=(mat_link&&) = delete;

    int generate_table_data(const std::shared_ptr<assembler>&) override;

 private:
    int generate_action_id(const std::string&, const std::shared_ptr<mat_assembler>&);
    int output_normal_action_ids(const std::string&);
    int output_default_action_ids(const std::string&);
    int process_normal_action(const std::smatch&);
    std::vector<std::uint16_t> get_table_entries(const std::smatch&);
    int assign_single_table_action(std::uint8_t, char, std::uint8_t, std::uint8_t, const std::vector<std::uint16_t>&);
    int assign_multi_table_action(std::uint8_t, char, std::uint8_t, std::uint8_t, const std::vector<std::uint16_t>&);
    int process_default_action(const std::smatch&);
    int assign_single_def_action(std::uint8_t, std::uint8_t);
    inline int assign_multi_def_action(std::uint8_t, std::uint8_t, std::uint8_t);
    inline int check_cluster_range(std::uint8_t, std::uint8_t);
    inline bool is_using_ad(const std::shared_ptr<mat_assembler>&);

    std::uint16_t cur_line_idx{0};
    std::uint8_t cur_ram_idx{0};
    map_of_cluster_table_entry_2_action cluster_table_entry_2_ram_action;
    map_of_cluster_table_2_action cluster_table_2_ram_action;

    static const char* start_end_line_pattern;
    static const char* nop_line_pattern;
    static const char* cluster_table_entry_line_pattern;
    static const char* cluster_table_entry_field_pattern;
};

#endif  // INC_TABLE_PROC_MAT_LINK_H_
