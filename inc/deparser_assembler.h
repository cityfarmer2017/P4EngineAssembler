#ifndef DEPARSER_ASSEMBLER_H
#define DEPARSER_ASSEMBLER_H

#include "assembler.h"
#include "deparser_def.h"

using str_u32_map = std::unordered_map<string, u32>;
using u32_regex_map = std::unordered_map<u32, regex>;

class deparser_assembler : public assembler {
public:
    deparser_assembler() = default;
    virtual ~deparser_assembler() = default;

    deparser_assembler(const deparser_assembler&) = delete;
    deparser_assembler(deparser_assembler&&) = delete;
    deparser_assembler& operator=(const deparser_assembler&) = delete;
    deparser_assembler& operator=(deparser_assembler&&) = delete;

protected:
    inline string get_name_pattern(void) const override;
    string get_name_matched(const smatch&, vector<bool>&) const override;
    int line_process(const string&, const string&, const vector<bool>&) override;
    void write_machine_code(void) override;
    void print_machine_code(void) override;

private:
    std::vector<u32> mcode_vec;
    std::string prev_line_name;

    static const string cmd_name_pattern;
    static const int sndm_flg_idx;
    static const int calc_flg_idx;
    static const int xor_flg_idx;
    static const int j_flg_idx;
    static const str_u32_map cmd_opcode_map;
    static const u32_regex_map opcode_regex_map;

    int check_previous(const string &line) const
    {
        if (prev_line_name != "MSKALL" && prev_line_name != "MSKADDR") {
            return -1;
        }
        return 0;
    }

    void swap_previous(machine_code &mcode)
    {
        auto val = mcode_vec.back();
        mcode_vec.pop_back();
        mcode_vec.emplace_back(mcode.val32);
        mcode.val32 = val;
    }

    static void print_cmd_bit_vld_unmatch_message(const string &line)
    {
        std::cout << line + "\n\tshall be following a <MSKALL / MSKADDR> line." << std::endl;
    }
};

#endif // DEPARSER_ASSEMBLER_H