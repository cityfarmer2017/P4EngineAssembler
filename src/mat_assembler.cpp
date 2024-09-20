/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <limits>
#include <filesystem>
#include "mat_assembler.h"  // NOLINT [build/include_subdir]
#include "mat_def.h"  // NOLINT [build/include_subdir]

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

string mat_assembler::name_matched(const smatch &m, vector<bool> &flags) const {
    auto name = m.str(1);

    auto long_flg = !m.str(l_flg_idx).empty() || name == "HASH";

    auto crc_p1_flg = false;
    // auto crc_p2_flg = false;
    if (!m.str(crc_flg_idx).empty()) {
        crc_p1_flg = m.str(crc_flg_idx) == "1";
        // crc_p2_flg = m.str(crc_flg_idx) == "2";
        long_flg = true;
        name = m.str(crc_flg_idx-1);
    }

    // auto xor_4_flg = false;
    auto xor_8_flg = false;
    auto xor_16_flg = false;
    auto xor_32_flg = false;
    if (!m.str(xor_flg_idx).empty()) {
        // xor_4_flg = m.str(xor_flg_idx) == "4";
        xor_8_flg = m.str(xor_flg_idx) == "8";
        xor_16_flg = m.str(xor_flg_idx) == "16";
        xor_32_flg = m.str(xor_flg_idx) == "32";
        long_flg = true;
        name = m.str(xor_flg_idx-1);
    }

    // auto sm_16_flg = false;
    auto sm_32_flg = false;
    if (!m.str(sm_flg_idx).empty()) {
        // sm_16_flg = m.str(sm_flg_idx) == "16";
        sm_32_flg = m.str(sm_flg_idx) == "32";
        long_flg = true;
        name = m.str(sm_flg_idx-1);
    }

    flags.emplace_back(long_flg);
    flags.emplace_back(crc_p1_flg);
    // flags.emplace_back(crc_p2_flg);
    // flags.emplace_back(xor_4_flg);
    flags.emplace_back(xor_8_flg);
    flags.emplace_back(xor_16_flg);
    flags.emplace_back(xor_32_flg);
    // flags.emplace_back(sm_16_flg);
    flags.emplace_back(sm_32_flg);

    return name;
}

constexpr auto long_flg_idx = 0;
constexpr auto crc_poly1_flg_idx = 1;
constexpr auto xor8_flg_idx = 2;
constexpr auto xor16_flg_idx = 3;
constexpr auto xor32_flg_idx = 4;
constexpr auto stm32_flg_idx = 5;
constexpr auto match_assist_line_flg_idx = 6;
constexpr auto flags_size = 7;

int mat_assembler::process_assist_line(const string &line) {
    const auto r = std::regex(R"(^)" + assist_line_pattern() + R"((\s+\/\/.*)?[\n\r]?$)");
    std::smatch m;
    if (!std::regex_match(line, m, r)) {
        return -1;
    }

    if (m.str(1) == "start") {
        if (!start_end_stk.empty() && start_end_stk.back() != "end") {
            std::cout << "line #" << file_line_idx << ": \n\t";
            std::cout << "a '.start' line shall be the initial line or succeeding a '.end' line." << std::endl;
            return -1;
        }
        start_end_stk.emplace_back("start");
        // cur_ram_idx = start_end_stk.size() / 2;
        if (cur_ram_idx > 7) {
            std::cout << "test" << std::endl;
            return -1;
        }
    } else {
        if (start_end_stk.empty() || start_end_stk.back() != "start") {
            std::cout << "line #" << file_line_idx << ": \n\t";
            std::cout << "a '.end' line shall be coupled with a '.start' line." << std::endl;
            return -1;
        }
        start_end_stk.emplace_back("end");
        ++cur_ram_idx;
    }

    return 0;
}

int mat_assembler::line_process(const string &line, const string &name, const vector<bool> &flags) {
    if (flags.size() != flags_size) {
        return -1;
    }

    if (flags[match_assist_line_flg_idx]) {
        return process_assist_line(line);
    }

    if (!(start_end_stk.size() % 2)) {
        std::cout << "any normal code line shall be located between a '.start' and a '.end'" << std::endl;
        return -1;
    }

    if (start_end_stk.size() / 2 < 6 && flags[long_flg_idx]) {
        std::cout << "line #" << file_line_idx << ": " << line << "\n\t";
        std::cout << "long instructions shall be located in ram_6-7 only." << std::endl;
        return -1;
    }

    if (start_end_stk.size() / 2 >= 6 && !flags[long_flg_idx]) {
        std::cout << "line #" << file_line_idx << ": " << line << "\n\t";
        std::cout << "normal instructions shall be located in ram_0-5 only." << std::endl;
        return -1;
    }

    machine_code mcode;
    if (!cmd_opcode_map.count(name)) {
        std::cout << "ERROR: line #" << file_line_idx << "\n\t";
        std::cout << "no opcode match this instruction name - " << name << std::endl;
        return -1;
    }
    mcode.val64 = cmd_opcode_map.at(name);
    mcode.universe.imm64_l = 0;
    mcode.universe.imm64_h = 0;

    if (name == "NOP" || name == "NOPL") {
        ram_mcode_vec[cur_ram_idx].emplace_back(mcode.val64);
        return 0;
    }

    smatch m;
    if (!regex_match(line, m, opcode_regex_map.at(mcode.val64))) {
        cout << "regex match failed with line: #" << file_line_idx << "\n\t" << line << endl;
        return -1;
    }

    switch (mcode.val64) {
    case 0b00001:  // MOV
        if (m.str(2).empty()) {
            if (stoull(m.str(1), nullptr, 0) > std::numeric_limits<std::uint32_t>::max()) {
            cout << "imm32 value exceeds limit.\n\t" << line << endl;
                return -1;
            }
            mcode.op_00001.imm32 = stoul(m.str(1), nullptr, 0);  // heximal or decimal
        } else {
            mcode.op_00001.mode = 1;
            mcode.op_00001.ad_idx = stoul(m.str(2));
        }
        if (!m.str(4).empty()) {
            mcode.op_00001.dst_slct = 1;
            mcode.op_00001.offset = stoul(m.str(5));
            mcode.op_00001.length = stoul(m.str(6)) - 1;
        } else {
            mcode.op_00001.offset = stoul(m.str(7));
            mcode.op_00001.length = stoul(m.str(8)) - 1;
        }
        break;

    case 0b00010:  // MDF
        if (m.str(2).empty()) {
            if (stoull(m.str(1), nullptr, 0) > std::numeric_limits<uint16_t>::max()) {
            cout << "imm16 value exceeds limit.\n\t" << line << endl;
                return -1;
            }
            mcode.op_00010.imm16 = stoul(m.str(1), nullptr, 0);  // heximal or decimal
        } else {
            mcode.op_00010.mode = 1;
            mcode.op_00010.ad_idx = stoul(m.str(2));
        }
        mcode.op_00010.mask = stoul(m.str(3), nullptr, 0);
        if (!m.str(5).empty()) {
            mcode.op_00010.dst_slct = 1;
            mcode.op_00010.offset = stoul(m.str(6));
            mcode.op_00010.length = stoul(m.str(7)) - 1;
        } else {
            mcode.op_00010.offset = stoul(m.str(8));
            mcode.op_00010.length = stoul(m.str(9)) - 1;
        }
        break;

    case 0b00011:  // COPY
    case 0b10110:  // COPYL
        if (m.str(1) == "PHV" && m.str(4) == "META") {
            mcode.op_00011.direction = 1;
        } else if (m.str(1) == "META" && m.str(4) == "PHV") {
            mcode.op_00011.direction = 2;
        } else if (m.str(1) == "META" && m.str(4) == "META") {
            mcode.op_00011.direction = 3;
        }
        mcode.op_00011.src_off = stoul(m.str(2));
        mcode.op_00011.length = stoul(m.str(3)) - 1;
        mcode.op_00011.dst_off = stoul(m.str(5));
        break;

    case 0b00100:  // COUNT
        if (m.str(1).empty()) {
            mcode.op_00100.mode = 1;
        } else if (!m.str(2).empty()) {
            mcode.op_00100.counter_id = stoul(m.str(2), nullptr, 0);
        } else {  // if (!m.str(3).empty()) {
            mcode.op_00100.ad_idx = stoul(m.str(3));
            mcode.op_00100.mode = 2;
        }
        break;

    case 0b00101:  // METER
        if (m.str(1).empty()) {
            mcode.op_00101.mode = 1;
        } else if (!m.str(2).empty()) {
            mcode.op_00101.meter_id = stoul(m.str(2), nullptr, 0);
        } else {  // if (!m.str(3).empty()) {
            mcode.op_00101.ad_idx = stoul(m.str(3));
            mcode.op_00101.mode = 2;
        }
        mcode.op_00101.dst_off = stoul(m.str(4));
        break;

    case 0b00110:  // LOCK
    case 0b00111:  // ULCK
        if (m.str(1).empty()) {
            mcode.op_00110.mode = 1;
        } else if (!m.str(2).empty()) {
            mcode.op_00110.flow_id = stoul(m.str(2), nullptr, 0);
        } else {  // if (!m.str(3).empty()) {
            mcode.op_00110.ad_idx = stoul(m.str(3));
            mcode.op_00110.mode = 2;
        }
        break;

    case 0b10100:  // MOVL
        if (!m.str(2).empty()) {
            mcode.universe.imm64_l = std::stoull(m.str(2), nullptr, 0);
            if (!m.str(3).empty()) {
                mcode.universe.imm64_h = std::stoull(m.str(4), nullptr, 0);
            }
            mcode.op_10100.length = 4;
        } else {
            mcode.op_10100.mode = 1;
            mcode.op_10100.ad_idx = stoul(m.str(5));
            mcode.op_10100.length = stoul(m.str(6)) - 1;
        }
        mcode.op_10100.offset = stoul(m.str(7));
        break;

    case 0b10101:  // HASH / CRC16P[12] / XOR(4|8|16|32)
        if (name == "HASH") {
            if (m.str(4).empty()) {
                print_cmd_param_unmatch_message(name, line);
                return -1;
            }
            mcode.op_10101.ad_idx = stoul(m.str(5));
        } else {
            if (!m.str(4).empty()) {
                print_cmd_param_unmatch_message(name, line);
                return -1;
            }
        }
        mcode.op_10101.src1_off = stoul(m.str(1));
        if (!m.str(2).empty()) {
            mcode.op_10101.src_mode = 1;
            mcode.op_10101.src2_off = stoul(m.str(3));
        }
        if (m.str(6) == "PHV") {
            mcode.op_10101.dst_slct = 1;
        }
        mcode.op_10101.dst_off = stoul(m.str(7));
        if (name == "CRC") {
            mcode.op_10101.calc_mode = flags[crc_poly1_flg_idx] ? 1 : 2;
        } else if (name == "XOR") {
            mcode.op_10101.calc_mode = 3;
            mcode.op_10101.xor_unit = xor_unit(flags[xor8_flg_idx], flags[xor16_flg_idx], flags[xor32_flg_idx]);
        }
        break;

    case 0b10111:  // RSM(16|32)
    case 0b11000:  // WSM(16|32)
        mcode.op_10111.offset = stoul(m.str(1));
        mcode.op_10111.line_shift = stoul(m.str(5));
        if (!m.str(3).empty()) {
            mcode.op_10111.ad_idx = stoul(m.str(3));
            mcode.op_10111.left_shift = stoul(m.str(4));
            mcode.op_10111.mode = 1;
        } else {
            mcode.op_10111.addr_h36 = stoull(m.str(2), nullptr, 0);
        }
        if (flags[stm32_flg_idx]) {
            mcode.op_10111.len = 1;
        }
        break;

    default:
        break;
    }

    ram_mcode_vec[cur_ram_idx].emplace_back(mcode.val64);
    if (mcode.universe.imm64_l) {
        ram_mcode_vec[cur_ram_idx].emplace_back(mcode.universe.imm64_l);
        ram_mcode_vec[cur_ram_idx].emplace_back(mcode.universe.imm64_h);
    }

    return 0;
}

int mat_assembler::open_output_file(const string &out_fname) {
    for (auto i = 0; i < static_cast<int>(ram_mcode_vec.size()); ++i) {
        auto str_strm = std::ostringstream();
        str_strm << "_ram" << i;
        auto ot_fpath = out_fname;
        ot_fpath.insert(ot_fpath.size() - 4, str_strm.str());
        if (auto rc = assembler::open_output_file(ot_fpath)) {
            return rc;
        }
        ot_fstrms.emplace_back(std::move(dst_fstrm));
    }
    return 0;
}

void mat_assembler::close_output_file(void) {
    for (auto &strm : ot_fstrms) {
        strm.close();
    }
    ot_fstrms.clear();
}

void mat_assembler::write_machine_code(void) {
    auto i = 0;
    for (const auto &mcode_vec : ram_mcode_vec) {
        ot_fstrms[i++].write(reinterpret_cast<const char*>(mcode_vec.data()), sizeof(mcode_vec[0]) * mcode_vec.size());
    }
}

void mat_assembler::print_machine_code(void) {
    auto i = 0;
    for (const auto &mcode_vec : ram_mcode_vec) {
        #ifdef DEBUG
        std::cout << "_ram" << i << "\n";
        print_mcode_line_by_line(std::cout, mcode_vec);
        #endif
        print_mcode_line_by_line(ot_fstrms[i++], mcode_vec);
    }
}

int mat_assembler::process_extra_data(const string &in_fname, const string &ot_fname) {
    #if !WITHOUT_SUB_MODULES
    src_fname = std::filesystem::path(in_fname).stem();
    if (auto rc = p_tbl->generate_table_data(shared_from_this())) {
        return rc;
    }
    #endif
    return 0;
}

const char* mat_assembler::cmd_name_pattern =
    R"(((MOV|COPY|NOP)(L)?|MDF|COUNT|METER|LOCK|ULCK|HASH|(CRC)16P([12])|(XOR)(4|8|16|32)|(RSM|WSM)(16|32)))";

const char* mat_assembler::extra_line_pattern = R"(\.(start|end))";

const int mat_assembler::l_flg_idx = 3;
const int mat_assembler::crc_flg_idx = 5;
const int mat_assembler::xor_flg_idx = 7;
const int mat_assembler::sm_flg_idx = 9;

const str_u64_map mat_assembler::cmd_opcode_map = {
    {"MOV",    0b00001},
    {"MDF",    0b00010},
    {"COPY",   0b00011},
    {"NOP",    0b00011},
    {"COUNT",  0b00100},
    {"METER",  0b00101},
    {"LOCK",   0b00110},
    {"ULCK",   0b00111},
    {"MOVL",   0b10100},
    {"HASH",   0b10101},
    {"CRC",    0b10101},
    {"XOR",    0b10101},
    {"COPYL",  0b10110},
    {"NOPL",   0b10110},
    {"RSM",    0b10111},
    {"WSM",    0b11000}
};

const u64_regex_map mat_assembler::opcode_regex_map = {
    {0b00001, regex(string(g_normal_line_prefix_p) + P_00001 + g_normal_line_posfix_p)},
    {0b00010, regex(string(g_normal_line_prefix_p) + P_00010 + g_normal_line_posfix_p)},
    {0b00011, regex(string(g_normal_line_prefix_p) + P_00011 + g_normal_line_posfix_p)},
    {0b00100, regex(string(g_normal_line_prefix_p) + P_00100 + g_normal_line_posfix_p)},
    {0b00101, regex(string(g_normal_line_prefix_p) + P_00101 + g_normal_line_posfix_p)},
    {0b00110, regex(string(g_normal_line_prefix_p) + P_00110_00111 + g_normal_line_posfix_p)},
    {0b00111, regex(string(g_normal_line_prefix_p) + P_00110_00111 + g_normal_line_posfix_p)},
    {0b10100, regex(string(g_normal_line_prefix_p) + P_10100 + g_normal_line_posfix_p)},
    {0b10101, regex(string(g_normal_line_prefix_p) + P_10101 + g_normal_line_posfix_p)},
    {0b10110, regex(string(g_normal_line_prefix_p) + P_10110 + g_normal_line_posfix_p)},
    {0b10111, regex(string(g_normal_line_prefix_p) + P_10111_11000 + g_normal_line_posfix_p)},
    {0b11000, regex(string(g_normal_line_prefix_p) + P_10111_11000 + g_normal_line_posfix_p)}
};
