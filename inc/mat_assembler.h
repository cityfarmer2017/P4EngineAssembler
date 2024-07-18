/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_MAT_ASSEMBLER_H_
#define INC_MAT_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include "assembler.h"  // NOLINT [build/include_subdir]

using str_u64_map = std::unordered_map<string, u64>;
using u64_regex_map = std::unordered_map<u64, regex>;

class mat_assembler : public assembler {
 public:
    mat_assembler() = default;
    virtual ~mat_assembler() = default;

    mat_assembler(const mat_assembler&) = delete;
    mat_assembler(mat_assembler&&) = delete;
    mat_assembler& operator=(const mat_assembler&) = delete;
    mat_assembler& operator=(mat_assembler&&) = delete;

 protected:
    inline string get_name_pattern(void) const override;
    string get_name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;

 private:
    std::vector<u64> mcode_vec;

    static const string cmd_name_pattern;
    static const int l_flg_idx;
    static const int crc_flg_idx;
    static const int xor_flg_idx;
    static const int sm_flg_idx;
    static const str_u64_map cmd_opcode_map;
    static const u64_regex_map opcode_regex_map;
};

#endif  // INC_MAT_ASSEMBLER_H_
