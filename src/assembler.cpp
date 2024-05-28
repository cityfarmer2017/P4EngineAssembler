#include "assembler.h"

using std::cout;
using std::endl;

int assembler::execute(const string &in_fname, const string &out_fname)
{
    string line;

    src_fstrm.open(in_fname);
    if (!src_fstrm) {
        std::cout << "cannot open source file: " << in_fname << std::endl;
        return -1;
    }

    while (getline(src_fstrm, line)) {
        const regex r(R"(^\s*)" + get_name_pattern() + R"(\s+.*;\s*(\/\/.*)?[\n\r]?$)");
        smatch m;
        if (!regex_match(line, m, r)) {
            const regex r(comment_empty_line_p);
            if (regex_match(line, r)) {
                continue;
            }

            while (line.at(line.size() - 1) == '\n' || line.at(line.size() - 1) == '\r') {
                line.pop_back();
            }

            cout << "ERROR:\tline - \" " + line + " \""<< endl;

            return -1;
        }

        while (line.at(line.size() - 1) == '\n' || line.at(line.size() - 1) == '\r') {
            line.pop_back();
        }

        vector<bool> flags;
        string name = get_name_matched(m, flags);

        int rc = 0;
        rc = line_process(line, name, flags);
        if (rc) {
            return rc;
        }

        flags.clear();
    }

    if (line.empty()) {
        return -1;
    }

    return write_mcode_to_destinations(out_fname);
}

int assembler::open_output_file(const string &out_fname)
{
    string posfix = out_fname.substr(out_fname.size() - 4);
    if (posfix == ".dat") {
        dst_fstrm.open(out_fname, std::ios::binary);
    } else if (posfix == ".txt") {
        dst_fstrm.open(out_fname);
    } else {
        std::cout << "wrong file posfix." << endl;
        return -1;
    }
    
    if (!dst_fstrm) {
        std::cout << "cannot open out file: " << out_fname << std::endl;
        return -1;
    }

    return 0;
}

const string assembler::comment_empty_line_p = R"(^\s*\/\/.*[\n\r]?$|^\s*$)";
const string assembler::normal_line_prefix_p = R"(^\s*[A-Z\d+-]+\s+)";
const string assembler::normal_line_posfix_p = R"(\s*;\s*(\/\/.*)?$)";