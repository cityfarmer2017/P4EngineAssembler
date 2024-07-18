/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_PARSER_ASSEMBLER_H_
#define INC_PARSER_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include "assembler.h"  // NOLINT [build/include_subdir]

using str_u64_map = std::unordered_map<string, u64>;
using u64_regex_map = std::unordered_map<u64, regex>;

class parser_assembler : public assembler {
 public:
    parser_assembler() = default;
    virtual ~parser_assembler() = default;

    parser_assembler(const parser_assembler&) = delete;
    parser_assembler(parser_assembler&&) = delete;
    parser_assembler& operator=(const parser_assembler&) = delete;
    parser_assembler& operator=(parser_assembler&&) = delete;

 protected:
    inline string get_name_pattern(void) const override;
    string get_name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;

 private:
    vector<u64> mcode_vec;

    static const string cmd_name_pattern;
    static const int l_idx;   // index of L sub-pattern for Last flag
    static const int u_idx;   // index of U sub-pattern for Unsigned flag
    static const int m0_idx;  // index of M0 sub-pattern for Mask to 0 flag
    static const str_u64_map cmd_opcode_map;
    static const u64_regex_map opcode_regex_map;
};

#endif  // INC_PARSER_ASSEMBLER_H_
