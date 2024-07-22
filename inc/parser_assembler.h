/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_PARSER_ASSEMBLER_H_
#define INC_PARSER_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <map>
#include "assembler.h"  // NOLINT [build/include_subdir]

using str_u64_map = std::unordered_map<string, std::uint64_t>;
using u64_regex_map = std::unordered_map<std::uint64_t, regex>;
using vec_of_u16 = std::vector<std::uint16_t>;
using map_of_u16 = std::map<std::uint16_t, std::uint16_t>;
using unorderedmap_of_u16_map = std::unordered_map<std::uint16_t, map_of_u16>;

constexpr std::uint16_t MAX_STATE_NO = 255;
constexpr std::uint16_t MAX_LINE_NO = 256 * 32 - 1;

class parser_assembler : public assembler {
 public:
    parser_assembler() = default;
    virtual ~parser_assembler() = default;

    parser_assembler(const parser_assembler&) = delete;
    parser_assembler(parser_assembler&&) = delete;
    parser_assembler& operator=(const parser_assembler&) = delete;
    parser_assembler& operator=(parser_assembler&&) = delete;

 private:
    string get_name_pattern(void) const override {
        return cmd_name_pattern;
    }
    string get_name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;
    int output_entry_code(const string &) override;
    string get_state_no_pattern(void) const override {
        return state_no_pattern;
    }

    int process_state_no_line(const string&, const string&);

 private:
    vector<std::uint64_t> mcode_vec;
    std::uint64_t entry_code{0};
    std::uint16_t init_state;
    vec_of_u16 states_seq;
    unorderedmap_of_u16_map state_line_sub_map;
    bool pre_last_flag{false};

    static const char* cmd_name_pattern;
    static const char* state_no_pattern;
    static const int l_idx;   // index of L sub-pattern for Last flag
    static const int u_idx;   // index of U sub-pattern for Unsigned flag
    static const int m0_idx;  // index of M0 sub-pattern for Mask to 0 flag
    static const str_u64_map cmd_opcode_map;
    static const u64_regex_map opcode_regex_map;
};

#endif  // INC_PARSER_ASSEMBLER_H_
