#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <regex>
#include <bitset>
#include <cstdint>

using std::string;
using std::vector;
using std::regex;
using std::smatch;

using u32 = std::uint32_t;
using u64 = std::uint64_t;

class assembler {
    friend class parser_assembler;
    friend class deparser_assembler;

public:
    assembler() = default;
    virtual ~assembler() = default;

    assembler(const assembler&) = delete;
    assembler(assembler&&) = delete;
    assembler& operator=(const assembler&) = delete;
    assembler& operator=(assembler&&) = delete;

    int execute(const string&, const string&);

    static const string comment_empty_line_p;
    static const string normal_line_prefix_p;
    static const string normal_line_posfix_p;

protected:
    virtual string get_name_pattern(void) const = 0;
    virtual string get_name_matched(const smatch&, vector<bool>&) const = 0;
    virtual int line_process(const string&, const string&, const vector<bool>&) = 0;
    virtual void write_machine_code(void) = 0;
    virtual void print_machine_code(void) = 0;

private:
    int open_output_file(const string &out_fname);
    void close_output_file(void)
    {
        dst_fstrm.close();
    }

    std::ifstream src_fstrm;
    std::ofstream dst_fstrm;

    static void print_mcode_line_by_line(std::ostream &os, const std::vector<u64> &vec)
    {
        for (const auto &mcode: vec) {
            os << std::bitset<64>(mcode) << "\n";
        }
        os << std::flush;
    }

    static void print_mcode_line_by_line(std::ostream &os, const std::vector<u32> &vec)
    {
        for (const auto &mcode: vec) {
            os << std::bitset<32>(mcode) << "\n";
        }
        os << std::flush;
    }

    static void print_cmd_param_unmatch_message(const string &name, const string &line)
    {
        std::cout << name + " doesn't match those parameters.\n\t" << line << std::endl;
    }
};

#endif // ASSEMBLER_H