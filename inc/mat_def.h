#ifndef MAT_DEF_H
#define MAT_DEF_H

#include "mat_assembler.h"

typedef union {
    struct {
        u64 opcode: 5;
        u64 offset: 9;
        u64 length: 3;
        u64 rsvd1: 6;
        u64 imm32: 32;
        u64 ad_idx: 6;
        u64 mode: 1;
        u64 dst_slct: 1;
        u64 rsvd2: 1;
    } op_00001;
    struct {
        u64 opcode: 5;
        u64 offset: 9;
        u64 length: 2;
        u64 rsvd1: 7;
        u64 imm16: 16;
        u64 mask: 16;
        u64 ad_idx: 6;
        u64 mode: 1;
        u64 dst_slct: 1;
        u64 rsvd2: 1;
    } op_00010;
    // struct {
    //     u64 opcode: 5;
    //     u64 src_off: 9;
    //     u64 length: 3;
    //     u64 rsvd1: 24;
    //     u64 dst_off: 9;
    //     u64 direction: 2;
    //     u64 rsvd2: 12;
    // } op_00011;
    struct {
        u64 opcode: 5;
        u64 counter_id: 24;
        u64 rsvd1: 26;
        u64 ad_idx: 6;
        u64 mode: 2;
        u64 rsvd2: 1;
    } op_00100;
    struct {
        u64 opcode: 5;
        u64 meter_id: 12;
        u64 rsvd1: 24;
        u64 dst_off: 9;
        u64 rsvd2: 5;
        u64 ad_idx: 6;
        u64 mode: 2;
        u64 rsvd3: 1;
    } op_00101;
    struct {
        u64 opcode: 5;
        u64 flow_id: 24;
        u64 rsvd1: 26;
        u64 ad_idx: 6;
        u64 mode: 2;
        u64 rsvd2: 1;
    } op_00110, op_00111;
    struct {
        u64 opcode: 5;
        u64 offset: 9;
        u64 length: 4;
        u64 rsvd1: 37;
        u64 ad_idx: 6;
        u64 mode: 1;
        u64 rsvd2: 2;
    } op_10100;
    struct {
        u64 opcode: 5;
        u64 src1_off: 9;
        u64 src_mode: 1;
        u64 rsvd1: 8;
        u64 src2_off: 9;
        u64 dst_off: 9;
        u64 dst_slct: 1;
        u64 calc_mode: 4;
        u64 xor_unit: 2;
        u64 calc_len: 6;
        u64 rsvd2: 10;
    } op_10101;
    struct {
        u64 opcode: 5;
        u64 src_off: 9;
        u64 length: 4;
        u64 rsvd1: 23;
        u64 dst_off: 9;
        u64 direction: 2;
        u64 rsvd2: 12;
    } op_10110, op_00011;
    struct {
        u64 opcode: 5;
        u64 offset: 9;
        u64 addr_h36: 36;
        u64 left_shift: 3;
        u64 line_shift: 2;
        u64 ad_idx: 6;
        u64 mode: 1;
        u64 len: 1;
        u64 rsvd: 1;
    } op_10111, op_11000;
    struct {
        u64 opcode;
        u64 imm64_l;
        u64 imm64_h;
    } universe;
    u64 val64;
} machine_code;

// typedef struct _long_mcode {
//     machine_code normal;
//     char extension[];
// } long_mcode;

const string P_00001 = assembler::normal_line_prefix_p +
    R"((\d{1,10}|0[xX][\dA-Fa-f]{1,8}|AD\s*\[\s*([0-9]|[1-5][0-9]|6[0-3])\s*])\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}))"
    + assembler::normal_line_posfix_p;

const string P_00010 = assembler::normal_line_prefix_p +
    R"((\d{1,5}|0[xX][\dA-Fa-f]{1,4}|AD\s*\[\s*([0-9]|[1-5][0-9]|6[0-3])\s*])\s*,\s+)"
    R"((0[xX][\dA-Fa-f]{1,4})\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|)"
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([12])\s*\}))"
    + assembler::normal_line_posfix_p;

const string P_00011 = assembler::normal_line_prefix_p +
    R"((META|PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}\s*,\s+)"
    R"((META|PHV)\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_00100 = assembler::normal_line_prefix_p +
    R"(((0[xX][\dA-Fa-f]{1,6})|AD\s*\[\s*([0-9]|[1-5][0-9]|6[0-3])\s*])?)"
    + assembler::normal_line_posfix_p;

const string P_00101 = assembler::normal_line_prefix_p +
    R"(((0[xX][\dA-Fa-f]{1,3})\s*,\s+|AD\s*\[\s*([0-9]|[1-5][0-9]|6[0-3])\s*]\s*,\s+)?)"
    R"(PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_00110_00111 = assembler::normal_line_prefix_p +
    R"(((0[xX][\dA-Fa-f]{1,6})|AD\s*\[\s*([0-9]|[1-5][0-9]|6[0-3])\s*])?)"
    + assembler::normal_line_posfix_p;

const string P_10100 = assembler::normal_line_prefix_p +
    R"(((0[xX][\dA-Fa-f]{16})(\s*,\s+(0[xX][\dA-Fa-f]{1,16}))?|)"
    R"(AD\s*\{\s*([0-9]|[1-5][0-9]|6[0-3])\s*:\s*([1-4])\s*\})\s*,\s+)"
    R"(PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_10101 = assembler::normal_line_prefix_p +
    R"(PHV\s*(\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-9]|[1-3][0-9]|40)\s*\}|)"
    R"(\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*]\s*,\s+)"
    R"(META\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-9]|[1-3][0-9]|40)\s*\})\s*,\s+)"
    R"((META|PHV)\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_10110 = assembler::normal_line_prefix_p +
    R"((META|PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-9]|1[0-6])\s*\}\s*,\s+)"
    R"((META|PHV)\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])"
    + assembler::normal_line_posfix_p;

const string P_10111_11000 = assembler::normal_line_prefix_p +
    R"(PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*]\s*,\s+)"
    R"((0[xX][\dA-Fa-f]{1,9}|AD\s*\{\s*([0-9]|[1-5][0-9]|6[0-3])\s*:\s*([0-7])\s*\})\s*,\s+([0-3]))"
    + assembler::normal_line_posfix_p;

#endif // MAT_DEF_H