#ifndef COMPLEX_H
#define COMPLEX_H

#include "../BigFloat/BigFloat.h"
#include <string>

class Complex {
public:
    Complex() : re_(0), im_(0) {}
    Complex(const BigFloat& re, const BigFloat& im) : re_(re), im_(im) {}

    const BigFloat& real() const { return re_; }
    const BigFloat& imag() const { return im_; }

    std::string toString() const;

    Complex operator-() const { return Complex(-re_, -im_); }

private:
    BigFloat re_;
    BigFloat im_;
};

#endif