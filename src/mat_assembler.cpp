/**
 * Copyright [2024] <wangdc1111@gmail.com>
 */
#include <limits>
#include <filesystem>
#include "../inc/mat_assembler.h"
#include "../inc/mat_def.h"

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

    auto rvs16_flg = false;
    auto rvs32_flg = false;
    auto rvs64_flg = false;
    auto rvs128_flg = false;
    auto rvs_str = m.str(rvs_flg_idx);
    if (!rvs_str.empty()) {
        if (rvs_str == "R16") {
            rvs16_flg = true;
        } else if (rvs_str == "R32") {
            rvs32_flg = true;
        } else if (rvs_str == "R64") {
            rvs64_flg = true;
        } else {  // (rvs_str == "R128")
            rvs128_flg = true;
        }
        name.erase(0, rvs_str.size());
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
    flags.emplace_back(rvs16_flg);
    flags.emplace_back(rvs32_flg);
    flags.emplace_back(rvs64_flg);
    flags.emplace_back(rvs128_flg);

    return name;
}

constexpr auto LONG_FLG_IDX = 0;
constexpr auto CRC_POLY1_FLG_IDX = 1;
constexpr auto XOR8_FLG_IDX = 2;
constexpr auto XOR16_FLG_IDX = 3;
constexpr auto XOR32_FLG_IDX = 4;
constexpr auto STM32_FLG_IDX = 5;
constexpr auto RVS16_FLAG_IDX = 6;
constexpr auto RVS32_FLAG_IDX = 7;
constexpr auto RVS64_FLAG_IDX = 8;
constexpr auto RVS128_FLAG_IDX = 9;
constexpr auto MATCH_ASSIST_LINE_FLG_IDX = 10;
constexpr auto FLAGS_SZ = 11;

int mat_assembler::process_assist_line(const string &line) {
    const auto r = std::regex(R"(^)" + assist_line_pattern() + R"((\s+\/\/.*)?[\n\r]?$)");
    std::smatch m;
    if (!std::regex_match(line, m, r)) {
        return -1;
    }

    if (m.str(1) == "start") {
        if (!start_end_stk.empty() && start_end_stk.back() != "end") {
            std::cout << "line #" << src_file_line_idx() << ": \n\t";
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
            std::cout << "line #" << src_file_line_idx() << ": \n\t";
            std::cout << "a '.end' line shall be coupled with a '.start' line." << std::endl;
            return -1;
        }
        start_end_stk.emplace_back("end");
        ++cur_ram_idx;
    }

    return 0;
}

int mat_assembler::line_process(const string &line, const string &name, const vector<bool> &flags) {
    if (flags.size() != FLAGS_SZ) {
        return -1;
    }

    if (flags[MATCH_ASSIST_LINE_FLG_IDX]) {
        return process_assist_line(line);
    }

    if (!(start_end_stk.size() % 2)) {
        std::cout << "any normal code line shall be located between a '.start' and a '.end'" << std::endl;
        return -1;
    }

    if (start_end_stk.size() / 2 < 6 && flags[LONG_FLG_IDX]) {
        std::cout << "line #" << src_file_line_idx() << ": " << line << "\n\t";
        std::cout << "long instructions shall be located in ram_6-7 only." << std::endl;
        return -1;
    }

    if (start_end_stk.size() / 2 >= 6 && !flags[LONG_FLG_IDX]) {
        std::cout << "line #" << src_file_line_idx() << ": " << line << "\n\t";
        std::cout << "normal instructions shall be located in ram_0-5 only." << std::endl;
        return -1;
    }

    machine_code mcode;
    if (!cmd_opcode_map.count(name)) {
        std::cout << "ERROR: line #" << src_file_line_idx() << "\n\t";
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
        print_line_unmatch_message(line);
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
        if (flags[RVS16_FLAG_IDX]) {
            mcode.op_00011.swap_mode = 1;
        } else if (flags[RVS32_FLAG_IDX]) {
            mcode.op_00011.swap_mode = 2;
        } else if (flags[RVS64_FLAG_IDX]) {
            mcode.op_00011.swap_mode = 3;
        } else if (flags[RVS128_FLAG_IDX]) {
            if (!flags[LONG_FLG_IDX]) {
                std::cout << "instruction name wrong.\n\t" << line << std::endl;
                return -1;
            }
            mcode.op_00011.swap_mode = 4;
        }
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
            mcode.op_10101.calc_mode = flags[CRC_POLY1_FLG_IDX] ? 1 : 2;
        } else if (name == "XOR") {
            mcode.op_10101.calc_mode = 3;
            mcode.op_10101.xor_unit = xor_unit(flags[XOR8_FLG_IDX], flags[XOR16_FLG_IDX], flags[XOR32_FLG_IDX]);
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
        if (flags[STM32_FLG_IDX]) {
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
    #if !NO_TBL_PROC
    if (p_tbl == nullptr) {
        std::cout << "ERROR: table class instantiation failed." << std::endl;
        return -1;
    }
    if (auto rc = p_tbl->generate_table_data(shared_from_this())) {
        return rc;
    }
    #endif
    return 0;
}

const char* mat_assembler::cmd_name_pattern =
    R"(((MOV|(R16|R32|R64|R128)?COPY|NOP)(L)?|)"
    R"(MDF|COUNT|METER|LOCK|ULCK|HASH|(CRC)16P([12])|(XOR)(4|8|16|32)|(RSM|WSM)(16|32)))";

const char* mat_assembler::extra_line_pattern = R"(\.(start|end))";

const int mat_assembler::rvs_flg_idx = 3;
const int mat_assembler::l_flg_idx = 4;
const int mat_assembler::crc_flg_idx = 6;
const int mat_assembler::xor_flg_idx = 8;
const int mat_assembler::sm_flg_idx = 10;

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
    {0b00001, regex(string(CODE_LINE_PREFIX_P) + P_00001 + CODE_LINE_POSFIX_P)},
    {0b00010, regex(string(CODE_LINE_PREFIX_P) + P_00010 + CODE_LINE_POSFIX_P)},
    {0b00011, regex(string(CODE_LINE_PREFIX_P) + P_00011 + CODE_LINE_POSFIX_P)},
    {0b00100, regex(string(CODE_LINE_PREFIX_P) + P_00100 + CODE_LINE_POSFIX_P)},
    {0b00101, regex(string(CODE_LINE_PREFIX_P) + P_00101 + CODE_LINE_POSFIX_P)},
    {0b00110, regex(string(CODE_LINE_PREFIX_P) + P_00110_00111 + CODE_LINE_POSFIX_P)},
    {0b00111, regex(string(CODE_LINE_PREFIX_P) + P_00110_00111 + CODE_LINE_POSFIX_P)},
    {0b10100, regex(string(CODE_LINE_PREFIX_P) + P_10100 + CODE_LINE_POSFIX_P)},
    {0b10101, regex(string(CODE_LINE_PREFIX_P) + P_10101 + CODE_LINE_POSFIX_P)},
    {0b10110, regex(string(CODE_LINE_PREFIX_P) + P_10110 + CODE_LINE_POSFIX_P)},
    {0b10111, regex(string(CODE_LINE_PREFIX_P) + P_10111_11000 + CODE_LINE_POSFIX_P)},
    {0b11000, regex(string(CODE_LINE_PREFIX_P) + P_10111_11000 + CODE_LINE_POSFIX_P)}
};
