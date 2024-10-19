/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_ASSEMBLER_H_
#define INC_ASSEMBLER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <regex>  // NOLINT [build/c++11]
#include <bitset>
#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <filesystem>
#include "../inc/global_def.h"
#if !NO_TBL_PROC
#include "table_proc/table.h"
#endif
#if !NO_PRE_PROC
#include "pre_proc/preprocessor.h"
#endif

using std::string;
using std::vector;
using std::regex;
using std::smatch;

constexpr auto INSTRUCTION_LINE_PREFIX_P = R"(^\s*[A-Z\d+-]+\s+)";
constexpr auto INSTRUCTION_LINE_POSFIX_P = R"(\s*;\s*(\/\/.*)?$)";

class assembler : public std::enable_shared_from_this<assembler> {
    friend class parser_assembler;
    friend class deparser_assembler;
    friend class mat_assembler;

 public:
    #if !NO_TBL_PROC && !NO_PRE_PROC
    assembler(std::unique_ptr<table> tbl, const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : p_tbl(std::move(tbl)), p_pre(pre), src_fname(fname) {}
    #elif !NO_TBL_PROC
    assembler(std::unique_ptr<table> tbl, const std::string &fname)
        : p_tbl(std::move(tbl)), src_fname(fname) {}
    #elif !NO_PRE_PROC
    assembler(const std::shared_ptr<preprocessor> &pre, const std::string &fname)
        : p_pre(pre), src_fname(fname) {}
    #else
    explicit assembler(const std::string &fname) : src_fname(fname) {}
    #endif
    assembler() = delete;
    virtual ~assembler() = default;

    assembler(const assembler&) = delete;
    assembler(assembler&&) = delete;
    assembler& operator=(const assembler&) = delete;
    assembler& operator=(assembler&&) = delete;

    static int handle(const std::filesystem::path&, std::string);

    int execute(const string&, const string&);

    const string src_file_stem() const {
        return std::filesystem::path(src_fname).stem().string();
    }

    static std::uint32_t xor_unit(const bool xor8_flg, const bool xor16_flg, const bool xor32_flg) {
        if (xor8_flg) {
            return 1;
        } else if (xor16_flg) {
            return 2;
        } else if (xor32_flg) {
            return 3;
        }
        return 0;
    }

 protected:
    virtual string name_pattern(void) const = 0;
    virtual string name_matched(const smatch&, vector<bool>&) const = 0;
    virtual int line_process(const string&, const string&, const vector<bool>&) = 0;
    virtual void write_machine_code(void) = 0;
    virtual void print_machine_code(void) = 0;
    virtual int process_extra_data(const string&, const string&) = 0;
    virtual string assist_line_pattern(void) const {
        return "default";
    }
    virtual int open_output_file(const string &out_fname);
    virtual void close_output_file(void) {
        dst_fstrm.close();
    }
    virtual int check_state_chart_valid() {
        return 0;
    }

    std::string src_file_name() const {
        #if !NO_PRE_PROC
        return p_pre->dst_cline_2_src_fline[ir_fname_line_idx_str()].first;
        #else
        return src_fname;
        #endif
    }

    std::uint16_t src_file_line_idx() const {
        #if !NO_PRE_PROC
        return p_pre->dst_cline_2_src_fline[ir_fname_line_idx_str()].second;
        #else
        return file_line_idx;
        #endif
    }

    #if !NO_TBL_PROC
    std::unique_ptr<table> p_tbl;
    #endif

    #if !NO_PRE_PROC
    std::string ir_fname_line_idx_str() const {
        return src_fname + IR_STR + std::to_string(file_line_idx);
    }
    std::shared_ptr<preprocessor> p_pre;
    #endif

 private:
    static void print_mcode_line_by_line(std::ostream &os, const std::vector<std::uint64_t> &vec) {
        for (const auto &mcode : vec) {
            os << std::bitset<64>(mcode) << "\n";
        }
        os << std::flush;
    }

    static void print_mcode_line_by_line(std::ostream &os, const std::vector<std::uint32_t> &vec) {
        for (const auto &mcode : vec) {
            os << std::bitset<32>(mcode) << "\n";
        }
        os << std::flush;
    }

    void print_line_file_info(string line) {
        std::cout << "line #" << src_file_line_idx() << " in file - " << src_file_name() << "\n\t";
        std::cout << "\" " + trim_ends(line) + " \""<< std::endl;
    }

    void print_cmd_param_unmatch_message(const string &name, const string &line) {
        std::cout << name + " doesn't match those parameters, please check carefully.\n\t";
        print_line_file_info(line);
    }

    void print_line_unmatch_message(string line) {
        std::cout << "ERROR:\tline matching pattern.\n\t";
        print_line_file_info(line);
    }

    std::ofstream dst_fstrm;
    std::string src_fname;
    std::uint16_t file_line_idx{0};
};

#endif  // INC_ASSEMBLER_H_
