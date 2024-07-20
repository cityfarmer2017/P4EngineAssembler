/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_DEPARSER_ASSEMBLER_H_
#define INC_DEPARSER_ASSEMBLER_H_

#include <unordered_map>
#include <vector>
#include <string>
#include "assembler.h"     // NOLINT [build/include_subdir]

using str_u32_map = std::unordered_map<string, std::uint32_t>;
using u32_regex_map = std::unordered_map<std::uint32_t, regex>;

class deparser_assembler : public assembler {
 public:
    deparser_assembler() = default;
    virtual ~deparser_assembler() = default;

    deparser_assembler(const deparser_assembler&) = delete;
    deparser_assembler(deparser_assembler&&) = delete;
    deparser_assembler& operator=(const deparser_assembler&) = delete;
    deparser_assembler& operator=(deparser_assembler&&) = delete;

 private:
    inline string get_name_pattern(void) const override;
    string get_name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;

    inline int check_previous(const string &) const;
    inline void swap_previous(const std::uint32_t &);

 private:
    std::vector<std::uint32_t> mcode_vec;
    std::string prev_line_name;

    static const char* cmd_name_pattern;
    static const int sndm_flg_idx;
    static const int calc_flg_idx;
    static const int xor_flg_idx;
    static const int j_flg_idx;
    static const str_u32_map cmd_opcode_map;
    static const u32_regex_map opcode_regex_map;
};

#endif  // INC_DEPARSER_ASSEMBLER_H_
