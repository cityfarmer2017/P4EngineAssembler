/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include "assembler.h"  // NOLINT [build/include_subdir]

using std::cout;
using std::endl;

int assembler::execute(const string &in_fname, const string &out_fname) {
    string line;

    std::ifstream src_fstrm(in_fname);
    if (!src_fstrm) {
        std::cout << "cannot open source file: " << in_fname << std::endl;
        return -1;
    }

    while (getline(src_fstrm, line)) {
        ++file_line_idx;

        const regex r2(R"(^)" + assist_line_pattern() + R"((\s+\/\/.*)?[\n\r]?$)");
        auto match_assist_line = regex_match(line, r2);
        const regex r1(R"(^\s*)" + name_pattern() + R"(\s+.*;\s*(\/\/.*)?[\n\r]?$)");
        smatch m;
        if (!regex_match(line, m, r1) && !match_assist_line) {
            const regex r(comment_empty_line_p);
            if (regex_match(line, r)) {
                continue;
            }

            while (line.back() == '\n' || line.back() == '\r') {
                line.pop_back();
            }

            cout << "ERROR:\tline #" << file_line_idx << " - \" " + line + " \""<< endl;

            return -1;
        }

        while (line.back() == '\n' || line.back() == '\r') {
            line.pop_back();
        }

        vector<bool> flags;
        string name = name_matched(m, flags);
        flags.emplace_back(match_assist_line ? true : false);

        if (auto rc = line_process(line, name, flags)) {
            return rc;
        }

        flags.clear();
    }

    if (auto rc = open_output_file(out_fname + ".dat")) {
        return rc;
    }
    write_machine_code();
    close_output_file();

    if (auto rc = open_output_file(out_fname + ".txt")) {
        return rc;
    }
    print_machine_code();
    close_output_file();

    return process_extra_data(in_fname, out_fname);
}

int assembler::open_output_file(const string &out_fname) {
    if (dst_fstrm.is_open()) {
        std::cout << "output stream is in use." << std::endl;
        return -1;
    }
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
