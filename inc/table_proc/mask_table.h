/**
 * Copyright [2024] <wangdc1111@gmail.com>
 */
#ifndef INC_TABLE_PROC_MASK_TABLE_H_
#define INC_TABLE_PROC_MASK_TABLE_H_

#include <string>
#include <vector>
#include <memory>
#include "table.h"  // NOLINT

class mask_table : public table {
 public:
    mask_table(const std::string &in, const std::string &ot) : table(in, ot) {}
    mask_table() = default;
    ~mask_table() = default;

    mask_table(const mask_table&) = delete;
    mask_table(mask_table&&) = delete;
    mask_table& operator=(const mask_table&) = delete;
    mask_table& operator=(mask_table&&) = delete;

    int generate_table_data(const std::shared_ptr<assembler>&) override;

 private:
    // std::vector<std::string> mask_strings;

    static const char* mask_line_pattern;
    static const char* mask_off_len_pattern;
};

#endif  // INC_TABLE_PROC_MASK_TABLE_H_
