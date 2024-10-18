/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <limits>
#include <filesystem>
#include "../inc/deparser_assembler.h"
#include "../inc/deparser_def.h"
#if !NO_TBL_PROC
#include "table_proc/mask_table.h"
#endif

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

constexpr auto MAX_LINE_CNT = 1024UL;
constexpr auto MAX_LINE_PER_FILE = 256UL;
constexpr auto MAX_LINE_PER_BLOCK = 32UL;
constexpr auto MAX_FILE_CNT = 4UL;
constexpr auto MAX_BLOCK_CNT = 8UL;
constexpr auto NOP_MCODE = 0b00000000000000000000000000011000UL;

string deparser_assembler::name_matched(const smatch &m, vector<bool> &flags) const {
    auto name = m.str(1);

    auto sndm_m_flg = false;
    auto sndm_p_flg = false;
    if (!m.str(sndm_flg_idx).empty()) {
        sndm_m_flg = m.str(sndm_flg_idx) == "M";
        sndm_p_flg = m.str(sndm_flg_idx) == "P";
    }
    flags.emplace_back(sndm_m_flg);
    flags.emplace_back(sndm_p_flg);

    auto calc_ma_flg = false;
    auto calc_mo_flg = false;
    if (!m.str(calc_flg_idx).empty()) {
        calc_ma_flg = m.str(calc_flg_idx) == "MA";
        calc_mo_flg = m.str(calc_flg_idx) == "MO";
        name.pop_back();
        name.pop_back();
    }
    flags.emplace_back(calc_ma_flg);
    flags.emplace_back(calc_mo_flg);

    // auto xor_4_flg = false;
    auto xor_8_flg = false;
    auto xor_16_flg = false;
    auto xor_32_flg = false;
    if (!m.str(xor_flg_idx).empty()) {
        // xor_4_flg = m.str(xor_flg_idx) == "4";
        xor_8_flg = m.str(xor_flg_idx) == "8";
        xor_16_flg = m.str(xor_flg_idx) == "16";
        xor_32_flg = m.str(xor_flg_idx) == "32";
        name.pop_back();
    }
    // flags.emplace_back(xor_4_flg);
    flags.emplace_back(xor_8_flg);
    flags.emplace_back(xor_16_flg);
    flags.emplace_back(xor_32_flg);

    auto j_r_flg = !m.str(j_flg_idx).empty();
    flags.emplace_back(j_r_flg);

    if (sndm_m_flg || sndm_p_flg || j_r_flg || xor_16_flg || xor_32_flg) {
        name.pop_back();
    }

    return name;
}

constexpr auto SENDM_MASK_FLG_IDX = 0;
constexpr auto SENDM_PIPELINE_FLG_IDX = 1;
constexpr auto CALC_MASK_AND_FLG_IDX = 2;
constexpr auto CALC_MASK_OR_FLG_IDX = 3;
constexpr auto XOR8_FLG_IDX = 4;
constexpr auto XOR16_FLG_IDX = 5;
constexpr auto XOR32_FLG_IDX = 6;
constexpr auto JUMP_RELATIVE_FLG_IDX = 7;
constexpr auto MATCH_ASSIT_LINE_FLG_IDX = 8;
constexpr auto FLAGS_SZ = 9;

static inline void print_cmd_bit_vld_unmatch_message(const string &line) {
    std::cout << line + "\n\tshall be following a <MSKALL / MSKADDR> line." << std::endl;
}

static inline void compose_sndm(const vector<bool> &flags, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (flags[SENDM_MASK_FLG_IDX]) {
        mcode.op_00001.src_slct = 2;
    } else if (!flags[SENDM_PIPELINE_FLG_IDX]) {
        mcode.op_00001.src_slct = 1;
    }
}

static inline int compose_sndh_sndp_sndpc(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1).empty() && m.str(2).empty()) {
        mcode.op_00010.src_slct = 3;
    } else {
        if (m.str(1) == "PLD") {
            mcode.op_00010.src_slct = 2;
        } else if (m.str(1) == "HDR") {
            mcode.op_00010.src_slct = 1;
        }
        if (m.str(2).empty()) {
            mcode.op_00010.off_ctrl = 1;
            mcode.op_00010.len_ctrl = 1;
        } else {
            if (m.str(3).empty() && m.str(4).empty()) {
                return -1;
            }
            if (m.str(3).empty()) {
                mcode.op_00010.off_ctrl = 1;
            } else {
                mcode.op_00010.offset = stoul(m.str(3));
            }
            if (m.str(4).empty()) {
                mcode.op_00010.len_ctrl = 1;
            } else {
                mcode.op_00010.length = stoul(m.str(4));
            }
        }
    }
    if ((!m.str(1).empty() || !m.str(2).empty()) && m.str(5).empty() && !m.str(6).empty()) {
        return -1;
    }
    if (m.str(6) == "CSUM") {
        mcode.op_00011.calc_mode = 1;
    } else if (m.str(6) == "CRC16") {
        mcode.op_00011.calc_mode = 2;
    } else if (m.str(6) == "CRC32") {
        mcode.op_00011.calc_mode = 3;
    }
    return 0;
}

static inline int compose_seth_setl(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "COND") {
        mcode.op_00110.dst_slct = 1;
    } else if (m.str(1) == "POFF") {
        mcode.op_00110.dst_slct = 2;
    } else if (m.str(1) == "PLEN") {
        mcode.op_00110.dst_slct = 3;
    } else if (m.str(1) == "POLY") {
        mcode.op_00110.dst_slct = 4;
    } else if (m.str(1) == "INIT") {
        mcode.op_00110.dst_slct = 5;
    } else if (m.str(1) == "XOROT") {
        mcode.op_00110.dst_slct = 7;
    } else if (m.str(1) != "TMP") {  // "CTRL"
        mcode.op_00110.dst_slct = 6;
        mcode.op_00110.ctrl_mode = stoul(m.str(2), nullptr, 2);
    }
    if (stoul(m.str(3), nullptr, 0) > std::numeric_limits<std::uint16_t>::max()) {
        return -1;
    }
    mcode.op_00110.value = stoul(m.str(3), nullptr, 0);  // heximal or decimal
    return 0;
}

static inline int compose_add_addu(const std::string &name, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "COND") {
        mcode.op_01000.src_slct = 1;
    } else if (m.str(1) == "POFF") {
        mcode.op_01000.src_slct = 2;
    } else if (m.str(1) == "PLEN") {
        mcode.op_01000.src_slct = 3;
    } else {
        mcode.op_01000.src_off = stoul(m.str(2));
        mcode.op_01000.src_len = stoul(m.str(3));
        if ((name == "ADDT" || name == "ADDTU") && !m.str(4).empty()) {
            return -1;
        }
        if ((name == "ADD" || name == "ADDU") && m.str(4).empty()) {
            return -1;
        }
        if (!m.str(4).empty()) {
            mcode.op_01000.dst_off = stoul(m.str(5));
            mcode.op_01000.dst_len = stoul(m.str(6));
        } else {
            mcode.op_01000.dst_len = 0b1111;
        }
        if (mcode.op_01000.dst_len <= mcode.op_01000.src_len) {
            return -1;
        }
    }
    return 0;
}

static inline void compose_cmpct_and_or(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_01010.src_off = stoul(m.str(1));
    mcode.op_01010.src_len = stoul(m.str(2));
    mcode.op_01010.dst_off = stoul(m.str(3));
}

static inline void compose_cmpctr_andr_orr(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!m.str(1).empty()) {
        mcode.op_01011.dst_off = stoul(m.str(2));
    } else {
        mcode.op_01011.dst_slct = 1;
    }
}

static inline int compose_crc16_crc32_csum(
    const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "PLD") {
        mcode.op_01110.src_slct = 2;
    } else if (m.str(1) == "HDR") {
        mcode.op_01110.src_slct = 1;
    }
    if (m.str(2).empty()) {
        mcode.op_01110.off_ctrl = 1;
        mcode.op_01110.len_ctrl = 1;
    } else {
        if (m.str(3).empty() && m.str(4).empty()) {
            return -1;
        }
        if (m.str(3).empty()) {
            mcode.op_01110.off_ctrl = 1;
        } else {
            mcode.op_01110.offset = stoul(m.str(3));
        }
        if (m.str(4).empty()) {
            mcode.op_01110.len_ctrl = 1;
        } else {
            mcode.op_01110.length = stoul(m.str(4));
        }
    }
    if (flags[CALC_MASK_AND_FLG_IDX] || flags[CALC_MASK_OR_FLG_IDX]) {
        mcode.op_01110.mask_en = 1;
        if (flags[CALC_MASK_OR_FLG_IDX]) {
            mcode.op_01110.mask_flg = 1;
        }
    }
    return 0;
}

static inline void compose_xor(const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_10001.src_off = stoul(m.str(1));
    mcode.op_10001.src_len = stoul(m.str(2));
    mcode.op_10001.calc_unit = assembler::xor_unit(flags[XOR8_FLG_IDX], flags[XOR16_FLG_IDX], flags[XOR32_FLG_IDX]);
    mcode.op_10001.dst_slct = m.str(3) == "PHV";
    mcode.op_10001.dst_off = stoul(m.str(4));
}

static inline void compose_xorr(const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_10010.calc_unit = assembler::xor_unit(flags[XOR8_FLG_IDX], flags[XOR16_FLG_IDX], flags[XOR32_FLG_IDX]);
    mcode.op_10010.dst_slct = m.str(1) == "PHV";
    mcode.op_10010.dst_off = stoul(m.str(2));
}

static inline void compose_hash(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_10011.src_off = stoul(m.str(1));
    mcode.op_10011.src_len = stoul(m.str(2));
    mcode.op_10011.dst_slct = m.str(3) == "PHV";
    mcode.op_10011.dst_off = stoul(m.str(4));
}

static inline void compose_hashr(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_10100.dst_slct = m.str(1) == "PHV";
    mcode.op_10100.dst_off = stoul(m.str(2));
}

static inline int compose_counter_ops(const string &name, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (!m.str(10).empty()) {
        if (m.str(11) == "LCK") {
            mcode.op_10101.lck_flg = 1;
        } else {  // if (m.str(11) == "ULK")
            mcode.op_10101.ulk_flg = 1;
        }
    }
    if (name == "LDC") {
        if (!m.str(3).empty()) {
            return -1;
        }
        if (!m.str(9).empty()) {  // from TMP
            mcode.op_10101.cmd_type = 5;
        } else {  // from PHV
            mcode.op_10101.cmd_type = 4;
        }
    } else if (name == "GET--") {
        mcode.op_10101.cmd_type = 3;
    } else if (name == "--GET") {
        mcode.op_10101.cmd_type = 2;
    } else if (name == "GET++") {
        mcode.op_10101.cmd_type = 1;
    }
    if (!m.str(6).empty()) {
        mcode.op_10101.phv_off = stoul(m.str(7));
        mcode.op_10101.cntr_width = stoul(m.str(8)) >> 6;
    }
    if (m.str(1).empty()) {
        mcode.op_10101.cntr_idx_slct = 1;
    } else {
        mcode.op_10101.cntr_idx = stoul(m.str(2));
    }
    if (m.str(3).empty()) {
        mcode.op_10101.imm_slct = 1;
    } else {
        mcode.op_10101.imm8 = stoul(m.str(4));
    }
    return 0;
}

static inline int compose_mskall_mskaddr(const string &name, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (name == "MSKALL") {
        if (!m.str(1).empty()) {
            return -1;
        }
    } else if (!m.str(1).empty()) {
        mcode.op_10111.select = 1;
        mcode.op_10111.address = stoul(m.str(1), nullptr, 0);
    } else {
        mcode.op_10111.select = 2;
    }
    return 0;
}

static inline int compose_j_bez(
    const string &name, const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if ((name == "J" && !m.str(3).empty())) {
        return -1;
    }
    if (!flags[JUMP_RELATIVE_FLG_IDX] && (m.str(2) == "-" || m.str(5) == "-")) {
        return -1;
    }
    mcode.op_11001.target_1 =
        flags[JUMP_RELATIVE_FLG_IDX] ? std::stol(m.str(1), nullptr, 0) : stoul(m.str(1), nullptr, 0);
    if (!m.str(4).empty()) {
        mcode.op_11001.target_2 =
            flags[JUMP_RELATIVE_FLG_IDX] ? std::stol(m.str(4), nullptr, 0) : stoul(m.str(4), nullptr, 0);
    }
    if (!m.str(6).empty() && m.str(6) != "CONDR") {
        mcode.op_11001.src_slct = 1;
        auto src_off = stoul(m.str(8));
        if (m.str(7) == "8") {
            if (src_off % 8) {
                return -1;
            }
            mcode.op_11001.src_slct = 2;
        } else if (m.str(7) == "16") {
            if (src_off % 16) {
                return -1;
            }
            mcode.op_11001.src_slct = 3;
        } else if (m.str(7) == "32") {
            if (src_off % 32) {
                return -1;
            }
            mcode.op_11001.src_slct = 6;
        }
        mcode.op_11001.src_off = src_off;
    }
    if (name == "J") {
        mcode.op_11001.jump_mode = flags[JUMP_RELATIVE_FLG_IDX] ? 1 : 0;
    } else if (name == "BEZ") {
        mcode.op_11001.jump_mode = flags[JUMP_RELATIVE_FLG_IDX] ? 0b11 : 0b10;
    }
    return 0;
}

static inline void compose_copy(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1) == "META" && m.str(4) == "META") {
        mcode.op_10110.direction = 3;
    } else if (m.str(1) == "META" && m.str(4) == "PHV") {
        mcode.op_10110.direction = 2;
    } else if (m.str(1) == "PHV" && m.str(4) == "META") {
        mcode.op_10110.direction = 1;
    }
    mcode.op_10110.src_off = stoul(m.str(2));
    mcode.op_10110.length = stoul(m.str(3)) - 1;
    mcode.op_10110.dst_off = stoul(m.str(5));
}

int deparser_assembler::line_process(const string &line, const string &name, const vector<bool> &flags) {
    if (flags.size() != FLAGS_SZ) {
        return -1;
    }

    if (auto rc = align_mcode_per_file_block()) {
        return rc;
    }

    machine_code mcode;
    mcode.val64 = 0;
    if (!cmd_opcode_map.count(name)) {
        std::cout << "ERROR: line #" << src_file_line_idx() << "\n\t";
        std::cout << "no opcode match this instruction name - " << name << std::endl;
        return -1;
    }
    mcode.val32 = cmd_opcode_map.at(name);
    auto opcode = mcode.val32;

    smatch m;
    if (!regex_match(line, m, opcode_regex_map.at(mcode.val32))) {
        print_line_unmatch_message(line);
        return -1;
    }

    switch (opcode) {
    case 0b00001:  // SNDM[PM]
        compose_sndm(flags, mcode);
        if (flags[SENDM_MASK_FLG_IDX]) {
            if (previous_not_mask(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return -1;
            }
            swap_previous(mcode.val32);
        }
        break;

    case 0b00010:  // SNDH
    case 0b00011:  // SNDP
    case 0b00100:  // SNDPC
        if (auto rc = compose_sndh_sndp_sndpc(m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        if (name == "SNDH" && mcode.op_00010.src_slct == 0) {
            if (previous_not_mask(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return -1;
            }
            swap_previous(mcode.val32);
        }
        break;

    case 0b00101:  // MOVE
        mcode.op_00101.offset = std::stoul(m.str(1));
        mcode.op_00101.length = std::stoul(m.str(2)) - 1;
        break;

    case 0b00110:  // SETH
    case 0b00111:  // SETL
        if (auto rc = compose_seth_setl(m, mcode)) {
            cout << "imm16 value exceeds limit.\n\t" << line << endl;
            return rc;
        }
        break;

    case 0b01000:  // ADDU / ADDTU
    case 0b01001:  // ADD / ADDT
        if (auto rc = compose_add_addu(name, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        break;

    case 0b01010:  // CMPCT
    case 0b01100:  // AND
    case 0b11100:  // OR
        compose_cmpct_and_or(m, mcode);
        if (previous_not_mask(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return -1;
        }
        swap_previous(mcode.val32);
        break;

    case 0b01011:  // CMPCTR
    case 0b01101:  // ANDR
    case 0b11101:  // ORR
        compose_cmpctr_andr_orr(m, mcode);
        if (previous_not_mask(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return -1;
        }
        swap_previous(mcode.val32);
        break;

    case 0b01110:  // CRC16
    case 0b01111:  // CRC32
    case 0b10000:  // CSUM
        if (auto rc = compose_crc16_crc32_csum(flags, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        if (flags[CALC_MASK_AND_FLG_IDX] || flags[CALC_MASK_OR_FLG_IDX]) {
            if (previous_not_mask(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return -1;
            }
            swap_previous(mcode.val32);
        }
        break;

    case 0b10001:  // XOR
        compose_xor(flags, m , mcode);
        break;

    case 0b10010:  // XORR
        compose_xorr(flags, m , mcode);
        break;

    case 0b10011:  // HASH
        compose_hash(m , mcode);
        if (previous_not_mask(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return -1;
        }
        swap_previous(mcode.val32);
        break;

    case 0b10100:  // HASHR
        compose_hashr(m , mcode);
        if (previous_not_mask(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return -1;
        }
        swap_previous(mcode.val32);
        break;

    case 0b10101:  // ++GET / GET++ / --GET / GET-- / LDC
        if (auto rc = compose_counter_ops(name, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        break;

    case 0b10110:  // COPY
        compose_copy(m , mcode);
        break;

    case 0b10111:  // MSKALL / MSKADDR
        if (auto rc = compose_mskall_mskaddr(name, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        #if !NO_TBL_PROC
        be_mask_table_necessary = name == "MSKADDR";
        #endif
        break;

    case 0b11001:  // J / BEZ
        if (auto rc = compose_j_bez(name, flags, m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        break;

    case 0b11000:  // NOP
    case 0b11010:  // RET
    case 0b11011:  // END
        break;

    default:
        break;
    }

    if (mcode_vec.size() >= MAX_LINE_CNT) {
        std::cout << "ERROR: code lines exceed " << MAX_LINE_CNT << std::endl;
        return -1;
    }

    mcode_vec.emplace_back(mcode.val32);
    std::uint32_t high32 = mcode.val64 >> 32;
    if (opcode == 0b11001 || opcode == 0b10101) {
        mcode_vec.emplace_back(high32);
    }

    prev_line_name = name;

    return 0;
}

void deparser_assembler::write_machine_code(void) {
    dst_fstrm.write(reinterpret_cast<const char*>(mcode_vec.data()), sizeof(mcode_vec[0]) * mcode_vec.size());
}

void deparser_assembler::print_machine_code(void) {
    print_mcode_line_by_line(dst_fstrm, mcode_vec);
    #ifdef DEBUG
    print_mcode_line_by_line(std::cout, mcode_vec);
    #endif
}

int deparser_assembler::process_extra_data(const string &in_fname, const string &ot_fname) {
    #if !NO_TBL_PROC
    if (be_mask_table_necessary) {
        if (p_tbl == nullptr) {
            std::cout << "ERROR: table class instantiation failed." << std::endl;
            return -1;
        }
        if (auto rc = p_tbl->generate_table_data(shared_from_this())) {
            return rc;
        }
    }
    #endif
    (void)ot_fname;
    return 0;
}

#if !NO_PRE_PROC
inline void deparser_assembler::print_code_file_exceed_message() {
    std::cout << "ERROR: one deparser source file shall contain " << MAX_LINE_PER_FILE;
    std::cout << " lines of code at most.\n\t";
    std::cout << cur_fname << std::endl;
}

inline void deparser_assembler::print_code_block_exceed_message() {
    std::cout << "ERROR: one deparser source code block shall contain " << MAX_LINE_PER_BLOCK;
    std::cout << " lines of code at most.\n\t";
    std::cout << cur_fname << std::endl;
}

int deparser_assembler::inc_file_line_idx() {
    if (cur_fname_2_line_idx[cur_fname] >= MAX_LINE_PER_FILE) {
        print_code_file_exceed_message();
        return -1;
    }
    ++cur_fname_2_line_idx[cur_fname];
    if (cmd_opcode_map.at(prev_line_name) == 0b10101 || cmd_opcode_map.at(prev_line_name) == 0b11001) {
        if (cur_fname_2_line_idx[cur_fname] >= MAX_LINE_PER_FILE) {
            print_code_file_exceed_message();
            return -1;
        }
        ++cur_fname_2_line_idx[cur_fname];
    }
    return 0;
}

int deparser_assembler::inc_block_line_idx() {
    if (block_line_vec.back() >= MAX_LINE_PER_BLOCK) {
        print_code_block_exceed_message();
        return -1;
    }
    ++block_line_vec.back();
    if (cmd_opcode_map.at(prev_line_name) == 0b10101 || cmd_opcode_map.at(prev_line_name) == 0b11001) {
        if (block_line_vec.back() >= MAX_LINE_PER_BLOCK) {
            print_code_block_exceed_message();
            return -1;
        }
        ++block_line_vec.back();
    }
    return 0;
}

int deparser_assembler::switch_file() {
    if (cur_fname_2_line_idx.size() >= MAX_FILE_CNT) {
        std::cout << "ERROR: deparser source code could be splitted into " << MAX_FILE_CNT;
        std::cout << " files at most." << std::endl;
        return -1;
    }
    auto cur_fline = cur_fname_2_line_idx[cur_fname];
    for (auto i = 0UL; i < MAX_LINE_PER_FILE - cur_fline; ++i) {
        mcode_vec.emplace_back(NOP_MCODE);
        cur_fname_2_line_idx[cur_fname];
    }
    cur_fname = src_file_name();
    cur_fname_2_line_idx.emplace(cur_fname, 1);
    block_line_vec.clear();
    block_line_vec.emplace_back(1);
    return 0;
}

int deparser_assembler::switch_block() {
    if (block_line_vec.size() >= MAX_BLOCK_CNT) {
        std::cout << "ERROR: deparser source file could be splitted into " << MAX_BLOCK_CNT;
        std::cout << " blocks at most." << std::endl;
        return -1;
    }
    for (auto i = 0UL; i < MAX_LINE_PER_BLOCK - block_line_vec.back(); ++i) {
        mcode_vec.emplace_back(NOP_MCODE);
        ++cur_fname_2_line_idx[cur_fname];
    }
    block_line_vec.emplace_back(1);
    return 0;
}
#endif

int deparser_assembler::align_mcode_per_file_block() {
    #if !NO_PRE_PROC
    if (cur_fname.empty()) {
        cur_fname = src_file_name();
        cur_fname_2_line_idx.emplace(cur_fname, 1);
        block_line_vec.emplace_back(1);
    } else if (cur_fname != src_file_name()) {
        return switch_file();
    } else {
        if (auto rc = inc_file_line_idx()) {
            return rc;
        }
        if (prev_line_name == "END") {
            return switch_block();
        } else {
            return inc_block_line_idx();
        }
    }
    #endif
    return 0;
}

inline bool deparser_assembler::previous_not_mask(const string &line) const {
    return (prev_line_name != "MSKALL" && prev_line_name != "MSKADDR");
}

inline void deparser_assembler::swap_previous(const std::uint32_t &mcode) {
    auto val = mcode_vec.back();
    mcode_vec.pop_back();
    mcode_vec.emplace_back(mcode);
    (const_cast<std::uint32_t&>(mcode)) = val;
}

const char* deparser_assembler::cmd_name_pattern =
    R"((SNDM([PM])?|SND[HP]C?|MOVE|SET[HL]|ADDT?U?|CMPCTR?|ANDR?|ORR?|(CRC16|CRC32|CSUM)(M[AO])?|)"
    R"(XORR?(4|8|16|32)|HASHR?|[+-]{2}GET|GET[+-]{2}|LDC|COPY|MSKALL|MSKADDR|NOP|(J|BEZ)(R)?|RET|END))";

const int deparser_assembler::sndm_flg_idx = 2;
const int deparser_assembler::calc_flg_idx = 4;
const int deparser_assembler::xor_flg_idx = 5;
const int deparser_assembler::j_flg_idx = 7;

const str_u32_map deparser_assembler::cmd_opcode_map = {
    {"SNDM",     0b00001},
    {"SNDH",     0b00010},
    {"SNDP",     0b00011},
    {"SNDPC",    0b00100},
    {"MOVE",     0b00101},
    {"SETH",     0b00110},
    {"SETL",     0b00111},
    {"ADDU",     0b01000},
    {"ADDTU",    0b01000},
    {"ADD",      0b01001},
    {"ADDT",     0b01001},
    {"CMPCT",    0b01010},
    {"CMPCTR",   0b01011},
    {"AND",      0b01100},
    {"ANDR",     0b01101},
    {"CRC16",    0b01110},
    {"CRC32",    0b01111},
    {"CSUM",     0b10000},
    {"XOR",      0b10001},
    {"XORR",     0b10010},
    {"HASH",     0b10011},
    {"HASHR",    0b10100},
    {"++GET",    0b10101},
    {"GET++",    0b10101},
    {"--GET",    0b10101},
    {"GET--",    0b10101},
    {"LDC",      0b10101},
    {"COPY",     0b10110},
    {"MSKALL",   0b10111},
    {"MSKADDR",  0b10111},
    {"NOP",      0b11000},
    {"J",        0b11001},
    {"BEZ",      0b11001},
    {"RET",      0b11010},
    {"END",      0b11011},
    {"OR",       0b11100},
    {"ORR",      0b11101}
};

const u32_regex_map deparser_assembler::opcode_regex_map = {
    {0b00001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00010_00011_00100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00010_00011_00100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00010_00011_00100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00101 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00110, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00110_00111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b00111, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_00110_00111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01000, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01000_01001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01000_01001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01010_01100_11100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01011_01101_11101 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01010_01100_11100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01011_01101_11101 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01110, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01110_01111_10000 + INSTRUCTION_LINE_POSFIX_P)},
    {0b01111, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01110_01111_10000 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10000, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01110_01111_10000 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10001_10011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10010_10100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10001_10011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10010_10100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10101 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10110, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10110 + INSTRUCTION_LINE_POSFIX_P)},
    {0b10111, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_10111 + INSTRUCTION_LINE_POSFIX_P)},
    {0b11000, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_11000_11010_11011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b11001, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_11001 + INSTRUCTION_LINE_POSFIX_P)},
    {0b11010, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_11000_11010_11011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b11011, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_11000_11010_11011 + INSTRUCTION_LINE_POSFIX_P)},
    {0b11100, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01010_01100_11100 + INSTRUCTION_LINE_POSFIX_P)},
    {0b11101, regex(string(INSTRUCTION_LINE_PREFIX_P) + P_01011_01101_11101 + INSTRUCTION_LINE_POSFIX_P)}
};
