#include "NumberParser.h"
#include <cctype>
#include <string>

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

bool NumberParser::parse(const std::string& str, BigFloat& out) {
    std::string s = trim(str);
    if (s.empty()) return false;

    size_t pos = 0;
    bool negative = false;
    if (s[pos] == '+') {
        ++pos;
    } else if (s[pos] == '-') {
        negative = true;
        ++pos;
    }

    std::string mantissa;
    bool has_dot = false;
    bool has_digits = false;

    while (pos < s.size() && (std::isdigit(s[pos]) || s[pos] == '.')) {
        if (s[pos] == '.') {
            if (has_dot) return false;
            has_dot = true;
            mantissa.push_back('.');
            ++pos;
            continue;
        }
        mantissa.push_back(s[pos]);
        has_digits = true;
        ++pos;
    }

    if (!has_digits) return false;

    long exponent = 0;
    if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
        ++pos;
        bool exp_negative = false;
        if (pos < s.size() && s[pos] == '+') {
            ++pos;
        } else if (pos < s.size() && s[pos] == '-') {
            exp_negative = true;
            ++pos;
        }
        if (pos >= s.size() || !std::isdigit(s[pos])) return false;
        long exp_val = 0;
        while (pos < s.size() && std::isdigit(s[pos])) {
            exp_val = exp_val * 10 + (s[pos] - '0');
            ++pos;
        }
        exponent = exp_negative ? -exp_val : exp_val;
    }

    if (pos != s.size()) return false;

    try {
        BigFloat num(mantissa);
        if (exponent != 0) {
            num = num.shift(exponent);
        }
        if (negative) {
            num = num * BigFloat("-1");
        }
        out = num;
        return true;
    } catch (...) {
        return false;
    }
}