#include "BigFloat.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <iostream>

namespace {
    constexpr size_t KARATSUBA_THRESHOLD = 64;
    constexpr int MAX_FRAC_DIGITS_FROM_DOUBLE = 20;
    constexpr int RECIPROCAL_ROUND_EXTRA = 2;
    constexpr size_t DIVISION_PRECISION_EXTRA = 128;
}

BigFloat::BigFloat() : sign_(true), integer_{0}, fractional_{} {}

BigFloat::BigFloat(const std::string& str) {
    if (str.empty()) throw std::invalid_argument("Empty string");
    size_t pos = 0;
    sign_ = true;
    if (str[pos] == '+') ++pos;
    else if (str[pos] == '-') { sign_ = false; ++pos; }

    bool has_point = false;
    bool has_digits = false;
    std::string int_part, frac_part;
    while (pos < str.size()) {
        char ch = str[pos];
        if (ch == '.') {
            if (has_point) break;
            has_point = true;
            ++pos;
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            if (!has_point) int_part.push_back(ch);
            else frac_part.push_back(ch);
            has_digits = true;
            ++pos;
        } else {
            throw std::invalid_argument("Invalid character");
        }
    }
    if (!has_digits) throw std::invalid_argument("No digits");
    if (int_part.empty()) int_part = "0";
    else {
        size_t nz = int_part.find_first_not_of('0');
        if (nz == std::string::npos) int_part = "0";
        else int_part = int_part.substr(nz);
    }
    for (char c : int_part) integer_.push_back(c - '0');
    for (char c : frac_part) fractional_.push_back(c - '0');
    normalize();
}

BigFloat::BigFloat(long double value) {
    if (value < 0) { sign_ = false; value = -value; }
    else sign_ = true;
    if (value == 0.0L) {
        integer_ = {0};
        fractional_.clear();
        return;
    }
    long double intpart;
    long double frac = std::modf(value, &intpart);
    uint64_t int_val = static_cast<uint64_t>(intpart);
    if (int_val == 0) integer_.push_back(0);
    else {
        std::vector<char> digits;
        while (int_val > 0) {
            digits.push_back(static_cast<char>(int_val % 10));
            int_val /= 10;
        }
        std::reverse(digits.begin(), digits.end());
        integer_ = std::move(digits);
    }
    for (int i = 0; i < MAX_FRAC_DIGITS_FROM_DOUBLE; ++i) {
        frac *= 10.0L;
        long double digit;
        frac = std::modf(frac, &digit);
        fractional_.push_back(static_cast<char>(digit));
        if (frac == 0.0L) break;
    }
    normalize();
}

BigFloat BigFloat::operator+(const BigFloat& other) const {
    BigFloat result;
    if (sign_ == other.sign_) {
        result = addAbs(*this, other);
        result.sign_ = sign_;
    } else {
        int cmp = compareAbs(other);
        if (cmp == 0) {
            result.sign_ = true;
            result.integer_ = {0};
            result.fractional_.clear();
        } else if (cmp > 0) {
            result = subAbs(*this, other);
            result.sign_ = sign_;
        } else {
            result = subAbs(other, *this);
            result.sign_ = other.sign_;
        }
    }
    result.normalize();
    return result;
}

BigFloat BigFloat::operator-(const BigFloat& other) const {
    BigFloat neg_other = other;
    neg_other.sign_ = !other.sign_;
    return *this + neg_other;
}

BigFloat BigFloat::operator-() const {
    BigFloat res = *this;
    res.sign_ = !sign_;
    return res;
}

BigFloat& BigFloat::operator+=(const BigFloat& other) { *this = *this + other; return *this; }
BigFloat& BigFloat::operator-=(const BigFloat& other) { *this = *this - other; return *this; }
BigFloat& BigFloat::operator*=(const BigFloat& other) { *this = *this * other; return *this; }
BigFloat& BigFloat::operator/=(const BigFloat& other) { *this = *this / other; return *this; }

bool BigFloat::operator==(const BigFloat& other) const {
    if (sign_ != other.sign_) return false;
    return compareAbs(other) == 0;
}
bool BigFloat::operator!=(const BigFloat& other) const { return !(*this == other); }
bool BigFloat::operator<(const BigFloat& other) const {
    if (sign_ != other.sign_) return sign_ < other.sign_;
    if (sign_ == true) return compareAbs(other) < 0;
    else return compareAbs(other) > 0;
}
bool BigFloat::operator<=(const BigFloat& other) const { return (*this < other) || (*this == other); }
bool BigFloat::operator>(const BigFloat& other) const { return !(*this <= other); }
bool BigFloat::operator>=(const BigFloat& other) const { return !(*this < other); }

std::string BigFloat::toString() const {
    if (isZero()) return "0";
    std::string result;
    if (!sign_) result += '-';
    for (char d : integer_) result.push_back('0' + d);
    if (!fractional_.empty()) {
        result.push_back('.');
        for (char d : fractional_) result.push_back('0' + d);
    }
    return result;
}

void BigFloat::normalize() {
    removeLeadingZeros();
    removeTrailingZeros();
    if (integer_.size() == 1 && integer_[0] == 0 && fractional_.empty()) sign_ = true;
}

void BigFloat::round(size_t precision) {
    if (fractional_.size() <= precision) return;
    bool need_carry = (fractional_[precision] >= 5);
    fractional_.resize(precision);
    if (need_carry) {
        for (int i = (int)fractional_.size() - 1; i >= 0; --i) {
            if (++fractional_[i] < 10) break;
            fractional_[i] = 0;
        }
        bool all_zero = true;
        for (char c : fractional_) if (c != 0) { all_zero = false; break; }
        if (all_zero) {
            fractional_.clear();
            for (int i = (int)integer_.size() - 1; i >= 0; --i) {
                if (++integer_[i] < 10) break;
                integer_[i] = 0;
                if (i == 0) integer_.insert(integer_.begin(), 1);
            }
        }
    }
    normalize();
}

bool BigFloat::isZero() const {
    return (integer_.size() == 1 && integer_[0] == 0 && fractional_.empty());
}

BigFloat BigFloat::abs() const {
    BigFloat res = *this;
    res.sign_ = true;
    return res;
}

BigFloat BigFloat::shift(int n) const {
    if (n == 0) return *this;
    BigFloat res = *this;
    if (n > 0) {
        int k = n;
        while (k > 0 && !res.fractional_.empty()) {
            res.integer_.push_back(res.fractional_.front());
            res.fractional_.erase(res.fractional_.begin());
            --k;
        }
        if (k > 0) {
            res.integer_.insert(res.integer_.end(), k, 0);
        }
    } else {
        int k = -n;
        while (k > 0 && !res.integer_.empty() && !(res.integer_.size() == 1 && res.integer_[0] == 0)) {
            char last = res.integer_.back();
            res.integer_.pop_back();
            if (res.integer_.empty()) res.integer_.push_back(0);
            res.fractional_.insert(res.fractional_.begin(), last);
            --k;
        }
        if (k > 0) {
            res.fractional_.insert(res.fractional_.begin(), k, 0);
        }
    }
    res.normalize();
    return res;
}

int BigFloat::compareAbs(const BigFloat& other) const {
    if (integer_.size() != other.integer_.size()) return integer_.size() < other.integer_.size() ? -1 : 1;
    for (size_t i = 0; i < integer_.size(); ++i) {
        if (integer_[i] != other.integer_[i]) return integer_[i] < other.integer_[i] ? -1 : 1;
    }
    size_t max_frac = std::max(fractional_.size(), other.fractional_.size());
    for (size_t i = 0; i < max_frac; ++i) {
        char d1 = (i < fractional_.size()) ? fractional_[i] : 0;
        char d2 = (i < other.fractional_.size()) ? other.fractional_[i] : 0;
        if (d1 != d2) return d1 < d2 ? -1 : 1;
    }
    return 0;
}

BigFloat BigFloat::addAbs(const BigFloat& a, const BigFloat& b) const {
    BigFloat res;
    res.sign_ = true;
    size_t max_frac = std::max(a.fractional_.size(), b.fractional_.size());
    std::vector<char> frac1 = a.fractional_, frac2 = b.fractional_;
    frac1.resize(max_frac, 0);
    frac2.resize(max_frac, 0);
    int carry = 0;
    std::vector<char> new_frac(max_frac, 0);
    for (size_t i = max_frac; i-- > 0; ) {
        int sum = frac1[i] + frac2[i] + carry;
        new_frac[i] = sum % 10;
        carry = sum / 10;
    }
    size_t max_int = std::max(a.integer_.size(), b.integer_.size());
    std::vector<char> int1 = a.integer_, int2 = b.integer_;
    int1.insert(int1.begin(), max_int - int1.size(), 0);
    int2.insert(int2.begin(), max_int - int2.size(), 0);
    std::vector<char> new_int(max_int, 0);
    for (size_t i = max_int; i-- > 0; ) {
        int sum = int1[i] + int2[i] + carry;
        new_int[i] = sum % 10;
        carry = sum / 10;
    }
    if (carry) new_int.insert(new_int.begin(), static_cast<char>(carry));
    while (new_int.size() > 1 && new_int[0] == 0) new_int.erase(new_int.begin());
    while (!new_frac.empty() && new_frac.back() == 0) new_frac.pop_back();
    res.integer_ = std::move(new_int);
    res.fractional_ = std::move(new_frac);
    return res;
}

BigFloat BigFloat::subAbs(const BigFloat& a, const BigFloat& b) const {
    BigFloat res;
    res.sign_ = true;
    size_t max_frac = std::max(a.fractional_.size(), b.fractional_.size());
    std::vector<char> frac1 = a.fractional_, frac2 = b.fractional_;
    frac1.resize(max_frac, 0);
    frac2.resize(max_frac, 0);
    int borrow = 0;
    std::vector<char> new_frac(max_frac, 0);
    for (size_t i = max_frac; i-- > 0; ) {
        int diff = frac1[i] - frac2[i] - borrow;
        if (diff < 0) { diff += 10; borrow = 1; }
        else borrow = 0;
        new_frac[i] = static_cast<char>(diff);
    }
    size_t max_int = std::max(a.integer_.size(), b.integer_.size());
    std::vector<char> int1 = a.integer_, int2 = b.integer_;
    int1.insert(int1.begin(), max_int - int1.size(), 0);
    int2.insert(int2.begin(), max_int - int2.size(), 0);
    std::vector<char> new_int(max_int, 0);
    for (size_t i = max_int; i-- > 0; ) {
        int diff = int1[i] - int2[i] - borrow;
        if (diff < 0) { diff += 10; borrow = 1; }
        else borrow = 0;
        new_int[i] = static_cast<char>(diff);
    }
    while (new_int.size() > 1 && new_int[0] == 0) new_int.erase(new_int.begin());
    while (!new_frac.empty() && new_frac.back() == 0) new_frac.pop_back();
    res.integer_ = std::move(new_int);
    res.fractional_ = std::move(new_frac);
    return res;
}

void BigFloat::removeLeadingZeros() {
    while (integer_.size() > 1 && integer_[0] == 0) integer_.erase(integer_.begin());
    if (integer_.empty()) integer_.push_back(0);
}
void BigFloat::removeTrailingZeros() {
    while (!fractional_.empty() && fractional_.back() == 0) fractional_.pop_back();
}

static std::vector<char> multiply_naive(const std::vector<char>& a, const std::vector<char>& b) {
    if (a.empty() || b.empty()) return {0};
    size_t n = a.size(), m = b.size();
    std::vector<int> tmp(n + m, 0);
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < m; ++j)
            tmp[i + j + 1] += a[i] * b[j];
    int carry = 0;
    for (int i = (int)tmp.size() - 1; i >= 0; --i) {
        tmp[i] += carry;
        carry = tmp[i] / 10;
        tmp[i] %= 10;
    }
    std::vector<char> result;
    bool leading = true;
    for (int x : tmp) {
        if (leading && x == 0) continue;
        leading = false;
        result.push_back(static_cast<char>(x));
    }
    if (result.empty()) result.push_back(0);
    return result;
}

static std::vector<char> add_vectors(const std::vector<char>& a, const std::vector<char>& b) {
    size_t max_len = std::max(a.size(), b.size());
    std::vector<char> a_padded(max_len, 0);
    std::vector<char> b_padded(max_len, 0);
    std::copy(a.begin(), a.end(), a_padded.begin() + (max_len - a.size()));
    std::copy(b.begin(), b.end(), b_padded.begin() + (max_len - b.size()));
    std::vector<char> res(max_len, 0);
    int carry = 0;
    for (int i = (int)max_len - 1; i >= 0; --i) {
        int sum = a_padded[i] + b_padded[i] + carry;
        res[i] = sum % 10;
        carry = sum / 10;
    }
    if (carry) res.insert(res.begin(), static_cast<char>(carry));
    while (res.size() > 1 && res.front() == 0) res.erase(res.begin());
    return res;
}

static std::vector<char> sub_vectors(const std::vector<char>& a, const std::vector<char>& b) {
    size_t max_len = std::max(a.size(), b.size());
    std::vector<char> a_padded(max_len, 0);
    std::vector<char> b_padded(max_len, 0);
    std::copy(a.begin(), a.end(), a_padded.begin() + (max_len - a.size()));
    std::copy(b.begin(), b.end(), b_padded.begin() + (max_len - b.size()));
    std::vector<char> res(max_len, 0);
    int borrow = 0;
    for (int i = (int)max_len - 1; i >= 0; --i) {
        int diff = a_padded[i] - b_padded[i] - borrow;
        if (diff < 0) { diff += 10; borrow = 1; }
        else borrow = 0;
        res[i] = static_cast<char>(diff);
    }
    while (res.size() > 1 && res.front() == 0) res.erase(res.begin());
    return res;
}

static void normalize_vector(std::vector<char>& digits) {
    int carry = 0;
    for (int i = (int)digits.size() - 1; i >= 0; --i) {
        int val = digits[i] + carry;
        digits[i] = val % 10;
        carry = val / 10;
    }
    if (carry) digits.insert(digits.begin(), static_cast<char>(carry));
    while (digits.size() > 1 && digits.front() == 0) digits.erase(digits.begin());
}

static std::vector<char> multiply_karatsuba(const std::vector<char>& a, const std::vector<char>& b) {
    if (a.size() < KARATSUBA_THRESHOLD || b.size() < KARATSUBA_THRESHOLD) return multiply_naive(a, b);
    size_t n = std::max(a.size(), b.size());
    size_t m = 1;
    while (m < n) m <<= 1;
    std::vector<char> a_padded(m, 0), b_padded(m, 0);
    std::copy(a.begin(), a.end(), a_padded.begin() + (m - a.size()));
    std::copy(b.begin(), b.end(), b_padded.begin() + (m - b.size()));
    size_t half = m / 2;
    std::vector<char> a_low(a_padded.begin() + (m - half), a_padded.end());
    std::vector<char> a_high(a_padded.begin(), a_padded.begin() + (m - half));
    std::vector<char> b_low(b_padded.begin() + (m - half), b_padded.end());
    std::vector<char> b_high(b_padded.begin(), b_padded.begin() + (m - half));
    std::vector<char> z0 = multiply_karatsuba(a_low, b_low);
    std::vector<char> z2 = multiply_karatsuba(a_high, b_high);
    std::vector<char> sum_a = add_vectors(a_low, a_high);
    std::vector<char> sum_b = add_vectors(b_low, b_high);
    std::vector<char> z1_full = multiply_karatsuba(sum_a, sum_b);
    std::vector<char> z1 = sub_vectors(z1_full, z2);
    z1 = sub_vectors(z1, z0);
    size_t result_len = z2.size() + 2 * half;
    std::vector<char> result(result_len, 0);
    for (size_t i = 0; i < z0.size(); ++i) {
        size_t idx = result_len - 1 - i;
        if (idx < result.size()) result[idx] = z0[z0.size() - 1 - i];
    }
    for (size_t i = 0; i < z1.size(); ++i) {
        size_t pos = result_len - 1 - half - i;
        if (pos < result.size()) result[pos] += z1[z1.size() - 1 - i];
    }
    for (size_t i = 0; i < z2.size(); ++i) {
        size_t pos = result_len - 1 - 2 * half - i;
        if (pos < result.size()) result[pos] += z2[z2.size() - 1 - i];
    }
    normalize_vector(result);
    return result;
}

BigFloat BigFloat::operator*(const BigFloat& other) const {
    std::vector<char> a_digits = integer_;
    a_digits.insert(a_digits.end(), fractional_.begin(), fractional_.end());
    std::vector<char> b_digits = other.integer_;
    b_digits.insert(b_digits.end(), other.fractional_.begin(), other.fractional_.end());
    while (a_digits.size() > 1 && a_digits.front() == 0) a_digits.erase(a_digits.begin());
    while (b_digits.size() > 1 && b_digits.front() == 0) b_digits.erase(b_digits.begin());
    size_t result_frac_len = fractional_.size() + other.fractional_.size();
    std::vector<char> product_digits = multiply_karatsuba(a_digits, b_digits);
    normalize_vector(product_digits);
    bool result_sign = (sign_ == other.sign_);
    BigFloat result;
    result.sign_ = result_sign;
    if (product_digits.size() <= result_frac_len) {
        result.integer_ = {0};
        size_t leading_zeros = result_frac_len - product_digits.size();
        for (size_t i = 0; i < leading_zeros; ++i) result.fractional_.push_back(0);
        result.fractional_.insert(result.fractional_.end(), product_digits.begin(), product_digits.end());
    } else {
        size_t int_len = product_digits.size() - result_frac_len;
        result.integer_.assign(product_digits.begin(), product_digits.begin() + int_len);
        result.fractional_.assign(product_digits.begin() + int_len, product_digits.end());
    }
    result.normalize();
    return result;
}

BigFloat BigFloat::reciprocal(const BigFloat& x, size_t precision) {
    if (x.isZero()) throw std::runtime_error("Reciprocal of zero");
    std::string x_str = x.toString();
    if (x_str.size() > 16) x_str = x_str.substr(0, 16);
    long double xd = std::stold(x_str);
    long double invd = 1.0L / xd;
    BigFloat r(invd);
    size_t current_prec = 15;
    BigFloat two("2");
    while (current_prec < precision) {
        BigFloat xr = x * r;
        BigFloat two_minus_xr = two - xr;
        r = r * two_minus_xr;
        current_prec *= 2;
        if (current_prec > precision) current_prec = precision;
        r.round(current_prec + RECIPROCAL_ROUND_EXTRA);
    }
    return r;
}

BigFloat BigFloat::operator/(const BigFloat& other) const {
    if (other.isZero()) throw std::runtime_error("Division by zero");
    if (isZero()) return BigFloat("0");

    bool result_sign = (sign_ == other.sign_);
    BigFloat a = this->abs();
    BigFloat b = other.abs();

    int shift_b = 0;
    BigFloat b_norm = b;
    while (b_norm.integer_.size() > 1 || (b_norm.integer_.size() == 1 && b_norm.integer_[0] >= 10)) {
        char last_int = b_norm.integer_.back();
        b_norm.integer_.pop_back();
        if (b_norm.integer_.empty()) b_norm.integer_.push_back(0);
        b_norm.fractional_.insert(b_norm.fractional_.begin(), last_int);
        shift_b++;
        b_norm.normalize();
    }
    while (b_norm.integer_.size() == 1 && b_norm.integer_[0] == 0 && !b_norm.fractional_.empty()) {
        char first_frac = b_norm.fractional_.front();
        b_norm.fractional_.erase(b_norm.fractional_.begin());
        b_norm.integer_[0] = first_frac;
        shift_b--;
        b_norm.normalize();
    }

    size_t total_digits = a.integer_.size() + a.fractional_.size() + b.integer_.size() + b.fractional_.size();
    size_t precision = total_digits * 2 + DIVISION_PRECISION_EXTRA;

    BigFloat inv = reciprocal(b_norm, precision);
    BigFloat quotient = a * inv;
    quotient = quotient.shift(-shift_b);
    quotient.sign_ = result_sign;
    quotient.normalize();

    if (quotient.fractional_.size() > precision) {
        quotient.round(precision);
    }
    return quotient;
}

BigFloat BigFloat::sqrt() const {
    if (!sign_) throw std::runtime_error("Square root of negative number");
    if (isZero()) return BigFloat("0");

    BigFloat x = *this;
    int exponent = 0;

    while (x < BigFloat("1")) {
        x = x.shift(1);
        exponent--;
    }
    while (x >= BigFloat("100")) {
        x = x.shift(-1);
        exponent++;
    }
    if (exponent % 2 != 0) {
        x = x.shift(-1);
        exponent++;
    }

    size_t total_digits = integer_.size() + fractional_.size();
    size_t target_prec = total_digits * 2 + 30;
    if (target_prec < 30) target_prec = 30;
    size_t working_prec = target_prec + 10;

    std::string xs = x.toString();
    if (xs.size() > 16) xs = xs.substr(0, 16);
    long double init_val = std::sqrt(std::stold(xs));
    BigFloat y(init_val);

    size_t current_prec = 15;
    BigFloat two("2");
    while (current_prec < target_prec) {
        BigFloat inv_y = reciprocal(y, working_prec);
        BigFloat x_div_y = x * inv_y;
        BigFloat sum = y + x_div_y;
        y = sum / two;
        current_prec *= 2;
        if (current_prec > working_prec) current_prec = working_prec;
        y.round(current_prec);
    }

    int shift_half = exponent / 2;
    if (shift_half != 0) y = y.shift(shift_half);
    y.round(target_prec);
    return y;
}

std::ostream& operator<<(std::ostream& os, const BigFloat& num) {
    os << num.toString();
    return os;
}
std::istream& operator>>(std::istream& is, BigFloat& num) {
    std::string str;
    is >> str;
    num = BigFloat(str);
    return is;
}