/**
 * Copyright [2024] <wangdc1111@gmail.com>
 */
#ifndef INC_GLOBAL_DEF_H_
#define INC_GLOBAL_DEF_H_

constexpr auto COMMENT_EMPTY_LINE_P = R"(^\s*\/\/.*[\n\r]?$|^\s*$)";

constexpr auto IR_PATH = "irs/";

constexpr auto IR_STR = "i";

#include <string>
#include <algorithm>

inline std::string& ltrim(std::string &str) {  // NOLINT
    str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), isspace));
    return str;
}

inline std::string& rtrim(std::string &str) {  // NOLINT
    str.erase((std::find_if_not(str.rbegin(), str.rend(), isspace)).base(), str.end());
    return str;
}

inline std::string& trim_ends(std::string &str) {  // NOLINT
    return ltrim(rtrim(str));
}

inline std::string& trim_all(std::string &str) {  // NOLINT
    str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
    return str;
}

#endif  // INC_GLOBAL_DEF_H_
