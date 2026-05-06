#ifndef BIGFLOAT_H
#define BIGFLOAT_H

#include <string>
#include <vector>
#include <iosfwd>

class BigFloat {
public:
    BigFloat();
    explicit BigFloat(const std::string& str);
    explicit BigFloat(long double value);

    BigFloat operator+(const BigFloat& other) const;
    BigFloat operator-(const BigFloat& other) const;
    BigFloat operator-() const;
    BigFloat operator*(const BigFloat& other) const;
    BigFloat operator/(const BigFloat& other) const;

    BigFloat& operator+=(const BigFloat& other);
    BigFloat& operator-=(const BigFloat& other);
    BigFloat& operator*=(const BigFloat& other);
    BigFloat& operator/=(const BigFloat& other);

    bool operator==(const BigFloat& other) const;
    bool operator!=(const BigFloat& other) const;
    bool operator<(const BigFloat& other) const;
    bool operator<=(const BigFloat& other) const;
    bool operator>(const BigFloat& other) const;
    bool operator>=(const BigFloat& other) const;

    std::string toString() const;
    void normalize();
    void round(size_t precision);
    bool isZero() const;
    BigFloat abs() const;
    BigFloat shift(int n) const;

    BigFloat sqrt() const;

    friend std::ostream& operator<<(std::ostream& os, const BigFloat& num);
    friend std::istream& operator>>(std::istream& is, BigFloat& num);

private:
    bool sign_;
    std::vector<char> integer_;
    std::vector<char> fractional_;

    int compareAbs(const BigFloat& other) const;
    BigFloat addAbs(const BigFloat& a, const BigFloat& b) const;
    BigFloat subAbs(const BigFloat& a, const BigFloat& b) const;

    void removeLeadingZeros();
    void removeTrailingZeros();

    static BigFloat reciprocal(const BigFloat& x, size_t precision);
};

#endif