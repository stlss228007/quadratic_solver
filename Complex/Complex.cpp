#include "Complex.h"

std::string Complex::toString() const {
    std::string result;
    bool re_zero = re_.isZero();
    bool im_zero = im_.isZero();

    if (re_zero && im_zero) return "0";

    if (!re_zero) {
        result = re_.toString();
    }
    if (!im_zero) {
        if (!re_zero && im_ > BigFloat("0")) result += "+";
        if (im_ < BigFloat("0")) result += "-";
        BigFloat abs_im = im_.abs();
        if (abs_im.toString() != "1") result += abs_im.toString();
        result += "i";
    }
    return result;
}