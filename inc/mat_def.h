/**
 * Copyright [2024] <wangdc1111@gmail.com>
 */
#ifndef INC_MAT_DEF_H_
#define INC_MAT_DEF_H_

#include <cstdint>

typedef union {
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t offset: 9;
        std::uint64_t length: 3;
        std::uint64_t rsvd1: 6;
        std::uint64_t imm32: 32;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 1;
        std::uint64_t dst_slct: 1;
        std::uint64_t rsvd2: 1;
    } op_00001;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t offset: 9;
        std::uint64_t length: 2;
        std::uint64_t rsvd1: 7;
        std::uint64_t imm16: 16;
        std::uint64_t mask: 16;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 1;
        std::uint64_t dst_slct: 1;
        std::uint64_t rsvd2: 1;
    } op_00010;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t counter_id: 24;
        std::uint64_t rsvd1: 26;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 2;
        std::uint64_t rsvd2: 1;
    } op_00100;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t meter_id: 12;
        std::uint64_t rsvd1: 24;
        std::uint64_t dst_off: 9;
        std::uint64_t rsvd2: 5;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 2;
        std::uint64_t rsvd3: 1;
    } op_00101;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t flow_id: 24;
        std::uint64_t rsvd1: 26;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 2;
        std::uint64_t rsvd2: 1;
    } op_00110, op_00111;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t offset: 9;
        std::uint64_t length: 4;
        std::uint64_t rsvd1: 37;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 1;
        std::uint64_t rsvd2: 2;
    } op_10100;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t src1_off: 9;
        std::uint64_t src_mode: 1;
        std::uint64_t rsvd1: 8;
        std::uint64_t src2_off: 9;
        std::uint64_t dst_off: 9;
        std::uint64_t dst_slct: 1;
        std::uint64_t calc_mode: 4;
        std::uint64_t xor_unit: 2;
        std::uint64_t rsvd2: 7;
        std::uint64_t ad_idx: 6;
        std::uint64_t rsvd3: 3;
    } op_10101;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t src_off: 9;
        std::uint64_t length: 4;
        std::uint64_t rsvd1: 23;
        std::uint64_t dst_off: 9;
        std::uint64_t direction: 2;
        std::uint64_t swap_mode: 3;
        std::uint64_t rsvd2: 12;
    } op_10110, op_00011;
    struct {
        std::uint64_t opcode: 5;
        std::uint64_t offset: 9;
        std::uint64_t addr_h36: 36;
        std::uint64_t left_shift: 3;
        std::uint64_t line_shift: 2;
        std::uint64_t ad_idx: 6;
        std::uint64_t mode: 1;
        std::uint64_t len: 1;
        std::uint64_t rsvd: 1;
    } op_10111, op_11000;
    struct {
        std::uint64_t opcode;
        std::uint64_t imm64_l;
        std::uint64_t imm64_h;
    } universe;
    std::uint64_t val64;
} machine_code;

constexpr auto P_00001 =
    R"((\d{1,10}|0[xX][\dA-Fa-f]{1,8}|AD\s*\[\s*([12]?[0-9]|3[0-6])\s*])\s*,\s+)"
    R"(((META)\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}|)"
    R"(PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}))";

constexpr auto P_00010 =
    R"((\d{1,5}|0[xX][\dA-Fa-f]{1,4}|AD\s*\[\s*([12]?[0-9]|3[0-8])\s*])\s*,\s+)"
    R"((0[xX][\dA-Fa-f]{1,4})\s*,\s+)"
    R"(((META)\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-4])\s*\}|)"
    R"(PHV\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([12])\s*\}))";

constexpr auto P_00011 =
    R"((META|PHV)\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-8])\s*\}\s*,\s+)"
    R"((META|PHV)\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_00100 =
    R"(((0[xX][\dA-Fa-f]{1,6})|AD\s*\[\s*([12]?[0-9]|3[0-7])\s*])?)";

constexpr auto P_00101 =
    R"(((0[xX][\dA-Fa-f]{1,3})\s*,\s+|AD\s*\[\s*([12]?[0-9]|3[0-8])\s*]\s*,\s+)?)"
    R"(PHV\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_00110_00111 =
    R"(((0[xX][\dA-Fa-f]{1,6})|AD\s*\[\s*([12]?[0-9]|3[0-7])\s*])?)";

constexpr auto P_10100 =
    R"(((0[xX][\dA-Fa-f]{16})(\s*,\s+(0[xX][\dA-Fa-f]{1,16}))?|)"
    R"(AD\s*\{\s*([1?[0-9]|2[0-4])\s*:\s*([1-4])\s*\})\s*,\s+)"
    R"(PHV\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_10101 =
    R"(PHV\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*]\s*,)"
    R"((\s+META\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])]\s*,)?(\s+AD\s*\[\s*([1-3]?[0-9])\s*]\s*,)?)"
    R"(\s+(META|PHV)\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_10110 =
    R"((META|PHV)\s*\{\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*:\s*([1-9]|1[0-6])\s*\}\s*,\s+)"
    R"((META|PHV)\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*])";

constexpr auto P_10111_11000 =
    R"(PHV\s*\[\s*([1-9]?[0-9]|[1-4][0-9]{2}|50[0-9]|51[01])\s*]\s*,\s+)"
    R"((0[xX][\dA-Fa-f]{1,9}|AD\s*\{\s*([12]?[0-9]|3[0-7])\s*:\s*([0-7])\s*\})\s*,\s+([0-3]))";

#endif  // INC_MAT_DEF_H_
