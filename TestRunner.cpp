#include "TestRunner.h"
#include "QuadraticSolver/QuadraticSolver.h"
#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

static string makeDigits(char digit, size_t count) {
    return string(count, digit);
}

static void runTest(const string& description, const string& a, const string& b, const string& c) {
    cout << "\n=== " << description << " ===" << endl;
    cout << "Equation: " << a << " x^2 + " << b << " x + " << c << " = 0" << endl;
    QuadraticResult res = QuadraticSolver::solve(a, b, c);
    cout << "Status: " << res.status << endl;
    if (res.status == "OK") {
        if (!res.real_roots.empty()) {
            cout << "Number of roots: " << res.real_roots.size() << endl;
            for (const auto& x : res.real_roots) {
                cout << "x = " << x.toString() << endl;
            }
        } else if (!res.complex_roots.empty()) {
            cout << "Number of roots: " << res.complex_roots.size() << endl;
            for (const auto& x : res.complex_roots) {
                cout << "x = " << x.toString() << endl;
            }
        }
    }
    cout << "----------------------------------------" << endl;
}

static void testIntegerOnly() {
    cout << "\n=== TESTS: INTEGER COEFFICIENTS ===";
    runTest("x^2 - 5x + 6 = 0", "1", "-5", "6");
    runTest("x^2 + 2x + 1 = 0", "1", "2", "1");
    runTest("x^2 - 4 = 0", "1", "0", "-4");
    runTest("2x^2 - 8 = 0", "2", "0", "-8");
    runTest("x^2 + x + 1 = 0", "1", "1", "1");
    runTest("9x^2 - 12x + 4 = 0", "9", "-12", "4");
}

static void testFractionalOnly() {
    cout << "\n=== TESTS: FRACTIONAL COEFFICIENTS ===";
    runTest("0.5x^2 - 1.25x + 0.75 = 0", "0.5", "-1.25", "0.75");
    runTest("0.2x^2 + 0.4x + 0.2 = 0", "0.2", "0.4", "0.2");
    runTest("0.33x^2 - 0.66x + 0.33 = 0", "0.33", "-0.66", "0.33");
    runTest("1.5x^2 + 0.5 = 0", "1.5", "0", "0.5");
    runTest(".1x^2 - .2x + .1 = 0", ".1", "-.2", ".1");
}

static void testAllSolutions() {
    cout << "\n=== TESTS: ALL SOLUTION TYPES ===";
    runTest("Two distinct real roots", "1", "-3", "2");
    runTest("One real root (double)", "1", "-2", "1");
    runTest("No real roots (complex)", "1", "1", "1");
    runTest("a=0, linear, one root", "0", "2", "-4");
    runTest("a=0, b=0, c=0 => INF", "0", "0", "0");
    runTest("a=0, b=0, c non-zero => NO SOLUTION", "0", "0", "5");
    runTest("a=0, b=0, c=0 with leading zeros", "000", "000", "0");
    runTest("Negative discriminant large numbers", "100", "0", "1");
}

static void testInvalidInput() {
    cout << "\n=== TESTS: INVALID INPUT ===";
    runTest("Empty string", "", "2", "3");
    runTest("Letters", "abc", "2", "3");
    runTest("Two dots", "1.2.3", "2", "3");
    runTest("Invalid exponent", "1e", "2", "3");
    runTest("Missing exponent digits", "1e+", "2", "3");
    runTest("Multiple signs", "+-5", "2", "3");
    runTest("Only decimal point", ".", "2", "3");
    runTest("Empty after e", "5E", "0", "1");
    runTest("Invalid character", "1,2", "3", "4");
}

static void testLargeNumbers() {
    cout << "\n=== TESTS: LARGE NUMBERS (UP TO 100000 DIGITS) ===";
    const size_t len = 1000;
    string big9 = makeDigits('9', len);
    string big1 = makeDigits('1', len);
    string big2 = makeDigits('2', len);
    string big3 = makeDigits('3', len);
    string bigZero = "0";
    runTest("Large a, b, c (10^len)", big1, big2, big3);
    runTest("Large coefficients, D>0", big2, big1, big3);
    runTest("Large coefficients, D=0", big1, big2, big1);
    runTest("Large a=0, linear", bigZero, big1, big2);
    runTest("Large a=0, b=0, c=0", bigZero, bigZero, bigZero);
    runTest("Max size 100000 digits (disabled by default)", "1"+makeDigits('0',99999), "1", "1");
}

static void testTinyNumbers() {
    cout << "\n=== TESTS: VERY SMALL NUMBERS (10^-N) ===";
    string tiny = "0." + makeDigits('0', 99998) + "1";
    runTest("Tiny a, b, c", tiny, tiny, tiny);
    runTest("Tiny a, tiny b, zero c", tiny, tiny, "0");
}

static void testMixedLargeSmall() {
    cout << "\n=== TESTS: LARGE AND SMALL MIXED ===";
    string big9 = makeDigits('9', 1000);
    string tiny = "0." + makeDigits('0', 999) + "1";
    runTest("Large a, small b, small c", big9, tiny, tiny);
    runTest("Small a, large b, large c", tiny, big9, big9);
    runTest("Large a, large b, small c", big9, big9, tiny);
}

static void interactiveMode() {
    string a_str, b_str, c_str;
    cout << "\nEnter coefficients a, b, c (each on separate line):\n";
    if (!getline(cin, a_str) || !getline(cin, b_str) || !getline(cin, c_str)) {
        cerr << "Input error\n";
        return;
    }
    QuadraticResult result = QuadraticSolver::solve(a_str, b_str, c_str);
    cout << result.status << endl;
    if (result.status == "OK") {
        if (!result.real_roots.empty()) {
            cout << result.real_roots.size() << endl;
            for (const auto& x : result.real_roots) {
                cout << x.toString() << endl;
            }
        } else if (!result.complex_roots.empty()) {
            cout << result.complex_roots.size() << endl;
            for (const auto& x : result.complex_roots) {
                cout << x.toString() << endl;
            }
        } else {
            cout << "0" << endl;
        }
    }
}

void TestRunner::run() {
    while (true) {
        cout << "\nQuadratic Equation Solver (BigFloat)\n";
        cout << "1. Interactive mode\n";
        cout << "2. Run all tests\n";
        cout << "3. Run specific test groups (by letters, e.g., 'ifc')\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        string choice;
        if (!getline(cin, choice)) {
            cerr << "Input error, exiting.\n";
            break;
        }
        if (choice == "0") {
            break;
        } else if (choice == "1") {
            interactiveMode();
        } else if (choice == "2") {
            testIntegerOnly();
            testFractionalOnly();
            testAllSolutions();
            testInvalidInput();
            testLargeNumbers();
            testTinyNumbers();
            testMixedLargeSmall();
        } else if (choice == "3") {
            cout << "Enter test group letters (i=integer, f=fractional, a=all solution types, v=invalid input, l=large numbers, t=tiny numbers, m=mixed large/small). Max 100 characters.\n";
            string groups;
            if (!getline(cin, groups)) {
                cerr << "Input error\n";
                continue;
            }
            if (groups.length() > 100) {
                cout << "Error: string too long (max 100 chars).\n";
                continue;
            }
            bool valid = true;
            for (char c : groups) {
                c = tolower(c);
                if (c != 'i' && c != 'f' && c != 'a' && c != 'v' && c != 'l' && c != 't' && c != 'm') {
                    cout << "Invalid character: " << c << endl;
                    valid = false;
                    break;
                }
            }
            if (!valid) continue;
            for (char c : groups) {
                switch (tolower(c)) {
                    case 'i': testIntegerOnly(); break;
                    case 'f': testFractionalOnly(); break;
                    case 'a': testAllSolutions(); break;
                    case 'v': testInvalidInput(); break;
                    case 'l': testLargeNumbers(); break;
                    case 't': testTinyNumbers(); break;
                    case 'm': testMixedLargeSmall(); break;
                }
            }
        } else {
            cout << "Invalid choice, please try again.\n";
        }
    }
}