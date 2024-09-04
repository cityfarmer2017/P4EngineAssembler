/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_PARSER_DEF_H_
#define INC_PARSER_DEF_H_

#include <cstdint>

typedef union {
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t src_off: 5;
        std::uint64_t src_len: 5;
        std::uint64_t src_slct: 1;
        std::uint64_t imm32: 32;
        std::uint64_t dst_off: 9;
        std::uint64_t dst_len: 4;
        std::uint64_t dst_slct: 2;
        std::uint64_t last_flg: 1;
    } op_00001;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t src_off: 5;
        std::uint64_t src_len: 5;
        std::uint64_t src_slct: 1;
        std::uint64_t byte_rvs_mode: 2;
        std::uint64_t rsvd: 30;
        std::uint64_t dst_off: 9;
        std::uint64_t dst_len: 4;
        std::uint64_t dst_slct: 2;
        std::uint64_t last_flg: 1;
    } op_00010, op_00011;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t length: 8;
        std::uint64_t calc_slct: 3;
        std::uint64_t inline_flg: 1;
        std::uint64_t rsvd1: 30;
        std::uint64_t phv_flg: 1;
        std::uint64_t phv_off: 9;
        std::uint64_t rsvd2: 6;
        std::uint64_t last_flg : 1;
    } op_00100;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t inline_flg: 1;
        std::uint64_t rsvd1: 10;
        std::uint64_t flow_id : 24;
        std::uint64_t rsvd2: 23;
        std::uint64_t last_flg: 1;
    } op_00101, op_00110;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t inline_flg: 1;
        std::uint64_t rsvd1: 10;
        std::uint64_t addr : 36;
        std::uint64_t meta_left_shift: 3;
        std::uint64_t line_off: 2;
        std::uint64_t len_flg: 1;
        std::uint64_t rsvd2: 5;
        std::uint64_t last_flg: 1;
    } op_00111;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t rsvd: 58;
        std::uint64_t last_flg: 1;
    } op_01000, op_01101, op_01110, op_01111;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t calc_slct: 3;
        std::uint64_t ctrl_mode: 8;
        std::uint64_t value: 32;
        std::uint64_t rsvd: 15;
        std::uint64_t last_flg: 1;
    } op_01001;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t rsvd1: 2;
        std::uint64_t offset: 7;
        std::uint64_t length: 5;
        std::uint64_t src_slct: 2;
        std::uint64_t mask: 32;
        std::uint64_t mask_flg: 1;
        std::uint64_t rsvd2: 9;
        std::uint64_t last_flg: 1;
    } op_01010, op_01011, op_01100;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t offset: 5;
        std::uint64_t length: 5;
        std::uint64_t rsvd1: 1;
        std::uint64_t imm40: 40;
        std::uint64_t sign_flg: 1;
        std::uint64_t alu_mode: 3;
        std::uint64_t rsvd2: 3;
        std::uint64_t last_flg: 1;
    } op_10000;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t shift_val: 6;
        std::uint64_t sub_state: 8;
        std::uint64_t isr_off: 6;
        std::uint64_t isr_len: 6;
        std::uint64_t meta_intrinsic: 1;
        std::uint64_t meta_off: 9;
        std::uint64_t meta_len: 6;
        std::uint64_t rsvd: 16;
        std::uint64_t last_flg: 1;
    } op_10001;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t shift_val: 6;
        std::uint64_t rsvd: 52;
        std::uint64_t last_flg: 1;
    } op_10010, op_10011;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t rsvd1: 7;
        std::uint64_t src_slct: 4;
        std::uint64_t rsvd2: 32;
        std::uint64_t dst_off: 9;
        std::uint64_t dst_len: 4;
        std::uint64_t dst_slct: 2;
        std::uint64_t last_flg: 1;
    } op_10100;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t src0_off: 5;
        std::uint64_t src0_len: 5;
        std::uint64_t src0_slct: 1;
        std::uint64_t alu_mode: 2;
        std::uint64_t imm16: 16;
        std::uint64_t src1_slct: 1;
        std::uint64_t rsvd: 13;
        std::uint64_t dst_off: 9;
        std::uint64_t dst_len: 4;
        std::uint64_t dst_slct: 2;
        std::uint64_t last_flg: 1;
    } op_10101;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t rsvd: 58;
        std::uint64_t last_flg: 1;
    } universe;
    std::uint64_t val64;
} machine_code;

constexpr auto P_00001 =
    R"((\d{1,10}|0[xX][\dA-Fa-f]{1,8})\s*,\s+)"
    R"(((ISR|TMP)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+)?)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"((PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))";

constexpr auto P_00010_00011 =
    R"((ISR|TMP)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"((PHV)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))";

constexpr auto P_10101 =
    R"((ISR|TMP)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+)"
    R"((\d{1,5}|0[xX][\dA-Fa-f]{1,4}|TMP)\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))";

constexpr auto P_10100 =
    R"(META\s*\[\s*(CALC_RSLT|SM_DATA([0-7]))\s*]\s*,\s+)"
    R"(((META)\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"(PHV\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|TMP))";

constexpr auto P_00111 = R"((0[xX][\dA-Fa-f]{1,9})\s*,\s+(([0-7])\s*,\s+)?([0-3]))";

constexpr auto P_00101_00110 = R"((0[xX][\dA-Fa-f]{1,6})?)";

constexpr auto P_01000_01101_01110_01111 = R"()";

constexpr auto P_00100 =
    R"((([1-9][0-9]?|1[0-9]{2}|2[0-4][0-9]|25[0-6])|(CSUM|CRC16|CRC32)|)"
    R"(([1-9][0-9]?|1[0-9]{2}|2[0-4][0-9]|25[0-6])\s*,\s*(CSUM|CRC16|CRC32)|)"
    R"(([1-9][0-9]?|1[0-9]{2}|2[0-4][0-9]|25[0-6])\s*,\s*PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*]|)"
    R"(([1-9][0-9]?|1[0-9]{2}|2[0-4][0-9]|25[0-6])\s*,\s*(CSUM|CRC16|CRC32)\s*,\s*PHV\s*\[\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])?)";

constexpr auto P_01001 =
    R"((POLY|INIT|CTRL\s*,\s+0b000([01]{5})|XOROT)\s*,\s+(\d{1,10}|(0[xX])[\dA-Fa-f]{1,8}))";

constexpr auto P_01010_01011_01100 =
    R"((TMP|ISR|PHV)\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|[12][0-9]|3[0-2])\s*\}\s*,\s+(0[xX][\dA-Fa-f]{1,8}))";

constexpr auto P_10000 =
    R"(TCAM_KEY\s*\{\s*([0-9]|[12][0-9]|3[01])\s*:\s*([1-9]|10)\s*\}\s*,\s+(0[xX][\dA-Fa-f]{1,10}))";

constexpr auto P_10001 =
    R"((ISR\s*\{\s*([0-9]|[12][0-9]|3[0-2])\s*:\s*([1-9]|[123][0-9]|40)\s*\}\s*,\s+)?)"
    R"((META\s*\{\s*([0-9]|[1-9][0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-9]|[123][0-9]|40)\s*\}\s*,\s+)?)"
    R"((>>([1-9]|[12][0-9]|3[0-2])\s*,\s+)?->([0-9]|[1-9][0-9]{1,2}|2[0-4][0-9]|25[0-5]))";

constexpr auto P_10010_10011 = R"((>>([1-9]|[12][0-9]|3[0-2]))?)";

#endif  // INC_PARSER_DEF_H_
