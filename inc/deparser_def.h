#ifndef DEPARSER_DEF_H
#define DEPARSER_DEF_H

#include "deparser_assembler.h"

typedef union {
    struct {
        u32 opcode: 5;
        u32 rsvd: 25;
        u32 src_slct: 2;
    } op_00001;
    struct {
        u32 opcode: 5;
        u32 offset: 9;
        u32 length: 12;
        u32 rsvd: 2;
        u32 off_ctrl: 1;
        u32 len_ctrl: 1;
        u32 src_slct: 2;
    } op_00010;
    struct {
        u32 opcode: 5;
        u32 offset: 9;
        u32 length: 12;
        u32 calc_mode: 2;
        u32 off_ctrl: 1;
        u32 len_ctrl: 1;
        u32 src_slct: 2;
    } op_00011, op_00100;
    struct {
        u32 opcode: 5;
        u32 dst_slct: 3;
        u32 value: 16;
        u32 ctrl_mode: 8;
    } op_00110, op_00111;
    struct {
        u32 opcode: 5;
        u32 src_off: 9;
        u32 src_len: 3;
        u32 dst_off: 9;
        u32 dst_len: 4;
        u32 src_slct: 2;
    } op_01000, op_01001;
    struct {
        u32 opcode: 5;
        u32 src_off: 9;
        u32 src_len: 9;
        u32 dst_off: 9;
    } op_01010, op_01100, op_11100;
    struct {
        u32 opcode: 5;
        u32 rsvd: 17;
        u32 dst_slct: 1;
        u32 dst_off: 9;
    } op_01011, op_01101, op_11101;
    struct {
        u32 opcode: 5;
        u32 offset: 9;
        u32 length: 12;
        u32 mask_flg: 1;
        u32 mask_en: 1;
        u32 off_ctrl: 1;
        u32 len_ctrl: 1;
        u32 src_slct: 2;
    } op_01110, op_01111, op_10000;
    struct {
        u32 opcode: 5;
        u32 src_off: 9;
        u32 src_len: 6;
        u32 calc_unit: 2;
        u32 dst_slct: 1;
        u32 dst_off: 9;
    } op_10001;
    struct {
        u32 opcode: 5;
        u32 rsvd: 15;
        u32 calc_unit: 2;
        u32 dst_slct: 1;
        u32 dst_off: 9;
    } op_10010;
    struct {
        u32 opcode: 5;
        u32 src_off: 9;
        u32 src_len: 6;
        u32 hash_type: 2;
        u32 dst_slct: 1;
        u32 dst_off: 9;
    } op_10011;
    struct {
        u32 opcode: 5;
        u32 rsvd: 15;
        u32 hash_type: 2;
        u32 dst_slct: 1;
        u32 dst_off: 9;
    } op_10100;
    struct {
        u64 opcode: 5;
        u64 cmd_type: 4;
        u64 cntr_idx: 12;
        u64 imm8: 8;
        u64 phv_off: 9;
        u64 lck_flg: 1;
        u64 ulk_flg: 1;
        u64 cntr_idx_slct: 1;
        u64 imm_slct: 1;
        u64 cntr_width: 1;
        u64 rsvd: 21;
    } op_10101;
    struct {
        u32 opcode: 5;
        u32 direction: 2;
        u32 src_off: 9;
        u32 dst_off: 9;
        u32 length: 2;
        u32 rsvd: 5;
    } op_10110;
    struct {
        u32 opcode: 5;
        u32 select: 2;
        u32 address: 16;
        u32 rsvd: 9;
    } op_10111;
    struct {
        u64 opcode: 5;
        u64 target_1: 16;
        u64 target_2: 16;
        u64 jump_mode: 4;
        u64 src_slct: 4;
        u64 src_off: 12;
        u64 rsvd: 7;
    } op_11001;
    struct {
        u32 opcode: 5;
        u32 rsvd: 27;
    } universe;
    u32 val32;
    u64 val64;
} machine_code;

const string P_00001 = assembler::normal_line_prefix_p +
    R"()" + assembler::normal_line_posfix_p;

const string P_00010_00011_00100 = assembler::normal_line_prefix_p +
    R"(((PHV|HDR|PLD)\s*(\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])?\s*:\s*)"
    R"(([1-9][0-9]{0,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])?\s*\})?(\s*,\s+(CSUM|CRC16|CRC32))?)?)"
    + assembler::normal_line_posfix_p;

const string P_00101 = assembler::normal_line_prefix_p +
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\}\s*,\s+TMP)"
    + assembler::normal_line_posfix_p;

const string P_00110_00111 = assembler::normal_line_prefix_p +
    R"((TMP|COND|POFF|PLEN|POLY|INIT|CTRL\s*,\s+0b000([01]{5})|XOROT)\s*,\s+(\d{1,5}|0[xX][\dA-F]{1,4}))"
    + assembler::normal_line_posfix_p;

const string P_01000_01001 = assembler::normal_line_prefix_p +
    R"(TMP\s*,\s+(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}\s*,\s+)"
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([2-9])\s*\}|COND|POFF|PLEN))"
    + assembler::normal_line_posfix_p;

const string P_01010_01100_11100 = assembler::normal_line_prefix_p +
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*)"
    R"(([1-9][0-9]?|[1-4][0-9]{2}|50[0-9]|51[01])\s*\}\s*,\s+)"
    R"(PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_01011_01101_11101 = assembler::normal_line_prefix_p +
    R"((PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])?)"
    + assembler::normal_line_posfix_p;

const string P_01110_01111_10000 = assembler::normal_line_prefix_p +
    R"((PHV|HDR|PLD)\s*(\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])?\s*:\s*)"
    R"(([1-9][0-9]{0,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])?\s*\})?)"
    + assembler::normal_line_posfix_p;

const string P_10001_10011 = assembler::normal_line_prefix_p +
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-5][0-9]?|6[0-3])\s*\}\s*,\s+)"
    R"((PHV|META)\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_10010_10100 = assembler::normal_line_prefix_p +
    R"((PHV|META)\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_10101 = assembler::normal_line_prefix_p +
    R"((CNTR\s*\[\s*([0-9]|[1-9][0-9]{1,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])\s*]\s*,\s+)?)"
    R"((([1-9][0-9]{1,2}|2[0-4][0-9]|25[0-5])\s*,\s+)?)"
    R"(((PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*(32|64)\s*\})|(TMP))\s*,\s+(LCK|ULK))"
    + assembler::normal_line_posfix_p;

const string P_10110 = assembler::normal_line_prefix_p +
    R"((PHV|META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\}\s*,\s+)"
    R"((PHV|META)\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])])"
    + assembler::normal_line_posfix_p;

const string P_10111 = assembler::normal_line_prefix_p +
    R"((0[xX][\dA-Fa-f]{1,4})?)" + assembler::normal_line_posfix_p;

const string P_11001 = assembler::normal_line_prefix_p +
    R"((([-])?\d{1,5}|0[xX][\dA-Fa-f]{1,4})(\s*,\s+(([-])?\d{1,5}|0[xX][\dA-Fa-f]{1,4})\s*,\s+)"
    R"((CONDR|PHV(1|8|16|32)\s*\[\s*([0-9]|[1-9][0-9]{1,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])\s*]))?)"
    + assembler::normal_line_posfix_p;

const string P_11000_11010_11011 = P_00001;

#endif // DEPARSER_DEF_H