/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef INC_PRE_PROC_PREPROCESSOR_H_
#define INC_PRE_PROC_PREPROCESSOR_H_

#include <filesystem>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <utility>

using str_u16_pair = std::pair<std::string, std::uint16_t>;

class preprocessor {
    friend class assembler;
    friend class deparser_assembler;

 public:
    preprocessor() = default;
    ~preprocessor() = default;

    preprocessor(const preprocessor&) = delete;
    preprocessor(preprocessor&&) = delete;
    preprocessor& operator=(const preprocessor&) = delete;
    preprocessor& operator=(preprocessor&&) = delete;

    int handle(const std::filesystem::path&);

 private:
    int process_single_entry(const std::filesystem::path&);
    int process_multi_entries(const std::filesystem::path&);
    int process_imported_file(const std::filesystem::path&, const std::string&, std::uint16_t&, std::ofstream&);
    int process_included_file(const std::filesystem::path&);
    std::string replace_macros(std::string);

    std::unordered_map<std::string, std::string> token_to_val;
    std::unordered_map<std::string, str_u16_pair> dst_cline_2_src_fline;
};

#endif  // INC_PRE_PROC_PREPROCESSOR_H_
