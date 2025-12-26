#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <math.h>

// Global test state
static int _test_total = 0;
static int _test_passed = 0;
static int _test_failed = 0;
static const char* _current_test_name = nullptr;
static const char* _current_section_name = nullptr;
static int _current_test_failures = 0;
static bool _verbose = false; // Set to true to see all assertions

// Test registry
typedef void (*TestFunc)();
struct TestEntry {
    const char* name;
    TestFunc func;
    TestEntry* next;
};
static TestEntry* _test_list_head = nullptr;

// Approx helper for floating point comparisons
struct Approx
{
    double value;
    double margin_val;

    Approx(double v) : value(v), margin_val(0.0001) {}

    Approx& margin(double m)
    {
        margin_val = m;
        return *this;
    }

    bool matches(double other) const
    {
        return fabs(value - other) <= margin_val;
    }
};

inline bool operator==(double lhs, const Approx& rhs) { return rhs.matches(lhs); }
inline bool operator==(float lhs, const Approx& rhs) { return rhs.matches((double)lhs); }

// Test macros - using __COUNTER__ for unique names
#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define TEST_CASE_IMPL(name, id) \
    static void CONCAT(test_func_, id)(); \
    static TestEntry CONCAT(test_entry_, id) = {name, CONCAT(test_func_, id), nullptr}; \
    struct CONCAT(TestRegistrar_, id) { \
        CONCAT(TestRegistrar_, id)() { \
            CONCAT(test_entry_, id).next = _test_list_head; \
            _test_list_head = &CONCAT(test_entry_, id); \
        } \
    }; \
    static CONCAT(TestRegistrar_, id) CONCAT(registrar_, id); \
    static void CONCAT(test_func_, id)()

#define TEST_CASE(name, ...) TEST_CASE_IMPL(name, __COUNTER__)

#define SECTION(name) \
    _current_section_name = name; \
    if (_verbose) printf("  [SECTION] %s\n", name);

#define REQUIRE(expr) \
    do { \
        _test_total++; \
        if (expr) { \
            _test_passed++; \
            if (_verbose) printf("    [PASS] %s\n", #expr); \
        } else { \
            _test_failed++; \
            _current_test_failures++; \
            printf("    [FAIL] %s (at %s:%d)\n", #expr, __FILE__, __LINE__); \
        } \
    } while(0)

// Test runner
inline int RunTests()
{
    int test_cases_run = 0;
    int test_cases_passed = 0;

    // Run all registered tests
    for (TestEntry* test = _test_list_head; test != nullptr; test = test->next)
    {
        _current_test_name = test->name;
        _current_test_failures = 0;

        test->func();

        test_cases_run++;
        if (_current_test_failures == 0) {
            test_cases_passed++;
            printf("[PASS] %s\n", test->name);
        } else {
            printf("[FAIL] %s (%d assertion(s) failed)\n", test->name, _current_test_failures);
        }
    }

    printf("\n========================================\n");
    printf("Test Results:\n");
    printf("  Test Cases: %d passed, %d failed, %d total\n", test_cases_passed, test_cases_run - test_cases_passed, test_cases_run);
    printf("  Assertions: %d passed, %d failed, %d total\n", _test_passed, _test_failed, _test_total);
    printf("========================================\n");

    return _test_failed == 0 ? 0 : 1;
}

#endif // TEST_H
