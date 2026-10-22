/**
 * @file unity.h
 * @brief Minimal stand-in for the ESP-IDF Unity component (development only).
 * @details Used to type check the target test runner on a host machine.
 */
#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#define UNITY_BEGIN() (0)
#define UNITY_END() (0)
#define RUN_TEST(fn) (void)(fn)

#define TEST_ASSERT_TRUE(cond)                       \
    do {                                             \
        if (!(cond)) {                               \
            std::printf("FAIL %s:%d\n", __FILE__, __LINE__); \
        }                                            \
    } while (0)
#define TEST_ASSERT_FALSE(cond) TEST_ASSERT_TRUE(!(cond))
#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT_TRUE((ptr) != nullptr)
#define TEST_ASSERT_NULL(ptr) TEST_ASSERT_TRUE((ptr) == nullptr)
#define TEST_ASSERT_EQUAL_INT(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_EQUAL_INT32(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_EQUAL_INT64(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_EQUAL_UINT64(expected, actual) TEST_ASSERT_TRUE((expected) == (actual))
#define TEST_ASSERT_GREATER_THAN(threshold, actual) TEST_ASSERT_TRUE((actual) > (threshold))
#define TEST_ASSERT_LESS_THAN(threshold, actual) TEST_ASSERT_TRUE((actual) < (threshold))
#define TEST_ASSERT_EQUAL_STRING(expected, actual)                                     \
    do {                                                                              \
        const char* e = (expected);                                                    \
        const char* a = (actual);                                                      \
        TEST_ASSERT_TRUE(e != nullptr && a != nullptr && std::strcmp(e, a) == 0);     \
    } while (0)
