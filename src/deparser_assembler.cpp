/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <limits>
#include "deparser_assembler.h"  // NOLINT [build/include_subdir]
#include "deparser_def.h"  // NOLINT [build/include_subdir]
#include "table_proc/mask_table.h"

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

string deparser_assembler::get_name_matched(const smatch &m, vector<bool> &flags) const {
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

constexpr auto sendm_mask_flg_idx = 0;
constexpr auto sendm_pipeline_flg_idx = 1;
constexpr auto calc_mask_and_flg_idx = 2;
constexpr auto calc_mask_or_flg_idx = 3;
constexpr auto xor8_flg_idx = 4;
constexpr auto xor16_flg_idx = 5;
constexpr auto xor32_flg_idx = 6;
constexpr auto jump_relative_flg_idx = 7;
constexpr auto match_state_no_line_flg_idx = 8;
constexpr auto flags_size = 9;

static inline void print_cmd_bit_vld_unmatch_message(const string &line) {
    std::cout << line + "\n\tshall be following a <MSKALL / MSKADDR> line." << std::endl;
}

static inline void compose_sndm(const vector<bool> &flags, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (flags[sendm_mask_flg_idx]) {
        mcode.op_00001.src_slct = 2;
    } else if (!flags[sendm_pipeline_flg_idx]) {
        mcode.op_00001.src_slct = 1;
    }
}

static inline int compose_sndh_sndp_sndpc(const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    if (m.str(1).empty()) {
        mcode.op_00010.src_slct = 3;
    } else {
        if (m.str(2) == "PLD") {
            mcode.op_00010.src_slct = 2;
        } else if (m.str(2) == "HDR") {
            mcode.op_00010.src_slct = 1;
        }
        if (m.str(3).empty()) {
            mcode.op_00010.off_ctrl = 1;
            mcode.op_00010.len_ctrl = 1;
        } else {
            if (m.str(4).empty() && m.str(5).empty()) {
                return -1;
            }
            if (m.str(4).empty()) {
                mcode.op_00010.off_ctrl = 1;
            } else {
                mcode.op_00010.offset = stoul(m.str(4));
            }
            if (m.str(5).empty()) {
                mcode.op_00010.len_ctrl = 1;
            } else {
                mcode.op_00010.length = stoul(m.str(5));
            }
        }
        if (!m.str(6).empty()) {
            if (m.str(7) == "CSUM") {
                mcode.op_00011.calc_mode = 1;
            } else if (m.str(7) == "CRC16") {
                mcode.op_00011.calc_mode = 2;
            } else {  // m.str(7) == "CRC32"
                mcode.op_00011.calc_mode = 3;
            }
        }
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

static inline int compose_add_addu(const smatch &m, const machine_code &code) {
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
        mcode.op_01000.dst_off = stoul(m.str(4));
        mcode.op_01000.dst_len = stoul(m.str(5));
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
    if (flags[calc_mask_and_flg_idx] || flags[calc_mask_or_flg_idx]) {
        mcode.op_01110.mask_en = 1;
        if (flags[calc_mask_or_flg_idx]) {
            mcode.op_01110.mask_flg = 1;
        }
    }
    return 0;
}

static inline void compose_xor(const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_10001.src_off = stoul(m.str(1));
    mcode.op_10001.src_len = stoul(m.str(2));
    mcode.op_10001.calc_unit = assembler::get_xor_unit(flags[xor8_flg_idx], flags[xor16_flg_idx], flags[xor32_flg_idx]);
    mcode.op_10001.dst_slct = m.str(3) == "PHV";
    mcode.op_10001.dst_off = stoul(m.str(4));
}

static inline void compose_xorr(const vector<bool> &flags, const smatch &m, const machine_code &code) {
    auto &mcode = const_cast<machine_code&>(code);
    mcode.op_10010.calc_unit = assembler::get_xor_unit(flags[xor8_flg_idx], flags[xor16_flg_idx], flags[xor32_flg_idx]);
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
    if (m.str(10) == "LCK") {
        mcode.op_10101.lck_flg = 1;
    } else if (m.str(10) == "ULK") {
        mcode.op_10101.ulk_flg = 1;
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
    if (!flags[jump_relative_flg_idx] && (m.str(2) == "-" || m.str(5) == "-")) {
        return -1;
    }
    mcode.op_11001.target_1 =
        flags[jump_relative_flg_idx] ? std::stol(m.str(1), nullptr, 0) : stoul(m.str(1), nullptr, 0);
    if (!m.str(4).empty()) {
        mcode.op_11001.target_2 =
            flags[jump_relative_flg_idx] ? std::stol(m.str(4), nullptr, 0) : stoul(m.str(4), nullptr, 0);
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
        mcode.op_11001.jump_mode = flags[jump_relative_flg_idx] ? 1 : 0;
    } else if (name == "BEZ") {
        mcode.op_11001.jump_mode = flags[jump_relative_flg_idx] ? 0b11 : 0b10;
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
    if (flags.size() != flags_size) {
        return -1;
    }

    machine_code mcode;
    mcode.val64 = 0;
    mcode.val32 = cmd_opcode_map.at(name);
    auto opcode = mcode.val32;

    smatch m;
    if (!regex_match(line, m, opcode_regex_map.at(mcode.val32))) {
        cout << "regex match failed with line:\n\t" << line << endl;
        return -1;
    }

    switch (opcode) {
    case 0b00001:  // SNDM[PM]
        compose_sndm(flags, mcode);
        if (flags[sendm_mask_flg_idx]) {
            if (auto rc = check_previous(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return rc;
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
            if (auto rc = check_previous(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return rc;
            }
            swap_previous(mcode.val32);
        }
        break;

    case 0b00110:  // SETH
    case 0b00111:  // SETL
        if (auto rc = compose_seth_setl(m, mcode)) {
            cout << "imm16 value exceeds limit.\n\t" << line << endl;
            return rc;
        }
        break;

    case 0b01000:  // ADDU
    case 0b01001:  // ADD
        if (auto rc = compose_add_addu(m, mcode)) {
            print_cmd_param_unmatch_message(name, line);
            return rc;
        }
        break;

    case 0b01010:  // CMPCT
    case 0b01100:  // AND
    case 0b11100:  // OR
        compose_cmpct_and_or(m, mcode);
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
        }
        swap_previous(mcode.val32);
        break;

    case 0b01011:  // CMPCTR
    case 0b01101:  // ANDR
    case 0b11101:  // ORR
        compose_cmpctr_andr_orr(m, mcode);
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
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
        if (flags[calc_mask_and_flg_idx] || flags[calc_mask_or_flg_idx]) {
            if (auto rc = check_previous(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return rc;
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
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
        }
        swap_previous(mcode.val32);
        break;

    case 0b10100:  // HASHR
        compose_hashr(m , mcode);
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
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

inline int deparser_assembler::check_previous(const string &line) const {
    if (prev_line_name != "MSKALL" && prev_line_name != "MSKADDR") {
        return -1;
    }
    return 0;
}

inline void deparser_assembler::swap_previous(const std::uint32_t &mcode) {
    auto val = mcode_vec.back();
    mcode_vec.pop_back();
    mcode_vec.emplace_back(mcode);
    (const_cast<std::uint32_t&>(mcode)) = val;
}

const char* deparser_assembler::cmd_name_pattern =
    R"((SNDM([PM])?|SND[HP]C?|MOVE|SET[HL]|ADDU?|CMPCTR?|ANDR?|ORR?|(CRC16|CRC32|CSUM)(M[AO])?|)"
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
    {"ADD",      0b01001},
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
    {0b00001, regex(string(g_normal_line_prefix_p) + P_00001 + g_normal_line_posfix_p)},
    {0b00010, regex(string(g_normal_line_prefix_p) + P_00010_00011_00100 + g_normal_line_posfix_p)},
    {0b00011, regex(string(g_normal_line_prefix_p) + P_00010_00011_00100 + g_normal_line_posfix_p)},
    {0b00100, regex(string(g_normal_line_prefix_p) + P_00010_00011_00100 + g_normal_line_posfix_p)},
    {0b00101, regex(string(g_normal_line_prefix_p) + P_00101 + g_normal_line_posfix_p)},
    {0b00110, regex(string(g_normal_line_prefix_p) + P_00110_00111 + g_normal_line_posfix_p)},
    {0b00111, regex(string(g_normal_line_prefix_p) + P_00110_00111 + g_normal_line_posfix_p)},
    {0b01000, regex(string(g_normal_line_prefix_p) + P_01000_01001 + g_normal_line_posfix_p)},
    {0b01001, regex(string(g_normal_line_prefix_p) + P_01000_01001 + g_normal_line_posfix_p)},
    {0b01010, regex(string(g_normal_line_prefix_p) + P_01010_01100_11100 + g_normal_line_posfix_p)},
    {0b01011, regex(string(g_normal_line_prefix_p) + P_01011_01101_11101 + g_normal_line_posfix_p)},
    {0b01100, regex(string(g_normal_line_prefix_p) + P_01010_01100_11100 + g_normal_line_posfix_p)},
    {0b01101, regex(string(g_normal_line_prefix_p) + P_01011_01101_11101 + g_normal_line_posfix_p)},
    {0b01110, regex(string(g_normal_line_prefix_p) + P_01110_01111_10000 + g_normal_line_posfix_p)},
    {0b01111, regex(string(g_normal_line_prefix_p) + P_01110_01111_10000 + g_normal_line_posfix_p)},
    {0b10000, regex(string(g_normal_line_prefix_p) + P_01110_01111_10000 + g_normal_line_posfix_p)},
    {0b10001, regex(string(g_normal_line_prefix_p) + P_10001_10011 + g_normal_line_posfix_p)},
    {0b10010, regex(string(g_normal_line_prefix_p) + P_10010_10100 + g_normal_line_posfix_p)},
    {0b10011, regex(string(g_normal_line_prefix_p) + P_10001_10011 + g_normal_line_posfix_p)},
    {0b10100, regex(string(g_normal_line_prefix_p) + P_10010_10100 + g_normal_line_posfix_p)},
    {0b10101, regex(string(g_normal_line_prefix_p) + P_10101 + g_normal_line_posfix_p)},
    {0b10110, regex(string(g_normal_line_prefix_p) + P_10110 + g_normal_line_posfix_p)},
    {0b10111, regex(string(g_normal_line_prefix_p) + P_10111 + g_normal_line_posfix_p)},
    {0b11000, regex(string(g_normal_line_prefix_p) + P_11000_11010_11011 + g_normal_line_posfix_p)},
    {0b11001, regex(string(g_normal_line_prefix_p) + P_11001 + g_normal_line_posfix_p)},
    {0b11010, regex(string(g_normal_line_prefix_p) + P_11000_11010_11011 + g_normal_line_posfix_p)},
    {0b11011, regex(string(g_normal_line_prefix_p) + P_11000_11010_11011 + g_normal_line_posfix_p)},
    {0b11100, regex(string(g_normal_line_prefix_p) + P_01010_01100_11100 + g_normal_line_posfix_p)},
    {0b11101, regex(string(g_normal_line_prefix_p) + P_01011_01101_11101 + g_normal_line_posfix_p)}
};
