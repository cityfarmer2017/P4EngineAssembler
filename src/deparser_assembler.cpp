#include <limits>
#include "deparser_assembler.h"
#include "deparser_def.h"

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

inline string deparser_assembler::get_name_pattern(void) const
{
    return cmd_name_pattern;
}

int deparser_assembler::line_process(const string &line, const string &name, const vector<bool> &flags)
{
    machine_code mcode;
    mcode.val64 = 0;
    mcode.val32 = cmd_opcode_map.at(name);

    smatch m;
    if (!regex_match(line, m, opcode_regex_map.at(mcode.val32))) {
        cout << "regex match failed with line:\n\t" << line << endl;
        return -1;
    }

    if (flags.size() != 8) {
        return -1;
    }

    auto sndm_m = flags[0];
    auto sndm_p = flags[1];
    auto calc_ma = flags[2];
    auto calc_mo = flags[3];
    // auto xor_4 = flags[3];
    auto xor_8 = flags[4];
    auto xor_16 = flags[5];
    auto xor_32 = flags[6];
    auto r_flg = flags[7];

    switch (mcode.val32)
    {
    case 0b00001: // SNDM[PM]
        if (sndm_m) {
            if (auto rc = check_previous(line)) {
                print_cmd_bit_vld_unmatch_message(line);
                return rc;
            }
            mcode.op_00001.src_slct = 2;
            swap_previous(mcode);
        } else if (!sndm_p) {
            mcode.op_00001.src_slct = 1;
        }
        break;

    case 0b00010: // SNDH
    case 0b00011: // SNDP
    case 0b00100: // SNDPC
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
                    print_cmd_param_unmatch_message(name, line);
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
                } else { // m.str(7) == "CRC32"
                    mcode.op_00011.calc_mode = 3;
                }
            }
        }
        break;

    case 0b00110: // SETH
    case 0b00111: // SETL
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
        } else if (m.str(1) != "TMP") { // "CTRL"
            mcode.op_00110.dst_slct = 6;
            mcode.op_00110.ctrl_mode = stoul(m.str(2), nullptr, 2);
        }
        if (stoul(m.str(3), nullptr, 0) > std::numeric_limits<unsigned short>::max()) {
            cout << "imm16 value exceeds limit.\n\t" << line << endl;
            return -1;
        }
        mcode.op_00110.value = stoul(m.str(3), nullptr, 0); // heximal or decimal
        break;

    case 0b01000: // ADDU
    case 0b01001: // ADD
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
                print_cmd_param_unmatch_message(name, line);
                return -1;
            }
        }
        break;

    case 0b01010: // CMPCT
    case 0b01100: // AND
    case 0b11100: // OR
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
        }
        mcode.op_01010.src_off = stoul(m.str(1));
        mcode.op_01010.src_len = stoul(m.str(2));
        mcode.op_01010.dst_off = stoul(m.str(3));
        swap_previous(mcode);
        break;

    case 0b01011: // CMPCTR
    case 0b01101: // ANDR
    case 0b11101: // ORR
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
        }
        if (!m.str(1).empty()) {
            mcode.op_01011.dst_off = stoul(m.str(2));
        } else {
            mcode.op_01011.dst_slct = 1;
        }
        swap_previous(mcode);
        break;

    case 0b01110: // CRC16
    case 0b01111: // CRC32
    case 0b10000: // CSUM
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
                print_cmd_param_unmatch_message(name, line);
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
        if (calc_ma || calc_mo) {
            mcode.op_01110.mask_en = 1;
            if (calc_mo) {
                mcode.op_01110.mask_flg = 1;
            }
        }
        break;

    case 0b10001: // XOR
        mcode.op_10001.src_off = stoul(m.str(1));
        mcode.op_10001.src_len = stoul(m.str(2));
        mcode.op_10001.calc_unit = xor_8 ? 1 : xor_16 ? 2 : xor_32 ? 3 : 0;
        mcode.op_10001.dst_slct = m.str(3) == "PHV";
        mcode.op_10001.dst_off = stoul(m.str(4));
        break;

    case 0b10010: // XORR
        mcode.op_10010.calc_unit = xor_8 ? 1 : xor_16 ? 2 : xor_32 ? 3 : 0;
        mcode.op_10010.dst_slct = m.str(1) == "PHV";
        mcode.op_10010.dst_off = stoul(m.str(2));
        break;

    case 0b10011: // HASH
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
        }
        mcode.op_10011.src_off = stoul(m.str(1));
        mcode.op_10011.src_len = stoul(m.str(2));
        mcode.op_10011.dst_slct = m.str(3) == "PHV";
        mcode.op_10011.dst_off = stoul(m.str(4));
        swap_previous(mcode);
        break;

    case 0b10100: // HASHR
        if (auto rc = check_previous(line)) {
            print_cmd_bit_vld_unmatch_message(line);
            return rc;
        }
        mcode.op_10100.dst_slct = m.str(1) == "PHV";
        mcode.op_10100.dst_off = stoul(m.str(2));
        swap_previous(mcode);
        break;

    case 0b10101: // ++GET / GET++ / --GET / GET-- / LDPC / LDTC
        if (m.str(10) == "LCK") {
            mcode.op_10101.lck_flg = 1;
        } else if (m.str(10) == "ULK") {
            mcode.op_10101.ulk_flg = 1;
        }
        if (name == "LDC") {
            if (!m.str(3).empty()) {
                print_cmd_param_unmatch_message(name, line);
                return -1;
            }
            if(!m.str(9).empty()) { // from TMP
                mcode.op_10101.cmd_type = 5;
            } else { // from PHV
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
        break;

    case 0b10110:
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
        break;

    case 0b10111: // MSKALL / MSKADDR
        if (name == "VLDALL" && !m.str(1).empty()) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        if (!m.str(1).empty()) {
            mcode.op_10111.select = 1;
            mcode.op_10111.address = stoul(m.str(1), nullptr, 0);
        } else {
            mcode.op_10111.select = 2;
        }
        break;

    case 0b11001:
        if ((name == "J" && !m.str(3).empty())) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        if (!r_flg && (m.str(2) == "-" || m.str(5) == "-")) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        mcode.op_11001.target_1 = r_flg ? std::stol(m.str(1), nullptr, 0) : stoul(m.str(1), nullptr, 0);
        if (!m.str(4).empty()) {
            mcode.op_11001.target_2 = r_flg ? std::stol(m.str(4), nullptr, 0) : stoul(m.str(4), nullptr, 0);
        }
        if (!m.str(6).empty() && m.str(6) != "CONDR") {
            mcode.op_11001.src_slct = 1;
            auto src_off = stoul(m.str(8));
            if (m.str(7) == "8") {
                if (src_off % 8) {
                    print_cmd_param_unmatch_message(name, line);
                    return -1;
                }
                mcode.op_11001.src_slct = 2;
            } else if (m.str(7) == "16") {
                if (src_off % 16) {
                    print_cmd_param_unmatch_message(name, line);
                    return -1;
                }
                mcode.op_11001.src_slct = 3;
            } else if (m.str(7) == "32") {
                if (src_off % 32) {
                    print_cmd_param_unmatch_message(name, line);
                    return -1;
                }
                mcode.op_11001.src_slct = 6;
            }
            mcode.op_11001.src_off = src_off;
        }
        if (name == "J" && r_flg) {
            mcode.op_11001.jump_mode = 1;
        }
        if (name == "BGT0") {
            mcode.op_11001.jump_mode = r_flg ? 0b11 : 0b10;
        }
        if (name == "BGE0") {
            mcode.op_11001.jump_mode = r_flg ? 0b101 : 0b100;
        }
        break;

    case 0b11000: // NOP
    case 0b11010: // RET
    case 0b11011: // END
        break;

    default:
        break;
    }

    mcode_vec.emplace_back(mcode.val32);
    u32 high32 = mcode.val64 >> 32;
    if (high32) {
        mcode_vec.emplace_back(high32);
    }

    return 0;
}

void deparser_assembler::write_machine_code(void)
{
    dst_fstrm.write(reinterpret_cast<const char*>(mcode_vec.data()), sizeof(mcode_vec[0]) * mcode_vec.size());
}

void deparser_assembler::print_machine_code(void)
{
    print_mcode_line_by_line(std::cout, mcode_vec);
    print_mcode_line_by_line(dst_fstrm, mcode_vec);
}

string deparser_assembler::get_name_matched(const smatch &m, vector<bool> &flags) const
{
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

const string deparser_assembler::cmd_name_pattern = \
    R"((SNDM([PM])?|SND[HP]C?|MOVE|SET[HL]|ADDU?|CMPCTR?|ANDR?|ORR?|(CRC16|CRC32|CSUM)(M[AO])?|)"
    R"(XORR?(4|8|16|32)?|HASHR?|[+-]{2}GET|GET[+-]{2}|LDC|COPY|MSKALL|MSKADDR|NOP|(J|BG[TE]0)(R)?|RET|END))";

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
    {"BGT0",     0b11001},
    {"BGE0",     0b11001},
    {"RET",      0b11010},
    {"END",      0b11011},
    {"OR",       0b11100},
    {"ORR",      0b11101}
};

const u32_regex_map deparser_assembler::opcode_regex_map = {
    {0b00001, regex(P_00001)},
    {0b00010, regex(P_00010_00011_00100)},
    {0b00011, regex(P_00010_00011_00100)},
    {0b00100, regex(P_00010_00011_00100)},
    {0b00101, regex(P_00101)},
    {0b00110, regex(P_00110_00111)},
    {0b00111, regex(P_00110_00111)},
    {0b01000, regex(P_01000_01001)},
    {0b01001, regex(P_01000_01001)},
    {0b01010, regex(P_01010_01100_11100)},
    {0b01011, regex(P_01011_01101_11101)},
    {0b01100, regex(P_01010_01100_11100)},
    {0b01101, regex(P_01011_01101_11101)},
    {0b01110, regex(P_01110_01111_10000)},
    {0b01111, regex(P_01110_01111_10000)},
    {0b10000, regex(P_01110_01111_10000)},
    {0b10001, regex(P_10001_10011)},
    {0b10010, regex(P_10010_10100)},
    {0b10011, regex(P_10001_10011)},
    {0b10100, regex(P_10010_10100)},
    {0b10101, regex(P_10101)},
    {0b10110, regex(P_10110)},
    {0b10111, regex(P_10111)},
    {0b11000, regex(P_11000_11010_11011)},
    {0b11001, regex(P_11001)},
    {0b11010, regex(P_11000_11010_11011)},
    {0b11011, regex(P_11000_11010_11011)},
    {0b11100, regex(P_01010_01100_11100)},
    {0b11101, regex(P_01011_01101_11101)}
};