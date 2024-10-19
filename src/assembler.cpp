/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include "../inc/global_def.h"
#include "../inc/parser_assembler.h"
#include "../inc/table_proc/match_actionid.h"
#include "../inc/deparser_assembler.h"
#include "../inc/table_proc/mask_table.h"
#include "../inc/mat_assembler.h"
#include "../inc/table_proc/mat_link.h"

using std::cout;
using std::endl;

constexpr auto SOURCE_FILE_NAME_P = R"(([a-zA-Z][a-zA-Z0-9]*)(_[a-zA-Z0-9]+)*)";

#if !NO_PRE_PROC
int process_single_file(
    const std::filesystem::path &in_path, std::string dst_dir, const std::shared_ptr<preprocessor> &p_pre) {
#else
int process_single_file(const std::filesystem::path &in_path, std::string dst_dir) {
#endif
    #ifdef DEBUG
    std::cout << in_path.string() << "\n";
    #endif
    auto src_dir = (in_path.has_parent_path() ? in_path.parent_path() : std::filesystem::current_path()).string();
    auto src_fname = in_path.filename().string();
    auto src_fstem = in_path.stem().string();
    auto src_fext = in_path.extension().string();

    if (src_dir.back() != '/') {
        src_dir += "/";
    }

    if (dst_dir.empty()) {
        dst_dir = src_dir;
    }

    if (dst_dir.back() != '/') {
        dst_dir += "/";
    }

    auto valid_fext = std::string(".p4m");
    #if !NO_PRE_PROC
    valid_fext += IR_STR;
    auto original_fname = src_fname.substr(0, src_fname.size() - 1);
    #endif
    if (src_fext == valid_fext) {
        auto re = std::regex(R"(^()" + std::string(SOURCE_FILE_NAME_P) + R"()_(ma[0-9])$)");
        auto m = std::smatch();
        if (!std::regex_match(src_fstem, m, re)) {
            std::cout << "the file name - " << src_fname << " is not valid." << std::endl;
            return -1;
        }

        dst_dir += m.str(1) + "/";
        dst_dir += m.str(4) + "/";
    } else {
        auto re = std::regex(R"(^)" + std::string(SOURCE_FILE_NAME_P) + R"($)");
        if (!std::regex_match(src_fstem, re)) {
            std::cout << "the file name - " << src_fname << " is not valid." << std::endl;
            return -1;
        }

        dst_dir += src_fstem + "/";
    }

    #if !NO_TBL_PROC && !NO_PRE_PROC
    std::shared_ptr<assembler> p_asm(nullptr);
    if (src_fext == ".p4pi") {
        dst_dir += "parser/";
        auto p_tbl = std::make_unique<match_actionid>(src_dir, dst_dir);
        p_asm = std::make_unique<parser_assembler>(std::move(p_tbl), p_pre, original_fname);
    } else if (src_fext == ".p4mi") {
        auto p_tbl = std::make_unique<mat_link>(src_dir, dst_dir);
        p_asm = std::make_unique<mat_assembler>(std::move(p_tbl), p_pre, original_fname);
    } else {  // (src_fext == ".p4di")
        dst_dir += "deparser/";
        auto p_tbl = std::make_unique<mask_table>(src_dir, dst_dir);
        p_asm = std::make_unique<deparser_assembler>(std::move(p_tbl), p_pre, original_fname);
    }
    #elif !NO_TBL_PROC
    std::shared_ptr<assembler> p_asm(nullptr);
    if (src_fext == ".p4p") {
        dst_dir += "parser/";
        auto p_tbl = std::make_unique<match_actionid>(src_dir, dst_dir);
        p_asm = std::make_unique<parser_assembler>(std::move(p_tbl), src_fname);
    } else if (src_fext == ".p4m") {
        auto p_tbl = std::make_unique<mat_link>(src_dir, dst_dir);
        p_asm = std::make_unique<mat_assembler>(std::move(p_tbl), src_fname);
    } else {  // (src_fext == ".p4d")
        dst_dir += "deparser/";
        auto p_tbl = std::make_unique<mask_table>(src_dir, dst_dir);
        p_asm = std::make_unique<deparser_assembler>(std::move(p_tbl), src_fname);
    }
    #elif !NO_PRE_PROC
    std::unique_ptr<assembler> p_asm(nullptr);
    if (src_fext == ".p4pi") {
        dst_dir += "parser/";
        p_asm = std::make_unique<parser_assembler>(p_pre, original_fname);
    } else if (src_fext == ".p4mi") {
        p_asm = std::make_unique<mat_assembler>(p_pre, original_fname);
    } else {  // (src_fext == ".p4di")
        dst_dir += "deparser/";
        p_asm = std::make_unique<deparser_assembler>(p_pre, original_fname);
    }
    #else
    std::unique_ptr<assembler> p_asm(nullptr);
    if (src_fext == ".p4p") {
        p_asm = std::make_unique<parser_assembler>(src_fname);
        dst_dir += "parser/";
    } else if (src_fext == ".p4m") {
        p_asm = std::make_unique<mat_assembler>(src_fname);
    } else {  // (src_fext == ".p4d")
        p_asm = std::make_unique<deparser_assembler>(src_fname);
        dst_dir += "deparser/";
    }
    #endif  // !NO_TBL_PROC && !NO_PRE_PROC

    if (!std::filesystem::exists(dst_dir)) {
        if (!std::filesystem::create_directories(dst_dir)) {
            std::cout << "failed to create directory: " << dst_dir << std::endl;
            return -1;
        }
    }

    #ifdef DEBUG
    std::cout << "\n" << dst_dir + "action" << "\n";
    #endif

    if (p_asm == nullptr) {
        std::cout << "ERROR: assembler instantiation failed." << std::endl;
        return -1;
    }

    return p_asm->execute(src_dir + src_fname, dst_dir + "action");
}

std::string pre_processed_path(const std::filesystem::path &in_path) {
    auto src_dir = (in_path.has_parent_path() ? in_path.parent_path() : std::filesystem::current_path()).string();
    if (src_dir.back() != '/') {
        src_dir += "/";
    }
    return src_dir + IR_PATH + (std::filesystem::is_regular_file(in_path) ? in_path.filename().string() + IR_STR : "");
}

int assembler::handle(const std::filesystem::path &in_path, std::string dst_path) {
    auto src_path = in_path.string();
    #if !NO_PRE_PROC
    auto p_preproc = std::make_shared<preprocessor>();
    if (p_preproc == nullptr) {
        std::cout << "ERROR: preprocessor instantiation failed." << std::endl;
        return -1;
    }
    if (auto rc = p_preproc->handle(in_path)) {
        return rc;
    }
    src_path = pre_processed_path(in_path);
    #endif

    if (std::filesystem::is_regular_file(src_path)) {
        #if !NO_PRE_PROC
        return process_single_file(src_path, dst_path, p_preproc);
        #else
        string src_fext(in_path.extension());
        if ((src_fext != ".p4p") && (src_fext != ".p4m") && (src_fext != ".p4d")) {
            std::cout << "ERROR: wrong sourc file extension." << std::endl;
            return -1;
        }
        return process_single_file(in_path, dst_path);
        #endif
    }

    for (const auto &entry : std::filesystem::directory_iterator(src_path)) {
        #if !NO_PRE_PROC
        if (auto rc = process_single_file(entry.path(), dst_path, p_preproc)) {
            return rc;
        }
        #else
        string src_fext(entry.path().extension());
        if (!entry.is_regular_file() ||
            ((src_fext != ".p4p") && (src_fext != ".p4m") && (src_fext != ".p4d"))) {
            continue;
        }
        if (auto rc = process_single_file(entry.path(), dst_path)) {
            return rc;
        }
        #endif
    }

    return 0;
}

int assembler::execute(const string &in_fname, const string &out_fname) {
    std::ifstream src_fstrm(in_fname);
    if (!src_fstrm) {
        std::cout << "cannot open source file: " << in_fname << std::endl;
        return -1;
    }

    string line;
    while (getline(src_fstrm, line)) {
        ++file_line_idx;

        if (line.back() == '\n' || line.back() == '\r') {
            rtrim(line);
        }

        const regex r2(R"(^)" + assist_line_pattern() + R"((\s+\/\/.*)?$)");
        auto match_assist_line = regex_match(line, r2);
        const regex r1(R"(^\s*)" + name_pattern() + R"(\s+.*;\s*(\/\/.*)?$)");
        smatch m;
        if (!regex_match(line, m, r1) && !match_assist_line) {
            #if NO_PRE_PROC
            const regex r(COMMENT_EMPTY_LINE_P);
            if (regex_match(line, r)) {
                continue;
            }
            #endif

            print_line_unmatch_message(line);

            return -1;
        }

        vector<bool> flags;
        string name = name_matched(m, flags);
        flags.emplace_back(match_assist_line ? true : false);

        if (auto rc = line_process(line, name, flags)) {
            return rc;
        }

        flags.clear();
    }

    if (auto rc = check_state_chart_valid()) {
        return rc;
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
        std::cout << "wrong file extension." << endl;
        return -1;
    }

    if (!dst_fstrm) {
        std::cout << "cannot open out file: " << out_fname << std::endl;
        return -1;
    }

    return 0;
}
