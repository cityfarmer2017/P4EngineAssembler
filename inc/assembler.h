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
#if WITH_SUB_MODULES
#include "table_proc/table.h"
#endif

using std::string;
using std::vector;
using std::regex;
using std::smatch;

constexpr auto comment_empty_line_p = R"(^\s*\/\/.*[\n\r]?$|^\s*$)";
constexpr auto g_normal_line_prefix_p = R"(^\s*[A-Z\d+-]+\s+)";
constexpr auto g_normal_line_posfix_p = R"(\s*;\s*(\/\/.*)?$)";

class assembler : public std::enable_shared_from_this<assembler> {
    friend class parser_assembler;
    friend class deparser_assembler;
    friend class mat_assembler;

 public:
    #if WITH_SUB_MODULES
    explicit assembler(std::unique_ptr<table> tb) : p_tbl(std::move(tb)) {}
    #endif
    assembler() = default;
    virtual ~assembler() = default;

    assembler(const assembler&) = delete;
    assembler(assembler&&) = delete;
    assembler& operator=(const assembler&) = delete;
    assembler& operator=(assembler&&) = delete;

    int execute(const string&, const string&);

    const string src_file_name() const {
        #if WITH_SUB_MODULES
        return src_fname;
        #else
        return "";
        #endif
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
    virtual int process_extra_data(const string &, const string &) {
        return 0;
    }
    virtual string assist_line_pattern(void) const {
        return "default";
    }
    virtual int open_output_file(const string &out_fname);
    virtual void close_output_file(void) {
        dst_fstrm.close();
    }

    #if WITH_SUB_MODULES
    std::unique_ptr<table> p_tbl;
    std::string src_fname;
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

    void print_cmd_param_unmatch_message(const string &name, const string &line) {
        std::cout << name + " doesn't match those parameters.\n\t";
        std::cout << "line #" << file_line_idx << ": " << line << std::endl;
    }

    std::ofstream dst_fstrm;
    std::uint16_t cur_line_idx{0};
    std::uint16_t file_line_idx{0};
};

#endif  // INC_ASSEMBLER_H_
