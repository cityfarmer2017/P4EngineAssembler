#ifndef PARSER_DEF_H
#define PARSER_DEF_H

#include "parser_assembler.h"

typedef union {
    struct {
        u64 opcode: 5;
        u64 src_off: 5;
        u64 src_len: 5;
        u64 src_slct: 1;
        u64 imm32: 32;
        u64 dst_off: 9;
        u64 dst_len: 4;
        u64 dst_slct: 2;
        u64 last_flg: 1;
    } op_00001;
    struct {
        u64 opcode: 5;
        u64 src_off: 5;
        u64 src_len: 5;
        u64 src_slct: 1;
        u64 rsvd: 32;
        u64 dst_off: 9;
        u64 dst_len: 4;
        u64 dst_slct: 2;
        u64 last_flg: 1;
    } op_00010, op_00011;
    struct {
        u64 opcode: 5;
        u64 length: 8;
        u64 calc_slct: 3;
        u64 inline_flg: 1;
        u64 rsvd: 46;
        u64 last_flg : 1;
    } op_00100;
    struct {
        u64 opcode: 5;
        u64 inline_flg: 1;
        u64 rsvd1: 10;
        u64 flow_id : 24;
        u64 rsvd2: 23;
        u64 last_flg: 1;
    } op_00101, op_00110;
    struct {
        u64 opcode: 5;
        u64 inline_flg: 1;
        u64 rsvd1: 10;
        u64 addr : 36;
        u64 meta_left_shift: 3;
        u64 line_off: 2;
        u64 len_flg: 1;
        u64 rsvd2: 5;
        u64 last_flg: 1;
    } op_00111;
    struct {
        u64 opcode: 5;
        u64 rsvd: 58;
        u64 last_flg: 1;
    } op_01000, op_01101, op_01110, op_01111;
    struct {
        u64 opcode: 5;
        u64 calc_slct: 3;
        u64 ctrl_mode: 8;
        u64 value: 32;
        u64 rsvd: 15;
        u64 last_flg: 1;
    } op_01001;
    struct {
        u64 opcode: 5;
        u64 rsvd1: 2;
        u64 offset: 7;
        u64 length: 5;
        u64 src_slct: 2;
        u64 mask: 32;
        u64 mask_flg: 1;
        u64 rsvd2: 9;
        u64 last_flg: 1;
    } op_01010, op_01011, op_01100;
    struct {
        u64 opcode: 5;
        u64 offset: 5;
        u64 length: 5;
        u64 rsvd1: 1;
        u64 imm40: 40;
        u64 sign_flg: 1;
        u64 alu_mode: 3;
        u64 rsvd2: 3;
        u64 last_flg: 1;
    } op_10000;
    struct {
        u64 opcode: 5;
        u64 shift_val: 6;
        u64 sub_state: 8;
        u64 isr_off: 6;
        u64 isr_len: 6;
        u64 meta_intrinsic: 1;
        u64 meta_off: 9;
        u64 meta_len: 6;
        u64 rsvd: 16;
        u64 last_flg: 1;
    } op_10001;
    struct {
        u64 opcode: 5;
        u64 shift_val: 6;
        u64 rsvd: 52;
        u64 last_flg: 1;
    } op_10010, op_10011;
    struct {
        u64 opcode: 5;
        u64 rsvd1: 7;
        u64 src_slct: 4;
        u64 rsvd2: 32;
        u64 dst_off: 9;
        u64 dst_len: 4;
        u64 dst_slct: 2;
        u64 last_flg: 1;
    } op_10100;
    struct {
        u64 opcode: 5;
        u64 src0_off: 5;
        u64 src0_len: 5;
        u64 src0_slct: 1;
        u64 alu_mode: 2;
        u64 imm16: 16;
        u64 src1_slct: 1;
        u64 rsvd: 13;
        u64 dst_off: 9;
        u64 dst_len: 4;
        u64 dst_slct: 2;
        u64 last_flg: 1;
    } op_10101;
    struct {
        u64 opcode: 5;
        u64 rsvd: 58;
        u64 last_flg: 1;
    } universe;
    u64 val64;
} machine_code;

const string P_00001 = assembler::normal_line_prefix_p +
    R"((\d{1,10}|0[xX][\dA-Fa-f]{1,8})\s*,\s+)"
    R"(((ISR|TMP)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+)?)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"((PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))"
    + assembler::normal_line_posfix_p;

const string P_00010_00011 = assembler::normal_line_prefix_p +
    R"((ISR|TMP)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"((PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))"
    + assembler::normal_line_posfix_p;

const string P_10101 = assembler::normal_line_prefix_p +
    R"((ISR|TMP)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+)"
    R"((\d{1,5}|0[xX][\dA-Fa-f]{1,4}|TMP)\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"((PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))"
    + assembler::normal_line_posfix_p;

const string P_10100 = assembler::normal_line_prefix_p +
    R"(META\s*\[\s*(CALC_RSLT|SM_DATA([0-7]))\s*]\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"((PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))"
    + assembler::normal_line_posfix_p;

const string P_00111 = assembler::normal_line_prefix_p +
    R"((0[xX][\dA-Fa-f]{1,9})\s*,\s+(([0-7])\s*,\s+)?([0-3]))" + assembler::normal_line_posfix_p;

const string P_00101_00110 = assembler::normal_line_prefix_p +
    R"((0[xX][\dA-Fa-f]{1,6})?)" + assembler::normal_line_posfix_p;

const string P_01000_01101_01110_01111 = assembler::normal_line_prefix_p +
    R"()" + assembler::normal_line_posfix_p;

const string P_00100 = assembler::normal_line_prefix_p +
    R"((([1-9][0-9]?|1[0-9]{2}|2[0-4][0-9]|25[0-6])|(CSUM|CRC16|CRC32)|)"
    R"((([1-9][0-9]?|1[0-9]{2}|2[0-4][0-9]|25[0-6])\s*,\s*(CSUM|CRC16|CRC32)))?)"
    + assembler::normal_line_posfix_p;

const string P_01001 = assembler::normal_line_prefix_p +
    R"((POLY|INIT|CTRL\s*,\s+0b000([01]{5})|XOROT)\s*,\s+(\d{1,10}|(0[xX])[\dA-Fa-f]{1,8}))"
    + assembler::normal_line_posfix_p;

const string P_01010_01011_01100 = assembler::normal_line_prefix_p +
    R"((TMP|ISR|PHV)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+(0[xX][\dA-Fa-f]{1,8}))"
    + assembler::normal_line_posfix_p;

const string P_10000 = assembler::normal_line_prefix_p +
    R"(TCAM_KEY\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|10)\s*\}\s*,\s+(0[xX][\dA-Fa-f]{1,10}))"
    + assembler::normal_line_posfix_p;

const string P_10001 = assembler::normal_line_prefix_p +
    R"((ISR\s*\{\s*([0-9]|[12][0-9]|3[0-2])\s*:\s*([1-9]|[123][0-9]|40)\s*\}\s*,\s+)?)"
    R"((META\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-9]|[123][0-9]|40)\s*\}\s*,\s+)?)"
    R"((>>([1-9]|[12][0-9]|3[0-2])\s*,\s+)?->([0-9]|[1-9][0-9]{1,2}|2[0-4][0-9]|25[0-5]))"
    + assembler::normal_line_posfix_p;

const string P_10010_10011 = assembler::normal_line_prefix_p +
    R"((>>([1-9]|[12][0-9]|3[0-2]))?)" + assembler::normal_line_posfix_p;

#endif // PARSER_DEF_H