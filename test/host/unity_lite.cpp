#include "unity_lite.hpp"

namespace UnityLite {

int g_failures = 0;
int g_checks = 0;
int g_testFailures = 0;

void reportFailure(const char* file, int line, const std::string& message) {
    ++g_failures;
    ++g_testFailures;
    std::printf("\n    %s:%d: %s\n", file, line, message.c_str());
}

std::string describe(long long value) { return std::to_string(value); }

std::string describe(unsigned long long value) { return std::to_string(value); }

std::string describe(const char* value) { return value == nullptr ? "(null)" : value; }

}  // namespace UnityLite
