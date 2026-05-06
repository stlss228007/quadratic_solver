#ifndef NUMBER_PARSER_H
#define NUMBER_PARSER_H

#include "../BigFloat/BigFloat.h"
#include <string>

class NumberParser {
public:
    static bool parse(const std::string& str, BigFloat& out);
};

#endif