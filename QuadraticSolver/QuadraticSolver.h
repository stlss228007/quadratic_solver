#ifndef QUADRATIC_SOLVER_H
#define QUADRATIC_SOLVER_H

#include "../BigFloat/BigFloat.h"
#include "../Complex/Complex.h"
#include <string>
#include <vector>
#include <utility>

struct QuadraticResult {
    std::string status;
    std::vector<BigFloat> real_roots;
    std::vector<Complex> complex_roots;
};

class QuadraticSolver {
public:
    static QuadraticResult solve(const std::string& a_str, const std::string& b_str, const std::string& c_str);
};

#endif