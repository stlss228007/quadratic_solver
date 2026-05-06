#include "QuadraticSolver.h"
#include "../NumberParser/NumberParser.h"

QuadraticResult QuadraticSolver::solve(const std::string& a_str, const std::string& b_str, const std::string& c_str) {
    QuadraticResult res;
    BigFloat a, b, c;
    if (!NumberParser::parse(a_str, a) ||
        !NumberParser::parse(b_str, b) ||
        !NumberParser::parse(c_str, c)) {
        res.status = "WRONG";
        return res;
    }

    if (a.isZero()) {
        if (b.isZero()) {
            if (c.isZero()) {
                res.status = "INF";
            } else {
                res.status = "NO SOLUTION";
            }
            return res;
        } else {
            BigFloat x = (c * BigFloat("-1")) / b;
            res.status = "OK";
            res.real_roots = {x};
            return res;
        }
    }

    BigFloat four("4");
    BigFloat two("2");
    BigFloat neg_one("-1");

    BigFloat D = b * b - four * a * c;

    if (D > BigFloat("0")) {
        BigFloat sqrtD = D.sqrt();
        BigFloat two_a = two * a;
        BigFloat x1 = (neg_one * b - sqrtD) / two_a;
        BigFloat x2 = (neg_one * b + sqrtD) / two_a;
        if (x1 > x2) std::swap(x1, x2);
        res.status = "OK";
        res.real_roots = {x1, x2};
    } else if (D == BigFloat("0")) {
        BigFloat x = (neg_one * b) / (two * a);
        res.status = "OK";
        res.real_roots = {x};
    } else {
        BigFloat sqrt_D_abs = (-D).sqrt();
        BigFloat two_a = two * a;
        BigFloat re = (neg_one * b) / two_a;
        BigFloat im = sqrt_D_abs / two_a;
        
        if (im < BigFloat("0")) im = im * neg_one;
        res.status = "OK";
        res.complex_roots = {Complex(re, im), Complex(re, neg_one * im)};
    }

    return res;
}