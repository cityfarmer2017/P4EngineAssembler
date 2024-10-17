/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_DEPARSER_ASSEMBLER_H_
#define INC_DEPARSER_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <map>
#include "assembler.h"  // NOLINT [build/include_subdir]

using str_u32_map = std::unordered_map<string, std::uint32_t>;
using u32_regex_map = std::unordered_map<std::uint32_t, regex>;

class deparser_assembler : public assembler {
    friend class mask_table;

 public:
    #if !NO_TBL_PROC && !NO_PRE_PROC
    deparser_assembler(std::unique_ptr<table> tbl, const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : assembler(std::move(tbl), pre, fname) {}
    #elif !NO_TBL_PROC
    explicit deparser_assembler(std::unique_ptr<table> tbl, const std::string &fname)
        : assembler(std::move(tbl), fname) {}
    #elif !NO_PRE_PROC
    explicit deparser_assembler(const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : assembler(pre, fname) {}
    #else
    explicit deparser_assembler(const std::string &fname) : assembler(fname) {}
    #endif
    deparser_assembler() = delete;
    virtual ~deparser_assembler() = default;

    deparser_assembler(const deparser_assembler&) = delete;
    deparser_assembler(deparser_assembler&&) = delete;
    deparser_assembler& operator=(const deparser_assembler&) = delete;
    deparser_assembler& operator=(deparser_assembler&&) = delete;

 private:
    string name_pattern(void) const override {
        return cmd_name_pattern;
    }
    string name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;
    int process_extra_data(const string &, const string &) override;

    int align_mcode_per_file_block();

    inline bool previous_not_mask(const string &) const;
    inline void swap_previous(const std::uint32_t &);

 private:
    std::vector<std::uint32_t> mcode_vec;
    std::string prev_line_name;

    #if !NO_TBL_PROC
    bool be_mask_table_necessary{false};
    #endif

    #if !NO_PRE_PROC
    std::string cur_fname;
    std::map<std::string, std::uint16_t> cur_fname_2_line_idx;
    std::vector<std::uint16_t> block_line_vec;
    int inc_file_line_idx();
    int inc_block_line_idx();
    int switch_file();
    int switch_block();
    inline void print_code_file_exceed_message();
    inline void print_code_block_exceed_message();
    #endif

    static const char* cmd_name_pattern;
    static const int sndm_flg_idx;
    static const int calc_flg_idx;
    static const int xor_flg_idx;
    static const int j_flg_idx;
    static const str_u32_map cmd_opcode_map;
    static const u32_regex_map opcode_regex_map;
};

#endif  // INC_DEPARSER_ASSEMBLER_H_
