/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_MAT_ASSEMBLER_H_
#define INC_MAT_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include "assembler.h"  // NOLINT [build/include_subdir]

using str_u64_map = std::unordered_map<string, std::uint64_t>;
using u64_regex_map = std::unordered_map<std::uint64_t, regex>;

class mat_assembler : public assembler {
 public:
    explicit mat_assembler(std::unique_ptr<table> tb) : assembler(std::move(tb)) {}
    mat_assembler() = default;
    explicit mat_assembler(bool flag) : long_flag(flag) {}
    virtual ~mat_assembler() = default;

    mat_assembler(const mat_assembler&) = delete;
    mat_assembler(mat_assembler&&) = delete;
    mat_assembler& operator=(const mat_assembler&) = delete;
    mat_assembler& operator=(mat_assembler&&) = delete;

 private:
    string get_name_pattern(void) const override {
        return cmd_name_pattern;
    }
    string get_name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;

 private:
    std::vector<std::uint64_t> mcode_vec;
    bool long_flag{false};

    static const char* cmd_name_pattern;
    static const int l_flg_idx;
    static const int crc_flg_idx;
    static const int xor_flg_idx;
    static const int sm_flg_idx;
    static const str_u64_map cmd_opcode_map;
    static const u64_regex_map opcode_regex_map;
};

#endif  // INC_MAT_ASSEMBLER_H_
