/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_DEPARSER_DEF_H_
#define INC_DEPARSER_DEF_H_

#include <cstdint>

typedef union {
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t rsvd: 25;
        std::uint32_t src_slct: 2;
    } op_00001;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t offset: 9;
        std::uint32_t length: 12;
        std::uint32_t rsvd: 2;
        std::uint32_t off_ctrl: 1;
        std::uint32_t len_ctrl: 1;
        std::uint32_t src_slct: 2;
    } op_00010;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t offset: 9;
        std::uint32_t length: 12;
        std::uint32_t calc_mode: 2;
        std::uint32_t off_ctrl: 1;
        std::uint32_t len_ctrl: 1;
        std::uint32_t src_slct: 2;
    } op_00011, op_00100;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t offset: 9;
        std::uint32_t length: 2;
        std::uint32_t rsvd: 16;
    } op_00101;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t dst_slct: 3;
        std::uint32_t value: 16;
        std::uint32_t ctrl_mode: 8;
    } op_00110, op_00111;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t src_off: 9;
        std::uint32_t src_len: 3;
        std::uint32_t dst_off: 9;
        std::uint32_t dst_len: 4;
        std::uint32_t src_slct: 2;
    } op_01000, op_01001;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t src_off: 9;
        std::uint32_t src_len: 9;
        std::uint32_t dst_off: 9;
    } op_01010, op_01100, op_11100;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t rsvd: 17;
        std::uint32_t dst_slct: 1;
        std::uint32_t dst_off: 9;
    } op_01011, op_01101, op_11101;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t offset: 9;
        std::uint32_t length: 12;
        std::uint32_t mask_flg: 1;
        std::uint32_t mask_en: 1;
        std::uint32_t off_ctrl: 1;
        std::uint32_t len_ctrl: 1;
        std::uint32_t src_slct: 2;
    } op_01110, op_01111, op_10000;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t src_off: 9;
        std::uint32_t src_len: 6;
        std::uint32_t calc_unit: 2;
        std::uint32_t dst_slct: 1;
        std::uint32_t dst_off: 9;
    } op_10001;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t rsvd: 15;
        std::uint32_t calc_unit: 2;
        std::uint32_t dst_slct: 1;
        std::uint32_t dst_off: 9;
    } op_10010;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t src_off: 9;
        std::uint32_t src_len: 6;
        std::uint32_t hash_type: 2;
        std::uint32_t dst_slct: 1;
        std::uint32_t dst_off: 9;
    } op_10011;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t rsvd: 15;
        std::uint32_t hash_type: 2;
        std::uint32_t dst_slct: 1;
        std::uint32_t dst_off: 9;
    } op_10100;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t cmd_type: 4;
        std::uint64_t cntr_idx: 12;
        std::uint64_t imm8: 8;
        std::uint64_t phv_off: 9;
        std::uint64_t lck_flg: 1;
        std::uint64_t ulk_flg: 1;
        std::uint64_t cntr_idx_slct: 1;
        std::uint64_t imm_slct: 1;
        std::uint64_t cntr_width: 1;
        std::uint64_t rsvd: 21;
    } op_10101;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t direction: 2;
        std::uint32_t src_off: 9;
        std::uint32_t dst_off: 9;
        std::uint32_t length: 2;
        std::uint32_t rsvd: 5;
    } op_10110;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t select: 2;
        std::uint32_t address: 16;
        std::uint32_t rsvd: 9;
    } op_10111;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t target_1: 16;
        std::uint64_t target_2: 16;
        std::uint64_t jump_mode: 4;
        std::uint64_t src_slct: 4;
        std::uint64_t src_off: 12;
        std::uint64_t rsvd: 7;
    } op_11001;
    struct {
        std::uint32_t opcode: 5;
        std::uint32_t rsvd: 27;
    } universe;
    std::uint32_t val32;
    std::uint64_t val64;
} machine_code;

constexpr auto P_00001 = R"()";

constexpr auto P_00010_00011_00100 =
    R"((PHV|HDR|PLD)?\s*(\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])?\s*:\s*)"
    R"(([1-9][0-9]{0,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])?\s*\})?(\s*,\s+)?(CSUM|CRC16|CRC32)?)";

constexpr auto P_00101 =
    R"(PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\}\s*,\s+TMP)";

constexpr auto P_00110_00111 =
    R"((TMP|CNDR|OFFR|LENR|POLY|INIT|CTRL\s*,\s+0b00([01]{6})|XOROT)\s*,\s+(\d{1,5}|0[xX][\dA-F]{1,4}))";

constexpr auto P_01000_01001 =
    R"(TMP\s*,\s+(PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\})"
    R"((\s*,\s+PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([2-9])\s*\})?|CNDR|OFFR|LENR))";

constexpr auto P_01010_01100_11100 =
    R"(PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*)"
    R"(([1-9][0-9]?|[1-4][0-9]{2}|50[0-9]|51[01])\s*\}\s*,\s+)"
    R"(PHV\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_01011_01101_11101 =
    R"((PHV\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])?)";

constexpr auto P_01110_01111_10000 =
    R"((PHV|HDR|PLD)\s*(\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])?\s*:\s*)"
    R"(([1-9][0-9]{0,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])?\s*\})?)";

constexpr auto P_10001_10011 =
    R"(PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-5][0-9]?|6[0-3])\s*\}\s*,\s+)"
    R"((PHV|META)\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_10010_10100 =
    R"((PHV|META)\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_10101 =
    R"((CNTR\s*\[\s*([0-9]|[1-9][0-9]{1,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])\s*]\s*,\s+)?)"
    R"((([1-9][0-9]{0,2}|2[0-4][0-9]|25[0-5])\s*,\s+)?)"
    R"(((PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*(32|64)\s*\})|(TMP))(\s*,\s+(LCK|ULK))?)";

constexpr auto P_10110 =
    R"((PHV|META)\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\}\s*,\s+)"
    R"((PHV|META)\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])])";

constexpr auto P_10111 = R"((0[xX][\dA-Fa-f]{1,4})?)";

constexpr auto P_11001 =
    R"((([-])?\d{1,5}|0[xX][\dA-Fa-f]{1,4})(\s*,\s+(([-])?\d{1,5}|0[xX][\dA-Fa-f]{1,4})\s*,\s+)"
    R"((CONDR|PHV(1|8|16|32)\s*\[\s*([0-9]|[1-9][0-9]{1,2}|[1-3][0-9]{3}|40[0-8][0-9]|409[0-5])\s*]))?)";

constexpr auto P_11000_11010_11011 = P_00001;

#endif  // INC_DEPARSER_DEF_H_
