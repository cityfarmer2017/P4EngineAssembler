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
    friend class mat_link;

 public:
    #if !NO_TBL_PROC && !NO_PRE_PROC
    mat_assembler(std::unique_ptr<table> tbl, const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : assembler(std::move(tbl), pre, fname) {}
    #elif !NO_TBL_PROC
    explicit mat_assembler(std::unique_ptr<table> tbl, const std::string &fname)
        : assembler(std::move(tbl), fname) {}
    #elif !NO_PRE_PROC
    explicit mat_assembler(const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : assembler(pre, fname) {}
    #else
    explicit mat_assembler(const std::string &fname) : assembler(fname) {}
    #endif
    mat_assembler() = delete;
    virtual ~mat_assembler() = default;

    mat_assembler(const mat_assembler&) = delete;
    mat_assembler(mat_assembler&&) = delete;
    mat_assembler& operator=(const mat_assembler&) = delete;
    mat_assembler& operator=(mat_assembler&&) = delete;

 private:
    string name_pattern(void) const override {
        return cmd_name_pattern;
    }
    string name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    int open_output_file(const string&) override;
    void close_output_file(void) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;
    int process_extra_data(const string &, const string &) override;
    string assist_line_pattern(void) const override {
        return extra_line_pattern;
    }

    int process_assist_line(const string&);

 private:
    std::vector<std::vector<std::uint64_t>> ram_mcode_vec{8};
    std::vector<std::string> start_end_stk;
    std::uint16_t cur_ram_idx{0};
    std::vector<std::ofstream> ot_fstrms;

    static const char* cmd_name_pattern;
    static const char* extra_line_pattern;
    static const int rvs_flg_idx;
    static const int l_flg_idx;
    static const int crc_flg_idx;
    static const int xor_flg_idx;
    static const int sm_flg_idx;
    static const str_u64_map cmd_opcode_map;
    static const u64_regex_map opcode_regex_map;
};

#endif  // INC_MAT_ASSEMBLER_H_
