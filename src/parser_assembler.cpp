/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <limits>
#include <filesystem>
#include <memory>
#include "parser_def.h"  // NOLINT [build/include_subdir]
#include "parser_assembler.h"  // NOLINT [build/include_subdir]
// #if WITH_SUB_MODULES
#include "table_proc/match_actionid.h"
// #endif

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

string parser_assembler::get_name_matched(const smatch &m, vector<bool> &flags) const {
    auto l_flg = !m.str(l_idx).empty();
    auto u_flg = !m.str(u_idx).empty();
    auto m0_flg = !m.str(m0_idx).empty();
    string name;

    if (u_flg) {
        name = m.str(u_idx - 1);
    } else if (m0_flg) {
        name = m.str(m0_idx - 1);
    } else {
        name = m.str(1);
    }

    flags.emplace_back(l_flg);
    flags.emplace_back(u_flg);
    flags.emplace_back(m0_flg);

    return name;
}

constexpr auto last_flg_idx = 0;
constexpr auto unsigned_flg_idx = 1;
constexpr auto mask0_flg_idx = 2;
constexpr auto match_state_no_line_flg_idx = 3;
constexpr auto flags_size = 4;

static inline void compose_mov_mdf(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);

    mcode.op_00001.imm32 = stoul(m.str(1), nullptr, 0);  // heximal or decimal

    mcode.op_00001.src_len = 0x1f;
    if (!m.str(2).empty()) {
        if (m.str(3) == "TMP") {
            mcode.op_00001.src_slct = 1;
        }
        mcode.op_00001.src_off = stoul(m.str(4));
        mcode.op_00001.src_len = stoul(m.str(5)) - 1;
    }

    if (m.str(6) == "TMP") {
        mcode.op_00001.dst_slct = 2;
    } else {
        auto dst_off_idx = 8;
        auto dst_len_idx = 9;
        if (m.str(7) == "META") {
            mcode.op_00001.dst_slct = 1;
        }
        if (m.str(10) == "PHV") {
            dst_off_idx = 11;
            dst_len_idx = 12;
        }
        mcode.op_00001.dst_off = stoul(m.str(dst_off_idx));
        mcode.op_00001.dst_len = stoul(m.str(dst_len_idx)) - 1;
    }
}

static inline void compose_rmv_xct(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "TMP") {
        mcode.op_00011.src_slct = 1;
    }
    mcode.op_00011.src_off = stoul(m.str(2));
    mcode.op_00011.src_len = stoul(m.str(3)) - 1;

    if (m.str(4) == "TMP") {
        mcode.op_00011.dst_slct = 2;
    } else {
        auto dst_off_idx = 6;
        auto dst_len_idx = 7;
        if (m.str(5) == "META") {
            mcode.op_00011.dst_slct = 1;
        }
        if (m.str(8) == "PHV") {
            dst_off_idx = 9;
            dst_len_idx = 10;
        }
        mcode.op_00011.dst_off = stoul(m.str(dst_off_idx));
        mcode.op_00011.dst_len = stoul(m.str(dst_len_idx)) - 1;
    }
}

static inline int compose_addu_subu(const string &name, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "TMP") {
        mcode.op_10101.src0_slct = 1;
    }
    mcode.op_10101.src0_off = stoul(m.str(2));
    mcode.op_10101.src0_len = stoul(m.str(3)) - 1;
    if (name == "SUBU") {
        mcode.op_10101.alu_mode = 1;
    }
    if (m.str(4) == "TMP") {
        mcode.op_10101.src1_slct = 1;
    } else {
        if (stoul(m.str(4), nullptr, 0) > std::numeric_limits<std::uint16_t>::max()) {
            return -1;
        }
        mcode.op_10101.imm16 = stoul(m.str(4), nullptr, 0);
    }
    if (m.str(5) == "TMP") {
        mcode.op_10101.dst_slct = 2;
    } else {
        if (!m.str(6).empty()) {  // META
            mcode.op_10101.dst_slct = 1;
            mcode.op_10101.dst_off = stoul(m.str(7));
            mcode.op_10101.dst_len = stoul(m.str(8)) - 1;
        } else {  // PHV
            mcode.op_10101.dst_off = stoul(m.str(9));
            mcode.op_10101.dst_len = stoul(m.str(10)) - 1;
        }
    }
    return 0;
}

static inline void compose_copy(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) != "CALC_RSLT") {
        auto idx = stoul(m.str(2));
        if (idx <= 3) {
            mcode.op_10100.src_slct = idx + 0b100;
        } else {
            mcode.op_10100.src_slct = idx + 0b1000;
        }
    }
    if (m.str(3) == "TMP") {
        mcode.op_10100.dst_slct = 2;
    } else {
        if (!m.str(4).empty()) {  // META
            mcode.op_10100.dst_slct = 1;
            mcode.op_10100.dst_off = stoul(m.str(5));
            mcode.op_10100.dst_len = stoul(m.str(6)) - 1;
        } else {  // PHV
            mcode.op_10100.dst_off = stoul(m.str(7));
            mcode.op_10100.dst_len = stoul(m.str(8)) - 1;
        }
    }
}

static inline void compose_rsm16_rsm32(const string &name, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (name == "RSM32") {
        mcode.op_00111.len_flg = 1;
    }
    if (m.str(2).empty()) {
        mcode.op_00111.inline_flg = 1;
    } else {
        mcode.op_00111.meta_left_shift = stoul(m.str(3));
    }
    mcode.op_00111.addr = stoull(m.str(1), nullptr, 0);
    mcode.op_00111.line_off = stoul(m.str(4));
}

static inline void compose_shft(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!m.str(1).empty()) {
        if (!m.str(2).empty()) {
            mcode.op_00100.inline_flg = 1;
            mcode.op_00100.length = stoul(m.str(2)) - 1;
        } else if (m.str(3) == "CSUM") {
            mcode.op_00100.calc_slct = 1;
        } else if (m.str(3) == "CRC16") {
            mcode.op_00100.calc_slct = 2;
        } else if (m.str(3) == "CRC32") {
            mcode.op_00100.calc_slct = 3;
        } else if (!m.str(4).empty()) {
            mcode.op_00100.inline_flg = 1;
            mcode.op_00100.length = stoul(m.str(5)) - 1;
            if (m.str(6) == "CSUM") {
                mcode.op_00100.calc_slct = 1;
            } else if (m.str(6) == "CRC16") {
                mcode.op_00100.calc_slct = 2;
            } else if (m.str(6) == "CRC32") {
                mcode.op_00100.calc_slct = 3;
            }
        }
    }
}

static inline int compose_cset(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "POLY") {
        mcode.op_01001.calc_slct = 4;
    } else if (m.str(1) == "INIT") {
        mcode.op_01001.calc_slct = 5;
    } else if (m.str(1) == "XOROT") {
        mcode.op_01001.calc_slct = 7;
    } else {  // (m.str(1) == "CTRL")
        mcode.op_01001.calc_slct = 6;
        mcode.op_01001.ctrl_mode = stoul(m.str(2), nullptr, 2);
    }
    if (m.str(4).empty()) {
        auto val = stoul(m.str(3));
        if (val > std::numeric_limits<std::uint32_t>::max()) {
            return -1;
        }
        mcode.op_01001.value = val;
    } else {
        mcode.op_01001.value = stoul(m.str(3), nullptr, 0);  // heximal
    }
    return 0;
}

static inline void compose_hcsum_hcrc(const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!flags[mask0_flg_idx]) {
        mcode.op_01100.mask_flg = 1;
    }
    if (m.str(1) == "TMP") {
        mcode.op_01100.src_slct = 1;
    } else if (m.str(1) == "PHV") {
        mcode.op_01100.src_slct = 2;
    }
    // for PHV logic will always get data from byte offset 384 and on, totally 128 bytes
    // but the assembler will only open the 32 bytes (480 ~ 511 at the tail to software programmer
    // so here I need to add the offset by extra 96 bytes (480 = 384 + 96)
    mcode.op_01100.offset = stoul(m.str(2)) + (m.str(1) == "PHV" ? 96 : 0);
    mcode.op_01100.length = stoul(m.str(3)) - 1;
    mcode.op_01100.mask = stoul(m.str(4), nullptr, 0);
}

static inline void compose_alu_set(
    const string &name, const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!flags[unsigned_flg_idx]) {
        mcode.op_10000.sign_flg = 1;
    }
    if (name == "SGT") {
        mcode.op_10000.alu_mode = 1;
    } else if (name == "SLT") {
        mcode.op_10000.alu_mode = 2;
    } else if (name == "SEQ") {
        mcode.op_10000.alu_mode = 4;
    } else if (name == "SGE") {
        mcode.op_10000.alu_mode = 5;
    } else if (name == "SLE") {
        mcode.op_10000.alu_mode = 6;
    }
    mcode.op_10000.imm40 = stoull(m.str(3), nullptr, 0);
    mcode.op_10000.offset = stoul(m.str(1));
    mcode.op_10000.length = stoul(m.str(2));
}

static inline int compose_nxth(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!m.str(1).empty()) {
        mcode.op_10001.isr_off = stoul(m.str(2));
        mcode.op_10001.isr_len = stoul(m.str(3));
    }
    if (!m.str(4).empty()) {
        mcode.op_10001.meta_off = stoul(m.str(5));
        mcode.op_10001.meta_len = stoul(m.str(6));
        mcode.op_10001.meta_intrinsic = 1;
    }
    if (mcode.op_10001.meta_len + mcode.op_10001.isr_len > 40) {
        return -1;
    }
    if (!m.str(7).empty()) {
        mcode.op_10001.shift_val = stoul(m.str(8));
    }
    mcode.op_10001.sub_state = stoul(m.str(9));
    if (mcode.op_10001.sub_state > MAX_STATE_NO) {
        cout << "sub state no shall not exceed " << MAX_STATE_NO << endl;
        return -1;
    }
    return 0;
}

int parser_assembler::process_state_no_line(const string &line, const string &pattern) {
    const regex r(pattern);
    smatch m;
    if (!regex_match(line, m, r)) {
        cout << "state no line #" << file_line_idx << " match error.\n\t" << line << endl;
        return -1;
    }
    auto state_no = stoul(m.str(1));
    if (state_no > MAX_STATE_NO) {
        cout << "state no shall not exceed " << MAX_STATE_NO << ".\n\t" << line << endl;
        return -1;
    }
    states_seq.emplace_back(state_no);
    state_line_sub_map.emplace(std::make_pair(state_no, map_of_u16()));
    state_line_sub_map.at(state_no).emplace(std::make_pair(cur_line_idx, 0xFFFF));
    if (state_line_sub_map.at(state_no).size() > MATCH_ENTRY_CNT_PER_STATE) {
        cout << "for each state, the count of sub state shall not exceed " << MATCH_ENTRY_CNT_PER_STATE << endl;
        return -1;
    }
    pre_last_flag = false;
    return 0;
}

int parser_assembler::line_process(const string &line, const string &name, const vector<bool> &flags) {
    if (flags.size() != flags_size) {
        return -1;
    }

    if (flags[match_state_no_line_flg_idx]) {
        return process_state_no_line(line, get_state_no_pattern());
    }

    if (pre_last_flag && !states_seq.empty()) {
        auto cur_state = states_seq.back();
        state_line_sub_map.at(cur_state).emplace(std::make_pair(cur_line_idx, 0xFFFF));
    }

    machine_code mcode;
    mcode.val64 = cmd_opcode_map.at(name);
    smatch m;

    if (!regex_match(line, m, opcode_regex_map.at(mcode.val64))) {
        cout << "regex match failed with line #" << file_line_idx << ":\n\t" << line << endl;
        return -1;
    }

    if (cur_line_idx == 0 && (mcode.universe.opcode != 0b10001 || !flags[last_flg_idx])) {
        cout << "the first line of code must be NXTHL." << endl;
        return -1;
    }

    switch (mcode.val64) {
    case 0b00001:  // MOV, MDF
        if (name == "MOV" && !m.str(2).empty()) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }

        if (stoull(m.str(1), nullptr, 0) > std::numeric_limits<std::uint32_t>::max()) {
            cout << "imm32 value exceeds limit.\n\tline #" << file_line_idx << " : " << line << endl;
            return -1;
        }

        compose_mov_mdf(m, mcode);

        break;

    case 0b00010:  // RMV
    // intended fall through
    case 0b00011:  // XCT
        compose_rmv_xct(m, mcode);
        break;

    case 0b10101:  // ADDU / SUBU
        if (auto rc = compose_addu_subu(name, m, mcode)) {
            cout << "imm16 value exceeds limit.\n\tline #" << file_line_idx << " : " << line << endl;
            return rc;
        }
        break;

    case 0b10100:  // COPY
        compose_copy(m, mcode);
        break;

    case 0b00111:  // RSM16 / RSM32
        compose_rsm16_rsm32(name, m, mcode);
        break;

    case 0b00101:  // LOCK
    // intended fall through
    case 0b00110:  // ULCK
        if (!m.str(1).empty()) {
            mcode.op_00101.flow_id = stoul(m.str(1), nullptr, 0);
            mcode.op_00101.inline_flg = 1;
        }
        break;

    case 0b00100:  // SHFT
        compose_shft(m, mcode);
        break;

    case 0b01001:  // CSET
        if (auto rc = compose_cset(m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        break;

    case 0b01100:  // HCSUM
    // intended fall through
    case 0b01011:  // HCRC16
    // intended fall through
    case 0b01010:  // HCRC32
        compose_hcsum_hcrc(flags, m, mcode);
        break;

    case 0b01000:  // NOP
    // intended fall through
    case 0b01111:  // PCSUM
    // intended fall through
    case 0b01110:  // PCRC16
    // intended fall through
    case 0b01101:  // PCRC32
        /* code */
        break;

    case 0b10000:  // SNE(U) / SGT(U) / SLT(U) / SGE(U) / SLE(U)
        compose_alu_set(name, flags, m, mcode);
        break;

    case 0b10001:  // NXTH
        if (auto rc = compose_nxth(m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        if (cur_line_idx == 0) {
            init_state = static_cast<std::uint16_t>(mcode.op_10001.sub_state);
        } else {
        // if (cur_line_idx > 0) {
            auto state = static_cast<std::uint16_t>(mcode.op_10001.sub_state);
            auto cur_state = states_seq.back();
            state_line_sub_map.at(cur_state).rbegin()->second = state;
        }
        break;

    case 0b10010:  // NXTD
    // intended fall through
    case 0b10011:  // NXTP
        if (!m.str(1).empty()) {
            mcode.op_10010.shift_val = stoul(m.str(2));
        } {
            auto cur_state = states_seq.back();
            state_line_sub_map.at(cur_state).rbegin()->second = 0xFFFF;
        }
        break;

    default:
        break;
    }

    mcode.universe.last_flg = flags[last_flg_idx];
    pre_last_flag = flags[last_flg_idx];

    if (cur_line_idx == 0) {
        entry_code = mcode.val64;
        mcode_vec.emplace_back(0x8000000000000012);  // in action ram, line 0 always store NXTDL.
    } else if (cur_line_idx > MAX_LINE_NO) {
        cout << "total code lines shall not exceed " << MAX_LINE_NO << endl;
        return -1;
    } else {
        mcode_vec.emplace_back(mcode.val64);
    }

    ++cur_line_idx;

    return 0;
}

void parser_assembler::write_machine_code(void) {
    dst_fstrm.write(reinterpret_cast<const char*>(mcode_vec.data()), sizeof(mcode_vec[0]) * mcode_vec.size());
    dst_fstrm.close();
}

void parser_assembler::print_machine_code(void) {
    print_mcode_line_by_line(dst_fstrm, mcode_vec);
    #ifdef DEBUG
    print_mcode_line_by_line(std::cout, mcode_vec);
    #endif
}

int parser_assembler::process_extra_data(const string &in_fname, const string &ot_fname) {
    auto ot_path = std::filesystem::path(ot_fname).parent_path();
    if (auto rc = output_entry_code(ot_path.string())) {
        return rc;
    }

    if (auto rc = p_tbl->generate_table_data(shared_from_this())) {
        return rc;
    }

    return 0;
}

int parser_assembler::output_entry_code(const string &ot_path) {
    std::ofstream ot_fstrm(ot_path + "/parser_entry_action.dat", std::ios::binary);
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << ot_path << "/parser_entry_action.dat" << std::endl;
        return -1;
    }
    ot_fstrm.write(reinterpret_cast<const char*>(&entry_code), sizeof(entry_code));
    ot_fstrm.close();

    ot_fstrm.open(ot_path + "/parser_entry_action.txt");
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << ot_path << "/parser_entry_action.txt" << std::endl;
        return -1;
    }
    ot_fstrm << std::bitset<64>(entry_code);
    ot_fstrm.close();

    #ifdef DEBUG
    print_extra_data();
    #endif

    return 0;
}

const char* parser_assembler::cmd_name_pattern =
    R"((MOV|MDF|XCT|RMV|ADDU|SUBU|COPY|RSM16|RSM32|LOCK|ULCK|NOP|SHFT|CSET|)"
    R"((HCSUM|HCRC16|HCRC32)(M0)?|PCSUM|PCRC16|PCRC32|(SNE|SGT|SLT|SEQ|SGE|SLE)(U)?|NXTH|NXTP|NXTD)(L)?)";

const char* parser_assembler::state_no_pattern = R"(^#([\d]{3}):(\s+\/\/.*)?[\n\r]?$)";

const int parser_assembler::l_idx = 6;
const int parser_assembler::u_idx = 5;
const int parser_assembler::m0_idx = 3;

const str_u64_map parser_assembler::cmd_opcode_map = {
    {"MOV",    0b00001},
    {"MDF",    0b00001},
    {"XCT",    0b00011},
    {"RMV",    0b00010},
    {"ADDU",   0b10101},
    {"SUBU",   0b10101},
    {"COPY",   0b10100},
    {"RSM16",  0b00111},
    {"RSM32",  0b00111},
    {"LOCK",   0b00101},
    {"ULCK",   0b00110},
    {"NOP",    0b01000},
    {"SHFT",   0b00100},
    {"CSET",   0b01001},
    {"HCSUM",  0b01100},
    {"HCRC16", 0b01011},
    {"HCRC32", 0b01010},
    {"PCSUM",  0b01111},
    {"PCRC16", 0b01110},
    {"PCRC32", 0b01101},
    {"SNE",    0b10000},
    {"SGT",    0b10000},
    {"SLT",    0b10000},
    {"SEQ",    0b10000},
    {"SGE",    0b10000},
    {"SLE",    0b10000},
    {"NXTH",   0b10001},
    {"NXTP",   0b10011},
    {"NXTD",   0b10010}
};

const u64_regex_map parser_assembler::opcode_regex_map = {
    {0b00001, regex(string(g_normal_line_prefix_p) + P_00001 + g_normal_line_posfix_p)},
    {0b00011, regex(string(g_normal_line_prefix_p) + P_00010_00011 + g_normal_line_posfix_p)},
    {0b00010, regex(string(g_normal_line_prefix_p) + P_00010_00011 + g_normal_line_posfix_p)},
    {0b10101, regex(string(g_normal_line_prefix_p) + P_10101 + g_normal_line_posfix_p)},
    {0b10100, regex(string(g_normal_line_prefix_p) + P_10100 + g_normal_line_posfix_p)},
    {0b00111, regex(string(g_normal_line_prefix_p) + P_00111 + g_normal_line_posfix_p)},
    {0b00101, regex(string(g_normal_line_prefix_p) + P_00101_00110 + g_normal_line_posfix_p)},
    {0b00110, regex(string(g_normal_line_prefix_p) + P_00101_00110 + g_normal_line_posfix_p)},
    {0b01000, regex(string(g_normal_line_prefix_p) + P_01000_01101_01110_01111 + g_normal_line_posfix_p)},
    {0b00100, regex(string(g_normal_line_prefix_p) + P_00100 + g_normal_line_posfix_p)},
    {0b01001, regex(string(g_normal_line_prefix_p) + P_01001 + g_normal_line_posfix_p)},
    {0b01100, regex(string(g_normal_line_prefix_p) + P_01010_01011_01100 + g_normal_line_posfix_p)},
    {0b01011, regex(string(g_normal_line_prefix_p) + P_01010_01011_01100 + g_normal_line_posfix_p)},
    {0b01010, regex(string(g_normal_line_prefix_p) + P_01010_01011_01100 + g_normal_line_posfix_p)},
    {0b01111, regex(string(g_normal_line_prefix_p) + P_01000_01101_01110_01111 + g_normal_line_posfix_p)},
    {0b01110, regex(string(g_normal_line_prefix_p) + P_01000_01101_01110_01111 + g_normal_line_posfix_p)},
    {0b01101, regex(string(g_normal_line_prefix_p) + P_01000_01101_01110_01111 + g_normal_line_posfix_p)},
    {0b10000, regex(string(g_normal_line_prefix_p) + P_10000 + g_normal_line_posfix_p)},
    {0b10001, regex(string(g_normal_line_prefix_p) + P_10001 + g_normal_line_posfix_p)},
    {0b10011, regex(string(g_normal_line_prefix_p) + P_10010_10011 + g_normal_line_posfix_p)},
    {0b10010, regex(string(g_normal_line_prefix_p) + P_10010_10011 + g_normal_line_posfix_p)}
};
