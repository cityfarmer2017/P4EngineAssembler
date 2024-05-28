#include <limits>
#include "parser_def.h"
#include "parser_assembler.h"

using std::cout;
using std::endl;
using std::stoul;
using std::stoull;

inline string parser_assembler::get_name_pattern(void) const
{
    return cmd_name_pattern;
}

string parser_assembler::get_name_matched(const smatch &m, vector<bool> &flags) const
{
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

int parser_assembler::line_process(const string &line, const string &name, const vector<bool> &flags)
{
    machine_code mcode;
    mcode.val64 = cmd_opcode_map.at(name);
    smatch m;

    if (!regex_match(line, m, opcode_regex_map.at(mcode.val64))) {
        cout << "regex match failed with line:\n\t" << line << endl;
        return -1;
    }

    if (flags.size() != 3) {
        return -1;
    }

    bool last = flags[0];
    bool usign = flags[1];
    bool mask0 = flags[2];

    switch (mcode.val64)
    {
    case 0b00001: // MOV, MDF
        if (name == "MOV" && !m.str(2).empty()) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }

        if (stoull(m.str(1), nullptr, 0) > std::numeric_limits<u32>::max()) {
            cout << "imm32 value exceeds limit.\n\t" << line << endl;
            return -1;
        }
        mcode.op_00001.imm32 = stoul(m.str(1), nullptr, 0); // heximal or decimal

        mcode.op_00001.src_len = 0x1f;
        if (!m.str(2).empty()) {
            if (m.str(3) == "TMP") {
                mcode.op_00001.src_slct = 1;
            }
            mcode.op_00001.src_off = stoul(m.str(4));
            mcode.op_00001.src_len = stoul(m.str(5));
            if (mcode.op_00001.src_len) {
                mcode.op_00001.src_len--;
            }
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
            mcode.op_00001.dst_len = stoul(m.str(dst_len_idx));
            if (mcode.op_00001.dst_len) {
                mcode.op_00001.dst_len--;
            }
        }
        break;

    case 0b00010: // RMV
    // intended fall through
    case 0b00011: // XCT
        if (m.str(1) == "TMP") {
            mcode.op_00011.src_slct = 1;
        }
        mcode.op_00011.src_off = stoul(m.str(2));
        mcode.op_00011.src_len = stoul(m.str(3));
        if (mcode.op_00011.src_len) {
            mcode.op_00011.src_len--;
        }

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
            mcode.op_00011.dst_len = stoul(m.str(dst_len_idx));
            if (mcode.op_00011.dst_len) {
                mcode.op_00011.dst_len--;
            }
        }
        break;

    case 0b10101: // ADDU / SUBU
        if (m.str(1) == "TMP") {
            mcode.op_10101.src0_slct = 1;
        }
        mcode.op_10101.src0_off = stoul(m.str(2));
        mcode.op_10101.src0_len = stoul(m.str(3));
        if (mcode.op_10101.src0_len) {
            mcode.op_10101.src0_len--;
        }
        if (name == "SUBU") {
            mcode.op_10101.alu_mode = 1;
        }
        if (m.str(4) == "TMP") {
            mcode.op_10101.src1_slct = 1;
        } else {
            if (stoul(m.str(4), nullptr, 0) > std::numeric_limits<unsigned short>::max()) {
                cout << "imm16 value exceeds limit.\n\t" << line << endl;
                return -1;
            }
            mcode.op_10101.imm16 = stoul(m.str(4), nullptr, 0);
        }
        if (m.str(5) == "TMP") {
            mcode.op_10101.dst_slct = 2;
        } else {
            auto dst_off_idx = 7;
            auto dst_len_idx = 8;
            if (m.str(6) == "META") {
                mcode.op_10101.dst_slct = 1;
            }
            if (m.str(9) == "PHV") {
                dst_off_idx = 10;
                dst_len_idx = 11;
            }
            mcode.op_10101.dst_off = stoul(m.str(dst_off_idx));
            mcode.op_10101.dst_len = stoul(m.str(dst_len_idx));
            if (mcode.op_10101.dst_len) {
                mcode.op_10101.dst_len--;
            }
        }
        break;

    case 0b10100: // CPYM
        if (m.str(1) != "CALC_RSLT") {
            unsigned int idx = stoul(m.str(2));
            if (idx <= 3) {
                mcode.op_10100.src_slct = idx + 0b100;
            } else {
                mcode.op_10100.src_slct = idx + 0b1000;
            }
        }
        if (m.str(3) == "TMP") {
            mcode.op_10100.dst_slct = 2;
        } else {
            auto dst_off_idx = 5;
            auto dst_len_idx = 6;
            if (m.str(4) == "META") {
                mcode.op_10100.dst_slct = 1;
            }
            if (m.str(7) == "PHV") {
                dst_off_idx = 8;
                dst_len_idx = 9;
            }
            mcode.op_10100.dst_off = stoul(m.str(dst_off_idx));
            mcode.op_10100.dst_len = stoul(m.str(dst_len_idx));
            if (mcode.op_10100.dst_len) {
                mcode.op_10100.dst_len--;
            }
        }
        break;

    case 0b00111: // RSM16 / RSM32
        if (name == "RSM32") {
            mcode.op_00111.len_flg = 1;
        }
        if (m.str(3).empty()) {
            mcode.op_00111.inline_flg = 1;
        } else {
            mcode.op_00111.meta_left_shift = stoul(m.str(4));
        }
        mcode.op_00111.addr = stoull(m.str(1), nullptr, 0);
        mcode.op_00111.line_off = stoul(m.str(2));
        break;

    case 0b00101: // LOCK
    // intended fall through
    case 0b00110: // ULCK
        if (!m.str(1).empty()) {
            mcode.op_00101.flow_id = stoul(m.str(1), nullptr, 0);
            mcode.op_00101.inline_flg = 1;
        }
        break;

    case 0b00100: // SHFT
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
        break;

    case 0b01001: // CSET
        if (m.str(1) == "POLY") {
            mcode.op_01001.calc_slct = 4;
        } else if (m.str(1) == "INIT") {
            mcode.op_01001.calc_slct = 5;
        } else if (m.str(1) == "XOROT") {
            mcode.op_01001.calc_slct = 7;
        } else { // (m.str(1) == "CTRL")
            mcode.op_01001.calc_slct = 6;
            mcode.op_01001.ctrl_mode = stoul(m.str(2), nullptr, 2);
        }
        if (m.str(4).empty()) {
            auto val = stoul(m.str(3));
            if (val > std::numeric_limits<unsigned int>::max()) {
                print_cmd_param_unmatch_message(name, line);
                return -1;
            }
            mcode.op_01001.value = stoul(m.str(3));
        } else {
            mcode.op_01001.value = stoul(m.str(3), nullptr, 0); // heximal
        }
        break;

    case 0b01100: // HCSUM
    // intended fall through
    case 0b01011: // HCRC16
    // intended fall through
    case 0b01010: // HCRC32
        if (!mask0) {
            mcode.op_01100.mask_flg = 1;
        }
        if (m.str(1) == "TMP") {
            mcode.op_01100.src_slct = 1;
        }
        mcode.op_01100.offset = stoul(m.str(2));
        mcode.op_01100.length = stoul(m.str(3));
        mcode.op_01100.mask = stoul(m.str(4), nullptr, 0);
        break;

    case 0b01000: // NOP
    // intended fall through
    case 0b01111: // PCSUM
    // intended fall through
    case 0b01110: // PCRC16
    // intended fall through
    case 0b01101: // PCRC32
        /* code */
        break;

    case 0b10000: // SNE(U) / SGT(U) / SLT(U) / SGE(U) / SLE(U)
        if (!usign) {
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
        break;

    case 0b10001: // NXTH
        if (!m.str(1).empty()) {
            mcode.op_10001.isr_off = stoul(m.str(2));
            mcode.op_10001.isr_len = stoul(m.str(3));
        }
        if (!m.str(4).empty()) {
            mcode.op_10001.meta_off = stoul(m.str(5));
            mcode.op_10001.meta_len = stoul(m.str(6));
        }
        if (mcode.op_10001.meta_len + mcode.op_10001.isr_len > 40) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        if (mcode.op_10001.isr_len == 40 && !m.str(4).empty()) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        if (mcode.op_10001.meta_len == 40 && !m.str(1).empty()) {
            print_cmd_param_unmatch_message(name, line);
            return -1;
        }
        if (!m.str(7).empty()) {
            mcode.op_10001.shift_val = stoul(m.str(8));
        }
        mcode.op_10001.sub_state = stoul(m.str(9));
        break;

    case 0b10010: // NXTD
    // intended fall through
    case 0b10011: // NXTP
        if (!m.str(1).empty()) {
            mcode.op_10010.shift_val = stoul(m.str(2));
        }
        break;
    
    default:
        break;
    }

    mcode.universe.last_flg = last;

    mcode_vec.emplace_back(mcode.val64);

    return 0;
}

int parser_assembler::write_mcode_to_destinations(const string &out_fname)
{
    print_mcode_line_by_line(std::cout, mcode_vec);

    if (out_fname.empty()) {
        return 0;
    }

    if (auto rc = open_output_file(out_fname + ".dat"))
    {
        return rc;
    }

    dst_fstrm.write(reinterpret_cast<const char*>(mcode_vec.data()), sizeof(mcode_vec[0]) * mcode_vec.size());
    dst_fstrm.close();
#if 0
    std::ifstream ifs(out_fname + ".dat");
    std::vector<u64> data(mcode_vec.size());
    ifs.read(reinterpret_cast<char*>(data.data()), sizeof(data[0]) * data.size());
    for (const auto&d : data) {
        cout << d << endl;
    }
#endif

    if (auto rc = open_output_file(out_fname + ".txt")) {
        return rc;
    }

    print_mcode_line_by_line(dst_fstrm, mcode_vec);
    dst_fstrm.close();

    return 0;
}

const string parser_assembler::cmd_name_pattern = \
    R"((MOV|MDF|XCT|RMV|ADDU|SUBU|COPY|RSM16|RSM32|LOCK|ULCK|NOP|SHFT|CSET|)"
    R"((HCSUM|HCRC16|HCRC32)(M0)?|PCSUM|PCRC16|PCRC32|(SNE|SGT|SLT|SEQ|SGE|SLE)(U)?|NXTH|NXTP|NXTD)(L)?)";

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
    {0b00001, regex(P_00001)},
    {0b00011, regex(P_00010_00011)},
    {0b00010, regex(P_00010_00011)},
    {0b10101, regex(P_10101)},
    {0b10100, regex(P_10100)},
    {0b00111, regex(P_00111)},
    {0b00101, regex(P_00101_00110)},
    {0b00110, regex(P_00101_00110)},
    {0b01000, regex(P_01000_01101_01110_01111)},
    {0b00100, regex(P_00100)},
    {0b01001, regex(P_01001)},
    {0b01100, regex(P_01010_01011_01100)},
    {0b01011, regex(P_01010_01011_01100)},
    {0b01010, regex(P_01010_01011_01100)},
    {0b01111, regex(P_01000_01101_01110_01111)},
    {0b01110, regex(P_01000_01101_01110_01111)},
    {0b01101, regex(P_01000_01101_01110_01111)},
    {0b10000, regex(P_10000)},
    {0b10001, regex(P_10001)},
    {0b10011, regex(P_10010_10011)},
    {0b10010, regex(P_10010_10011)}
};