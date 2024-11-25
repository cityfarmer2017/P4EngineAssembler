/**
 * Copyright [2024] <wangdc1111@gmail.com>
 */
#ifndef INC_PARSER_ASSEMBLER_H_
#define INC_PARSER_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <map>
#include <filesystem>
#include <memory>
#include <utility>
#include "assembler.h"  // NOLINT [build/include_subdir]

using str_u64_map = std::unordered_map<string, std::uint64_t>;
using u64_regex_map = std::unordered_map<std::uint64_t, regex>;
using vec_of_u16 = std::vector<std::uint16_t>;
using map_of_u16 = std::map<std::uint16_t, std::uint16_t>;
using map_of_u16_map = std::map<std::uint16_t, map_of_u16>;

constexpr std::uint16_t MAX_STATE_NO = 31;
constexpr std::uint16_t MATCH_ENTRY_CNT_PER_STATE = 32;
constexpr std::uint16_t MAX_TCAM_ENTRY_CNT = (MAX_STATE_NO + 1) * MATCH_ENTRY_CNT_PER_STATE;
constexpr std::uint16_t MAX_LINE_NO = 16 * 1024 - 1;
constexpr std::uint16_t TCAM_CHIP_CNT = 20;
constexpr std::uint16_t ENTRY_CNT_PER_CHIP = (MAX_STATE_NO + 1) * 4;

class parser_assembler : public assembler {
    friend class match_actionid;

 public:
    #if !NO_TBL_PROC && !NO_PRE_PROC
    parser_assembler(std::unique_ptr<table> tbl, const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : assembler(std::move(tbl), pre, fname) {}
    #elif !NO_TBL_PROC
    explicit parser_assembler(std::unique_ptr<table> tbl, const std::string &fname)
        : assembler(std::move(tbl), fname) {}
    #elif !NO_PRE_PROC
    explicit parser_assembler(const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : assembler(pre, fname) {}
    #else
    explicit parser_assembler(const std::string &fname) : assembler(fname) {}
    #endif
    parser_assembler() = delete;
    virtual ~parser_assembler() = default;

    parser_assembler(const parser_assembler&) = delete;
    parser_assembler(parser_assembler&&) = delete;
    parser_assembler& operator=(const parser_assembler&) = delete;
    parser_assembler& operator=(parser_assembler&&) = delete;

 private:
    string name_pattern(void) const override {
        return cmd_name_pattern;
    }
    string name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;
    int process_extra_data(const string &, const string &) override;
    string assist_line_pattern(void) const override {
        return stateno_pattern;
    }
    int check_state_chart_valid() override;

    int process_state_no_line(const string&, const string&);
    int output_entry_code(const string&);

    #ifdef DEBUG
    void print_extra_data(void) {
        std::cout << "initial state: " << init_state << "\n";
        for (const auto &s : states_seq) {
            std::cout << s << ": ";
            for (const auto &line_sub : state_line_sub_map.at(s)) {
                std::cout << "<" << line_sub.first << ", " << line_sub.second << "> ";
            }
            std::cout << "\n";
        }
    }
    #endif

 private:
    vector<std::uint64_t> mcode_vec;
    std::uint64_t entry_code{0};
    std::uint16_t cur_line_idx{0};
    std::uint16_t init_state;
    vec_of_u16 states_seq;
    map_of_u16_map state_line_sub_map;
    bool pre_last_flag{false};

    static const char* cmd_name_pattern;
    static const char* stateno_pattern;
    static const int l_idx;   // index of L sub-pattern for Last flag
    static const int nf_idx;  // index of NF sub-pattern for not_final flag
    static const int u_idx;   // index of U sub-pattern for Unsigned flag
    static const int m0_idx;  // index of M0 sub-pattern for Mask to 0 flag
    static const int rvs_idx;
    static const str_u64_map cmd_opcode_map;
    static const u64_regex_map opcode_regex_map;
};

#endif  // INC_PARSER_ASSEMBLER_H_
