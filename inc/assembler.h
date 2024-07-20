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

using std::string;
using std::vector;
using std::regex;
using std::smatch;

constexpr auto comment_empty_line_p = R"(^\s*\/\/.*[\n\r]?$|^\s*$)";
constexpr auto g_normal_line_prefix_p = R"(^\s*[A-Z\d+-]+\s+)";
constexpr auto g_normal_line_posfix_p = R"(\s*;\s*(\/\/.*)?$)";

class assembler {
    friend class parser_assembler;
    friend class deparser_assembler;
    friend class mat_assembler;

 public:
    assembler() = default;
    virtual ~assembler() = default;

    assembler(const assembler&) = delete;
    assembler(assembler&&) = delete;
    assembler& operator=(const assembler&) = delete;
    assembler& operator=(assembler&&) = delete;

    int execute(const string&, const string&);

    static std::uint32_t get_xor_unit(const bool xor8_flg, const bool xor16_flg, const bool xor32_flg) {
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
    virtual string get_name_pattern(void) const = 0;
    virtual string get_name_matched(const smatch&, vector<bool>&) const = 0;
    virtual int line_process(const string&, const string&, const vector<bool>&) = 0;
    virtual void write_machine_code(void) = 0;
    virtual void print_machine_code(void) = 0;

 private:
    int open_output_file(const string &out_fname);
    void close_output_file(void) {
        dst_fstrm.close();
    }

    std::ifstream src_fstrm;
    std::ofstream dst_fstrm;

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

    static void print_cmd_param_unmatch_message(const string &name, const string &line) {
        std::cout << name + " doesn't match those parameters.\n\t" << line << std::endl;
    }
};

#endif  // INC_ASSEMBLER_H_
