/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <limits>
#include <filesystem>
#include <memory>
#include <set>
#include <stack>
#include "../inc/parser_def.h"
#include "../inc/parser_assembler.h"
#if !NO_TBL_PROC
#include "table_proc/match_actionid.h"
#endif

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

constexpr auto END_STATE_NO = static_cast<std::uint16_t>(0xFFFF);
constexpr auto MAX_META_LEN = 1040;

string parser_assembler::name_matched(const smatch &m, vector<bool> &flags) const {
    auto l_flg = !m.str(l_idx).empty();
    auto u_flg = !m.str(u_idx).empty();
    auto m0_flg = !m.str(m0_idx).empty();
    auto name = m.str(1);

    auto rvs32_flg = false;
    auto rvs16_flg = false;
    if (!m.str(rvs_idx).empty()) {
        name = m.str(rvs_idx + 1);
        if (m.str(rvs_idx) == "R16") {
            rvs16_flg = true;
        } else {
            rvs32_flg = true;
        }
    } else if (u_flg) {
        name = m.str(u_idx - 1);
    } else if (m0_flg) {
        name = m.str(m0_idx - 1);
    }

    flags.emplace_back(l_flg);
    flags.emplace_back(u_flg);
    flags.emplace_back(m0_flg);
    flags.emplace_back(rvs16_flg);
    flags.emplace_back(rvs32_flg);

    return name;
}

constexpr auto LAST_FLG_IDX = 0;
constexpr auto UNSIGNED_FLG_IDX = 1;
constexpr auto MASK0_FLG_IDX = 2;
constexpr auto RVS16_FLG_IDX = 3;
constexpr auto RVS32_FLG_IDX = 4;
constexpr auto MATCH_STATE_NO_LINE_FLG_IDX = 5;  // this flag must always be the last one in flags vector
constexpr auto FLAGS_SZ = 6;

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

static inline void compose_rmv_xct(const smatch &m, const machine_code &code, const vector<bool> &flags) {
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

    if (flags[RVS32_FLG_IDX]) {
        mcode.op_00010.byte_rvs_mode = 1;
    } else if (flags[RVS16_FLG_IDX]) {
        mcode.op_00010.byte_rvs_mode = 2;
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
        if (!m.str(2).empty()) {
            auto idx = stoul(m.str(2));
            if (idx <= 3) {
                mcode.op_10100.src_slct = idx + 0b100;
            } else {
                mcode.op_10100.src_slct = idx + 0b1000;
            }
        } else {
            auto idx = stoul(m.str(3));
            mcode.op_10100.src_slct = idx + 0b1000;
        }
    }
    if (m.str(4) == "TMP") {
        mcode.op_10100.dst_slct = 2;
    } else {
        if (!m.str(5).empty()) {  // META
            mcode.op_10100.dst_slct = 1;
            mcode.op_10100.dst_off = stoul(m.str(6));
            mcode.op_10100.dst_len = stoul(m.str(7)) - 1;
        } else {  // PHV
            mcode.op_10100.dst_off = stoul(m.str(8));
            mcode.op_10100.dst_len = stoul(m.str(9)) - 1;
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

static inline bool phv_length_valid(std::uint32_t length, std::uint32_t phv_len) {
    return length <= phv_len;
}

static inline int compose_shft(const smatch &m, const machine_code &code) {
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
            mcode.op_00100.length = stoul(m.str(4)) - 1;
            if (m.str(5) == "CSUM") {
                mcode.op_00100.calc_slct = 1;
            } else if (m.str(5) == "CRC16") {
                mcode.op_00100.calc_slct = 2;
            } else if (m.str(5) == "CRC32") {
                mcode.op_00100.calc_slct = 3;
            }
        } else if (!m.str(6).empty()) {
            if (!phv_length_valid(stoul(m.str(6)), (512 - stoul(m.str(7))) * 8)) {
                return -1;
            }
            mcode.op_00100.inline_flg = 1;
            mcode.op_00100.length = stoul(m.str(6)) - 1;
            mcode.op_00100.phv_flg = 1;
            mcode.op_00100.phv_off = stoul(m.str(7));
        } else if (!m.str(8).empty()) {
            if (!phv_length_valid(stoul(m.str(8)), (512 - stoul(m.str(10))) * 8)) {
                return -1;
            }
            mcode.op_00100.inline_flg = 1;
            mcode.op_00100.length = stoul(m.str(8)) - 1;
            if (m.str(9) == "CSUM") {
                mcode.op_00100.calc_slct = 1;
            } else if (m.str(9) == "CRC16") {
                mcode.op_00100.calc_slct = 2;
            } else if (m.str(9) == "CRC32") {
                mcode.op_00100.calc_slct = 3;
            }
            mcode.op_00100.phv_flg = 1;
            mcode.op_00100.phv_off = stoul(m.str(10));
        }
    }
    return 0;
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

static inline int check_offset_length(std::int16_t offset, std::int16_t length, std::int16_t valid_len) {
    return offset + length >= valid_len ? -1 : 0;
}
static inline int compose_hcsum_hcrc(const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!flags[MASK0_FLG_IDX]) {
        mcode.op_01100.mask_flg = 1;
    }
    mcode.op_01100.mask = stoul(m.str(6), nullptr, 0);
    mcode.op_01100.length = stoul(m.str(5)) - 1;
    if (!m.str(2).empty()) {
        mcode.op_01100.src_slct = 2;
        auto offset = stoul(m.str(2));
        mcode.op_01100.offset = offset;
        mcode.op_01100.phv_slct = offset >> 7;
        return check_offset_length(mcode.op_01100.offset * 8, mcode.op_01100.length, 1024);
    } else {
        if (m.str(3) == "TMP") {
            mcode.op_01100.src_slct = 1;
        }
        mcode.op_01100.offset = stoul(m.str(4));
        return check_offset_length(mcode.op_01100.offset, mcode.op_01100.length, 32);
    }
}

static inline int compose_alu_set(
    const string &name, const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (flags[LAST_FLG_IDX]) {
        return -1;
    }
    if (!flags[UNSIGNED_FLG_IDX]) {
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
    return 0;
}

static inline int compose_nxth(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!m.str(1).empty()) {
        mcode.op_10001.isr_off = stoul(m.str(2));
        mcode.op_10001.isr_len = stoul(m.str(3));
        if (mcode.op_10001.isr_off + mcode.op_10001.isr_len > 32) {
            return -1;
        }
    }
    if (!m.str(4).empty()) {
        mcode.op_10001.meta_off = stoul(m.str(5));
        mcode.op_10001.meta_len = stoul(m.str(6));
        mcode.op_10001.meta_intrinsic = 1;
        if (mcode.op_10001.meta_off + mcode.op_10001.meta_len > MAX_META_LEN) {
            return -1;
        }
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

static inline int compose_nxtp(const smatch &m, const vector<bool> &flags, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!flags[LAST_FLG_IDX]) {
        return -1;
    }
    if (!m.str(4).empty() && m.str(5).empty()) {
        return -1;
    }
    if (!m.str(1).empty()) {
        mcode.op_10011.dest_phv_off = stoul(m.str(2));
        mcode.op_10011.ignored_byte_cnt = stoul(m.str(3));
        mcode.op_10011.not_final_flg = 1;
    }
    if (!m.str(5).empty()) {
        mcode.op_10010.shift_val = stoul(m.str(6));
    }
    return 0;
}

int parser_assembler::process_state_no_line(const string &line, const string &pattern) {
    const regex r(pattern);
    smatch m;
    if (!regex_match(line, m, r)) {
        cout << "state no line #" << src_file_line_idx() << " match error.\n\t" << line << endl;
        return -1;
    }
    auto state_no = stoul(m.str(1));
    if (state_no > MAX_STATE_NO) {
        cout << "state no shall not exceed " << MAX_STATE_NO << ".\n\t" << line << endl;
        return -1;
    }
    states_seq.emplace_back(state_no);
    state_line_sub_map.emplace(std::make_pair(state_no, map_of_u16()));
    state_line_sub_map.at(state_no).emplace(std::make_pair(cur_line_idx, END_STATE_NO));
    if (state_line_sub_map.at(state_no).size() > MATCH_ENTRY_CNT_PER_STATE) {
        cout << "for each state, the count of sub state shall not exceed " << MATCH_ENTRY_CNT_PER_STATE << endl;
        return -1;
    }
    pre_last_flag = false;
    return 0;
}

int parser_assembler::line_process(const string &line, const string &name, const vector<bool> &flags) {
    if (flags.size() != FLAGS_SZ) {
        return -1;
    }

    if (flags[MATCH_STATE_NO_LINE_FLG_IDX]) {
        return process_state_no_line(line, assist_line_pattern());
    }

    if (pre_last_flag && !states_seq.empty()) {
        auto cur_state = states_seq.back();
        state_line_sub_map.at(cur_state).emplace(std::make_pair(cur_line_idx, END_STATE_NO));
    }

    machine_code mcode;
    if (!cmd_opcode_map.count(name)) {
        std::cout << "ERROR: line #" << src_file_line_idx() << "\n\t";
        std::cout << "no opcode match this instruction name - " << name << std::endl;
        return -1;
    }
    mcode.val64 = cmd_opcode_map.at(name);

    smatch m;
    if (!regex_match(line, m, opcode_regex_map.at(mcode.val64))) {
        print_line_unmatch_message(line);
        return -1;
    }

    if (cur_line_idx == 0 && (mcode.universe.opcode != 0b10001 || !flags[LAST_FLG_IDX])) {
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
            cout << "imm32 value exceeds limit.\n\tline #" << src_file_line_idx() << " : " << line << endl;
            return -1;
        }

        compose_mov_mdf(m, mcode);

        break;

    case 0b00010:  // RMV / RVS32_RMV / RVS16_RMV
    // intended fall through
    case 0b00011:  // XCT / RVS32_XCT / RVS16_XCT
        compose_rmv_xct(m, mcode, flags);
        break;

    case 0b10101:  // ADDU / SUBU
        if (auto rc = compose_addu_subu(name, m, mcode)) {
            cout << "imm16 value exceeds limit.\n\tline #" << src_file_line_idx() << " : " << line << endl;
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
        if (auto rc = compose_shft(m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
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
        if (auto rc = compose_hcsum_hcrc(flags, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
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
        if (auto rc = compose_alu_set(name, flags, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
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
        if (!flags[LAST_FLG_IDX]) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        if (!m.str(1).empty()) {
            mcode.op_10010.shift_val = stoul(m.str(2));
        } {
            auto cur_state = states_seq.back();
            state_line_sub_map.at(cur_state).rbegin()->second = END_STATE_NO;
        }
        break;

    case 0b10011:  // NXTP
        if (auto rc = compose_nxtp(m, flags, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        } {
            auto cur_state = states_seq.back();
            state_line_sub_map.at(cur_state).rbegin()->second = END_STATE_NO;
        }
        break;

    default:
        break;
    }

    mcode.universe.last_flg = flags[LAST_FLG_IDX];
    pre_last_flag = flags[LAST_FLG_IDX];

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
    if (auto rc = output_entry_code(ot_fname)) {
        return rc;
    }
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

int parser_assembler::output_entry_code(const string &ot_path) {
    auto ot_fname = ot_path + "_entry";
    std::ofstream ot_fstrm(ot_fname + ".dat", std::ios::binary);
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << ot_fname + ".dat" << std::endl;
        return -1;
    }
    ot_fstrm.write(reinterpret_cast<const char*>(&entry_code), sizeof(entry_code));
    ot_fstrm.close();

    ot_fstrm.open(ot_fname + ".txt");
    if (!ot_fstrm.is_open()) {
        std::cout << "cannot open file: " << ot_fname + ".txt" << std::endl;
        return -1;
    }
    ot_fstrm << std::bitset<64>(entry_code);
    ot_fstrm.close();

    #ifdef DEBUG
    print_extra_data();
    #endif

    return 0;
}

bool parser_assembler::state_chart_has_loop() {
    auto state_line_sub_set = std::set<std::tuple<std::uint16_t, std::uint16_t, std::uint16_t>>();
    auto state_line_sub_stk = std::stack<std::tuple<std::uint16_t, std::uint16_t, std::uint16_t>>();
    auto reach_end = false;

    for (auto state = init_state; ;) {
        for (const auto &line_sub : state_line_sub_map.at(state)) {
            if (line_sub.second == END_STATE_NO) {
                reach_end = true;
                break;
            }
            auto state_line_sub = std::make_tuple(state, line_sub.first, line_sub.second);
            if (state_line_sub_set.count(state_line_sub)) {
                return true;
            }
            state_line_sub_set.emplace(state_line_sub);
            state_line_sub_stk.emplace(state_line_sub);
        }

        if (reach_end) {
            while (!state_line_sub_stk.empty() && state == std::get<2>(state_line_sub_stk.top())) {
                state = std::get<0>(state_line_sub_stk.top());
                state_line_sub_set.erase(state_line_sub_stk.top());
                state_line_sub_stk.pop();
            }
            reach_end = false;
        }

        if (state_line_sub_stk.empty()) {
            break;
        }

        state = std::get<2>(state_line_sub_stk.top());
    }

    return false;
}

const char* parser_assembler::cmd_name_pattern =
    R"((MOV|MDF|(R32|R16)?(XCT|RMV)|ADDU|SUBU|COPY|RSM16|RSM32|LOCK|ULCK|NOP|SHFT|CSET|)"
    R"((HCSUM|HCRC16|HCRC32)(M0)?|PCSUM|PCRC16|PCRC32|(SNE|SGT|SLT|SEQ|SGE|SLE)(U)?|NXTH|NXTP|NXTD)(L)?)";

const char* parser_assembler::stateno_pattern = R"(#([\d]{3}):)";

const int parser_assembler::l_idx = 8;
const int parser_assembler::u_idx = 7;
const int parser_assembler::m0_idx = 5;
const int parser_assembler::rvs_idx = 2;

const str_u64_map parser_assembler::cmd_opcode_map = {
    {"MOV",       0b00001},
    {"MDF",       0b00001},
    {"XCT",       0b00011},
    {"RMV",       0b00010},
    {"ADDU",      0b10101},
    {"SUBU",      0b10101},
    {"COPY",      0b10100},
    {"RSM16",     0b00111},
    {"RSM32",     0b00111},
    {"LOCK",      0b00101},
    {"ULCK",      0b00110},
    {"NOP",       0b01000},
    {"SHFT",      0b00100},
    {"CSET",      0b01001},
    {"HCSUM",     0b01100},
    {"HCRC16",    0b01011},
    {"HCRC32",    0b01010},
    {"PCSUM",     0b01111},
    {"PCRC16",    0b01110},
    {"PCRC32",    0b01101},
    {"SNE",       0b10000},
    {"SGT",       0b10000},
    {"SLT",       0b10000},
    {"SEQ",       0b10000},
    {"SGE",       0b10000},
    {"SLE",       0b10000},
    {"NXTH",      0b10001},
    {"NXTP",      0b10011},
    {"NXTD",      0b10010}
};

const u64_regex_map parser_assembler::opcode_regex_map = {
    {0b00001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00010_00011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00010_00011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10101 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00111, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00101_00110 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00110, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00101_00110 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01000, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01000_01101_01110_01111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01010_01011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01010_01011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01111, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01000_01101_01110_01111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01110, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01000_01101_01110_01111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01000_01101_01110_01111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10000, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10000 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10010 + INSTRUCTION_LINE_POSFIX_P)}
};
