/**
 * @file unit_tests.cpp
 * @brief Automated test suite across the five spiral cycles (Unity framework).
 *
 * The suite is split into two groups:
 *   - core_cases.hpp   : hardware independent domain logic (16 cases)
 *   - driver_cases.hpp : FreeRTOS primitives and hardware drivers (6 cases)
 *
 * Build and run (ESP-IDF v6):
 *   cd test/test_clock_core
 *   idf.py set-target esp32
 *   idf.py build flash monitor
 */
#include <cstring>

#include "esp_log.h"
#include "unity.h"

#include "core_cases.hpp"
#include "driver_cases.hpp"

namespace {

constexpr const char* kTag = "TEST";

}  // namespace

/** @brief Unity fixture - nothing to prepare between the test cases. */
void setUp(void) {}

/** @brief Unity fixture - nothing to clean up between the test cases. */
void tearDown(void) {}

extern "C" void app_main() {
    ESP_LOGI(kTag, "----- SPIRAL 1: hardware layer & bus communication -----");
    UNITY_BEGIN();
    RUN_TEST(test_ds1307_bcd_conversion);
    RUN_TEST(test_i2c_bus_mutex_lock);
    RUN_TEST(test_ssd1306_buffer_bounds);

    ESP_LOGI(kTag, "----- SPIRAL 2: time keeping subsystem & synchronisation -----");
    RUN_TEST(test_time_manager_rtc_fallback);
    RUN_TEST(test_ntp_timestamp_parsing);

    ESP_LOGI(kTag, "----- SPIRAL 3: input & UI state machine -----");
    RUN_TEST(test_button_long_press_detect);
    RUN_TEST(test_ui_mode_cycle);

    ESP_LOGI(kTag, "----- SPIRAL 4 & 5: engines & FreeRTOS integration -----");
    RUN_TEST(test_alarm_time_comparator);
    RUN_TEST(test_stopwatch_lap_split_calc);
    RUN_TEST(test_freertos_event_group_sync);

    ESP_LOGI(kTag, "----- DASHBOARD SUITE: spiral 1 (HAL & core drivers) -----");
    RUN_TEST(DS1307_BCD_To_Dec_Conversion);
    RUN_TEST(DS1307_Dec_To_BCD_Conversion);
    RUN_TEST(SSD1306_Framebuffer_Allocation_1024B);

    ESP_LOGI(kTag, "----- DASHBOARD SUITE: spiral 2 (task engine & switch driver) -----");
    RUN_TEST(FreeRTOS_Button_Queue_Post_Receive);
    RUN_TEST(Button_Software_Debounce_50ms_Filter);
    RUN_TEST(I2C_Mutex_Thread_Safety_Lock);

    ESP_LOGI(kTag, "----- DASHBOARD SUITE: spiral 3 (network & NTP) -----");
    RUN_TEST(WiFi_Station_Connect_SSID_iotexpo);
    RUN_TEST(SNTP_Timestamp_Offset_Calculation);
    RUN_TEST(DS1307_NTP_AutoCalibration_Write);

    ESP_LOGI(kTag, "----- DASHBOARD SUITE: spiral 4 (alarm & lap timer) -----");
    RUN_TEST(Alarm_Comparator_Trigger_Match);
    RUN_TEST(Stopwatch_HighRes_Timer_Precision);
    RUN_TEST(Lap_Memory_Buffer_Push);

    UNITY_END();
}
