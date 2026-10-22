/**
 * @file host_test_main.cpp
 * @brief Host runner for the hardware independent part of the test suite.
 *
 * The pure domain layer of the clock (core/) contains no ESP-IDF dependency,
 * therefore the complete business logic can be verified on the development PC:
 *
 *   # Linux / macOS
 *   g++ -std=c++17 -Wall -Wextra \
 *       -I main/core -I main/drivers -I test/test_clock_core/main -I test/host \
 *       test/host/host_test_main.cpp test/host/unity_lite.cpp \
 *       (all sources of main/core) main/drivers/Stopwatch.cpp \
 *       -o build/host_tests && ./build/host_tests
 *
 * The very same test bodies are executed on the ESP32 by the Unity
 * application in test/test_clock_core.
 */
#include <cstdio>

#include "unity_lite.hpp"

#include "core_cases.hpp"

int main() {
    std::printf("=====================================================\n");
    std::printf(" ESP32 digital clock - host unit test runner\n");
    std::printf("=====================================================\n\n");

    UNITY_BEGIN();

    std::printf("[Spiral 1] hardware layer & bus communication\n");
    RUN_TEST(test_ds1307_bcd_conversion);
    RUN_TEST(DS1307_BCD_To_Dec_Conversion);
    RUN_TEST(DS1307_Dec_To_BCD_Conversion);
    RUN_TEST(SSD1306_Framebuffer_Allocation_1024B);

    std::printf("\n[Spiral 2] time keeping subsystem & synchronisation\n");
    RUN_TEST(test_time_manager_rtc_fallback);
    RUN_TEST(test_ntp_timestamp_parsing);
    RUN_TEST(SNTP_Timestamp_Offset_Calculation);
    RUN_TEST(DS1307_NTP_AutoCalibration_Write);

    std::printf("\n[Spiral 3] input & UI state machine\n");
    RUN_TEST(test_button_long_press_detect);
    RUN_TEST(Button_Software_Debounce_50ms_Filter);
    RUN_TEST(test_ui_mode_cycle);

    std::printf("\n[Spiral 4 & 5] engines & FreeRTOS integration\n");
    RUN_TEST(test_alarm_time_comparator);
    RUN_TEST(Alarm_Comparator_Trigger_Match);
    RUN_TEST(test_stopwatch_lap_split_calc);
    RUN_TEST(Stopwatch_HighRes_Timer_Precision);
    RUN_TEST(Lap_Memory_Buffer_Push);

    const int failures = UNITY_END();

    std::printf("\n=====================================================\n");
    std::printf(" checks: %d, failed tests: %d\n", UnityLite::g_checks, failures);
    std::printf(" %s\n", failures == 0 ? "ALL TESTS PASSED" : "THERE WERE FAILURES");
    std::printf("=====================================================\n");

    return failures == 0 ? 0 : 1;
}
