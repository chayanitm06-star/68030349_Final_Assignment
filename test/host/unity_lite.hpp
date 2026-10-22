/**
 * @file unity_lite.hpp
 * @brief Minimal drop-in replacement of the Unity API for host builds.
 * @details The ESP-IDF Unity framework is only available on the target. The
 *          shim implements the small subset of the API that the shared test
 *          cases use, so that the pure domain layer can be verified on the
 *          development PC with a plain C++ compiler.
 */
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace UnityLite {

/** @brief Number of failed checks of the whole run. */
extern int g_failures;
/** @brief Number of executed checks. */
extern int g_checks;
/** @brief Number of failed checks of the current test. */
extern int g_testFailures;

/** @brief Record a failed check and print the location. */
void reportFailure(const char* file, int line, const std::string& message);

/** @brief Format helper for the assertion messages. */
std::string describe(long long value);
std::string describe(unsigned long long value);
std::string describe(const char* value);

}  // namespace UnityLite

#define UNITY_BEGIN() (UnityLite::g_failures = 0, UnityLite::g_checks = 0)
#define UNITY_END() (UnityLite::g_failures)

#define UNITY_FAIL_AND_RETURN(message)                     \
    do {                                                   \
        UnityLite::reportFailure(__FILE__, __LINE__, (message)); \
        return;                                            \
    } while (0)

#define TEST_ASSERT_TRUE(condition)                                                 \
    do {                                                                            \
        ++UnityLite::g_checks;                                                      \
        if (!(condition)) {                                                         \
            UNITY_FAIL_AND_RETURN(std::string("expected TRUE: ") + #condition);    \
        }                                                                           \
    } while (0)

#define TEST_ASSERT_FALSE(condition)                                                 \
    do {                                                                            \
        ++UnityLite::g_checks;                                                      \
        if ((condition)) {                                                          \
            UNITY_FAIL_AND_RETURN(std::string("expected FALSE: ") + #condition);   \
        }                                                                           \
    } while (0)

#define TEST_ASSERT_EQUAL_INT(expected, actual)                                              \
    do {                                                                                    \
        ++UnityLite::g_checks;                                                              \
        const long long e = static_cast<long long>(expected);                               \
        const long long a = static_cast<long long>(actual);                                 \
        if (e != a) {                                                                       \
            UNITY_FAIL_AND_RETURN(std::string(#actual) + " == " + UnityLite::describe(e) +  \
                                  " failed (actual " + UnityLite::describe(a) + ")");       \
        }                                                                                   \
    } while (0)

#define TEST_ASSERT_EQUAL_INT32(expected, actual) TEST_ASSERT_EQUAL_INT(expected, actual)
#define TEST_ASSERT_EQUAL_INT64(expected, actual) TEST_ASSERT_EQUAL_INT(expected, actual)

#define TEST_ASSERT_EQUAL_UINT8(expected, actual)  TEST_ASSERT_EQUAL_INT(expected, actual)
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) TEST_ASSERT_EQUAL_INT(expected, actual)
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) TEST_ASSERT_EQUAL_INT(expected, actual)
#define TEST_ASSERT_EQUAL_UINT64(expected, actual) TEST_ASSERT_EQUAL_INT(expected, actual)

#define TEST_ASSERT_EQUAL_STRING(expected, actual)                                            \
    do {                                                                                      \
        ++UnityLite::g_checks;                                                                \
        const char* e = (expected);                                                           \
        const char* a = (actual);                                                            \
        if (e == nullptr || a == nullptr || std::strcmp(e, a) != 0) {                          \
            UNITY_FAIL_AND_RETURN(std::string(#actual) + " == \"" + (e ? e : "(null)") +      \
                                  "\" failed (actual \"" + (a ? a : "(null)") + "\")");        \
        }                                                                                     \
    } while (0)

#define TEST_ASSERT_NOT_NULL(pointer)                                                    \
    do {                                                                                \
        ++UnityLite::g_checks;                                                          \
        if ((pointer) == nullptr) {                                                     \
            UNITY_FAIL_AND_RETURN(std::string(#pointer) + " must not be NULL");         \
        }                                                                               \
    } while (0)

#define TEST_ASSERT_GREATER_THAN(threshold, actual)                                              \
    do {                                                                                        \
        ++UnityLite::g_checks;                                                                  \
        const long long t = static_cast<long long>(threshold);                                  \
        const long long a = static_cast<long long>(actual);                                      \
        if (!(a > t)) {                                                                         \
            UNITY_FAIL_AND_RETURN(std::string(#actual) + " > " + UnityLite::describe(t) +       \
                                  " failed (actual " + UnityLite::describe(a) + ")");            \
        }                                                                                       \
    } while (0)

#define TEST_ASSERT_LESS_THAN(threshold, actual)                                                \
    do {                                                                                        \
        ++UnityLite::g_checks;                                                                  \
        const long long t = static_cast<long long>(threshold);                                  \
        const long long a = static_cast<long long>(actual);                                      \
        if (!(a < t)) {                                                                         \
            UNITY_FAIL_AND_RETURN(std::string(#actual) + " < " + UnityLite::describe(t) +       \
                                  " failed (actual " + UnityLite::describe(a) + ")");            \
        }                                                                                       \
    } while (0)

/** @brief Execute one test case and print its result. */
#define RUN_TEST(function)                                                             \
    do {                                                                               \
        UnityLite::g_testFailures = 0;                                                 \
        std::printf("  %-45s", #function);                                            \
        std::fflush(stdout);                                                           \
        function();                                                                   \
        if (UnityLite::g_testFailures == 0) {                                         \
            std::printf("PASS\n");                                                     \
        } else {                                                                       \
            std::printf("FAIL (%d check(s))\n", UnityLite::g_testFailures);           \
        }                                                                              \
    } while (0)
