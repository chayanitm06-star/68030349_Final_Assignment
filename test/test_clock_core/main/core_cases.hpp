/**
 * @file core_cases.hpp
 * @brief Hardware independent test cases of the spiral unit test suite.
 *
 * Every test in this file only touches the pure domain layer, therefore the
 * cases can be executed
 *   - on the ESP32 target  (test/test_clock_core/unit_tests.cpp) and
 *   - on the host PC        (test/host/host_test_main.cpp).
 *
 * The test names are the ones that the assignment documents require:
 *   * test_ds1307_bcd_conversion, test_time_manager_rtc_fallback,
 *     test_ntp_timestamp_parsing, test_button_long_press_detect,
 *     test_ui_mode_cycle, test_alarm_time_comparator,
 *     test_stopwatch_lap_split_calc                       (suite flavour)
 *   * DS1307_BCD_To_Dec_Conversion, DS1307_Dec_To_BCD_Conversion,
 *     SSD1306_Framebuffer_Allocation_1024B,
 *     Button_Software_Debounce_50ms_Filter,
 *     SNTP_Timestamp_Offset_Calculation, DS1307_NTP_AutoCalibration_Write,
 *     Alarm_Comparator_Trigger_Match, Stopwatch_HighRes_Timer_Precision,
 *     Lap_Memory_Buffer_Push                             (dashboard flavour)
 */
#pragma once

#include <cstdint>

#include "AlarmEngine.hpp"
#include "Bcd.hpp"
#include "DateTime.hpp"
#include "Debounce.hpp"
#include "DisplayGeometry.hpp"
#include "NtpEpoch.hpp"
#include "Stopwatch.hpp"
#include "StopwatchEngine.hpp"
#include "TimeManager.hpp"
#include "UiStateMachine.hpp"
#include "mocks.hpp"

// The test cases live in the global namespace, the domain classes in Clock.
using namespace Clock;

// ---------------------------------------------------------------------------
// Spiral 1 - hardware layer & bus communication
// ---------------------------------------------------------------------------

/** @brief BCD <-> decimal conversion of the DS1307 register map. */
inline void test_ds1307_bcd_conversion() {
    TEST_ASSERT_EQUAL_UINT8(23, bcdToDecimal(0x23));
    TEST_ASSERT_EQUAL_UINT8(0, bcdToDecimal(0x00));
    TEST_ASSERT_EQUAL_UINT8(59, bcdToDecimal(0x59));
    TEST_ASSERT_EQUAL_UINT8(10, bcdToDecimal(0x10));
    TEST_ASSERT_EQUAL_UINT8(0x45, decimalToBcd(45));
    TEST_ASSERT_EQUAL_UINT8(0x00, decimalToBcd(0));
    TEST_ASSERT_EQUAL_UINT8(0x99, decimalToBcd(99));
    TEST_ASSERT_TRUE(isValidBcd(0x59));
    TEST_ASSERT_FALSE(isValidBcd(0x5A));
}

/** @brief Dashboard flavour of the BCD -> decimal conversion test. */
inline void DS1307_BCD_To_Dec_Conversion() {
    TEST_ASSERT_EQUAL_UINT8(23, bcdToDecimal(0x23));
    TEST_ASSERT_EQUAL_UINT8(45, bcdToDecimal(0x45));
    TEST_ASSERT_EQUAL_UINT8(59, bcdToDecimal(0x59));
    TEST_ASSERT_EQUAL_UINT8(7, bcdToDecimal(0x07));
}

/** @brief Dashboard flavour of the decimal -> BCD conversion test. */
inline void DS1307_Dec_To_BCD_Conversion() {
    TEST_ASSERT_EQUAL_UINT8(0x45, decimalToBcd(45));
    TEST_ASSERT_EQUAL_UINT8(0x23, decimalToBcd(23));
    TEST_ASSERT_EQUAL_UINT8(0x00, decimalToBcd(0));
    TEST_ASSERT_EQUAL_UINT8(0x59, decimalToBcd(59));
}

/** @brief The SSD1306 frame buffer must be exactly 1024 bytes. */
inline void SSD1306_Framebuffer_Allocation_1024B() {
    TEST_ASSERT_EQUAL_UINT8(128, Display::kWidth);
    TEST_ASSERT_EQUAL_UINT8(64, Display::kHeight);
    TEST_ASSERT_EQUAL_UINT16(1024, Display::kFramebufferBytes);
    TEST_ASSERT_EQUAL_UINT16((Display::kWidth * Display::kHeight) / 8, Display::kFramebufferBytes);
}

// ---------------------------------------------------------------------------
// Spiral 2 - time keeping subsystem & synchronisation
// ---------------------------------------------------------------------------

/** @brief The RTC is used whenever the network time is not available. */
inline void test_time_manager_rtc_fallback() {
    Test::FakeRtc rtc;
    Test::FakeSystemClock systemClock;
    Test::FakeNtpClient ntp;

    rtc.value.year = 2024;
    rtc.value.month = 5;
    rtc.value.day = 17;
    rtc.value.hour = 8;
    rtc.value.minute = 30;
    rtc.value.second = 15;

    TimeManager manager(rtc, systemClock, ntp);
    manager.setTimeZoneOffset(0);

    // Without a successful synchronisation the RTC is the time authority.
    TEST_ASSERT_FALSE(manager.isNtpSynced());
    DateTime now = manager.now();
    TEST_ASSERT_EQUAL_UINT16(2024, now.year);
    TEST_ASSERT_EQUAL_UINT8(5, now.month);
    TEST_ASSERT_EQUAL_UINT8(17, now.day);
    TEST_ASSERT_EQUAL_UINT8(8, now.hour);
    TEST_ASSERT_EQUAL_UINT8(30, now.minute);
    TEST_ASSERT_EQUAL_UINT8(15, now.second);

    // A failing synchronisation must not change the behaviour.
    ntp.syncSucceeds = false;
    TEST_ASSERT_FALSE(manager.requestSync());
    TEST_ASSERT_FALSE(manager.isNtpSynced());
    TEST_ASSERT_EQUAL_UINT8(8, manager.now().hour);

    // A successful synchronisation switches the time authority to the network.
    ntp.syncSucceeds = true;
    systemClock.available = true;
    systemClock.unixTime = 1704067200;  // 2024-01-01T00:00:00Z
    manager.setTimeZoneOffset(0);
    TEST_ASSERT_TRUE(manager.requestSync());
    TEST_ASSERT_TRUE(manager.isNtpSynced());
    TEST_ASSERT_EQUAL_UINT16(2024, manager.now().year);
    TEST_ASSERT_EQUAL_UINT8(1, manager.now().month);
    TEST_ASSERT_EQUAL_UINT8(1, manager.now().day);
    TEST_ASSERT_EQUAL_UINT8(0, manager.now().hour);

    // When the RTC disappears the last known value keeps the clock alive.
    rtc.available = false;
    TEST_ASSERT_TRUE(manager.now().isValid());
}

/** @brief NTP (1900 epoch) <-> POSIX (1970 epoch) timestamp parsing. */
inline void test_ntp_timestamp_parsing() {
    const int64_t unixSeconds = 1704067200;  // 2024-01-01T00:00:00Z

    TEST_ASSERT_EQUAL_INT64(2208988800LL, static_cast<int64_t>(kNtpToUnixEpochDelta));
    TEST_ASSERT_EQUAL_INT64(unixSeconds, ntpToUnixSeconds(unixToNtpTimestamp(unixSeconds)));
    TEST_ASSERT_EQUAL_INT64(unixSeconds + 2208988800LL,
                             static_cast<int64_t>(unixToNtpTimestamp(unixSeconds) >> 32));
    TEST_ASSERT_EQUAL_INT64(0, ntpToUnixSeconds(2208988800ULL << 32));

    // The time zone offset is applied on top of the UTC timestamp.
    TEST_ASSERT_EQUAL_INT64(25200, applyTimeZoneOffset(0, 25200));
    TEST_ASSERT_EQUAL_INT64(unixSeconds + 25200, applyTimeZoneOffset(unixSeconds, 25200));
    TEST_ASSERT_EQUAL_INT64(unixSeconds - 25200, applyTimeZoneOffset(unixSeconds, -25200));
}

/** @brief Dashboard flavour of the NTP / time zone calculation test. */
inline void SNTP_Timestamp_Offset_Calculation() {
    const int64_t utcSeconds = 1704067200;  // 2024-01-01T00:00:00Z
    const int32_t ictOffset = 7 * 3600;    // ICT (UTC+7)

    const DateTime local = DateTime::fromUnixTime(utcSeconds, ictOffset);
    TEST_ASSERT_EQUAL_UINT16(2024, local.year);
    TEST_ASSERT_EQUAL_UINT8(1, local.month);
    TEST_ASSERT_EQUAL_UINT8(1, local.day);
    TEST_ASSERT_EQUAL_UINT8(7, local.hour);
    TEST_ASSERT_EQUAL_UINT8(0, local.minute);
    TEST_ASSERT_EQUAL_UINT8(0, local.second);

    // Round trip through the POSIX representation.
    TEST_ASSERT_EQUAL_INT64(utcSeconds, local.toUnixTime() - ictOffset);
    TEST_ASSERT_EQUAL_INT64(utcSeconds + ictOffset, applyTimeZoneOffset(utcSeconds, ictOffset));
}

/** @brief A successful NTP synchronisation calibrates the DS1307. */
inline void DS1307_NTP_AutoCalibration_Write() {
    Test::FakeRtc rtc;
    Test::FakeSystemClock systemClock;
    Test::FakeNtpClient ntp;

    systemClock.unixTime = 1704067200;  // 2024-01-01T00:00:00Z

    TimeManager manager(rtc, systemClock, ntp);
    manager.setTimeZoneOffset(7 * 3600);

    TEST_ASSERT_TRUE(manager.requestSync());
    TEST_ASSERT_EQUAL_UINT32(1, rtc.writeCalls);
    TEST_ASSERT_EQUAL_UINT16(2024, rtc.value.year);
    TEST_ASSERT_EQUAL_UINT8(1, rtc.value.month);
    TEST_ASSERT_EQUAL_UINT8(1, rtc.value.day);
    TEST_ASSERT_EQUAL_UINT8(7, rtc.value.hour);
    TEST_ASSERT_EQUAL_UINT8(0, rtc.value.minute);

    // The calibrated RTC is also the fallback when the network disappears.
    ntp.syncSucceeds = false;
    TEST_ASSERT_FALSE(manager.requestSync());
    TEST_ASSERT_EQUAL_UINT32(1, rtc.writeCalls);
    TEST_ASSERT_EQUAL_UINT8(7, manager.now().hour);
}

// ---------------------------------------------------------------------------
// Spiral 3 - input & UI state machine
// ---------------------------------------------------------------------------

/** @brief Short press (< 1000 ms) versus long press (>= 1000 ms). */
inline void test_button_long_press_detect() {
    DebounceFilter filter(50, 1000);

    // --- long press ------------------------------------------------------
    filter.notifyPressEdge(1000);
    TEST_ASSERT_TRUE(filter.isPressed());
    TEST_ASSERT_EQUAL_UINT32(1000, filter.pressedSinceMs());

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::None), static_cast<uint8_t>(filter.update(1500)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::None), static_cast<uint8_t>(filter.update(1999)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::Long), static_cast<uint8_t>(filter.update(2000)));
    // The event has already been consumed by update().
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::None),
                            static_cast<uint8_t>(filter.takeEvent()));

    // Releasing after a long press must not emit a second event.
    filter.notifyReleaseEdge(2100);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::None),
                            static_cast<uint8_t>(filter.takeEvent()));
    TEST_ASSERT_FALSE(filter.isPressed());

    // --- short press -----------------------------------------------------
    DebounceFilter shortFilter(50, 1000);
    shortFilter.notifyPressEdge(1000);
    shortFilter.notifyReleaseEdge(1200);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::Short),
                            static_cast<uint8_t>(shortFilter.takeEvent()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::None),
                            static_cast<uint8_t>(shortFilter.takeEvent()));
}

/** @brief Dashboard flavour of the 50 ms software debounce filter test. */
inline void Button_Software_Debounce_50ms_Filter() {
    DebounceFilter filter(50, 1000);

    // A bouncing contact produces edges that are closer than 50 ms.
    filter.notifyPressEdge(1000);
    filter.notifyReleaseEdge(1020);
    TEST_ASSERT_TRUE(filter.isPressed());  // the release was swallowed
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::None),
                            static_cast<uint8_t>(filter.takeEvent()));

    // 100 ms later the button really is released.
    filter.notifyReleaseEdge(1100);
    TEST_ASSERT_FALSE(filter.isPressed());
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PressType::Short),
                            static_cast<uint8_t>(filter.takeEvent()));

    TEST_ASSERT_EQUAL_UINT32(50, filter.debounceMs());
    TEST_ASSERT_EQUAL_UINT32(1000, filter.longPressMs());
}

/** @brief The display modes are cycled in a fixed order. */
inline void test_ui_mode_cycle() {
    UiStateMachine ui;

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(UiMode::MainClock), static_cast<uint8_t>(ui.mode()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(UiMode::AlarmSet), static_cast<uint8_t>(ui.nextMode()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(UiMode::Stopwatch), static_cast<uint8_t>(ui.nextMode()));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(UiMode::ManualTime), static_cast<uint8_t>(ui.nextMode()));
    // The cycle wraps around.
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(UiMode::MainClock), static_cast<uint8_t>(ui.nextMode()));

    // The edit cursor only exists in the alarm editor.
    ui.nextMode();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EditField::Hour), static_cast<uint8_t>(ui.field()));
    ui.nextMode();
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(EditField::None), static_cast<uint8_t>(ui.field()));

    // The draft values wrap around.
    ui.setMode(UiMode::AlarmSet);
    ui.setDraft(7, 30);
    ui.setField(EditField::Hour);
    TEST_ASSERT_TRUE(ui.increment());
    TEST_ASSERT_EQUAL_UINT8(8, ui.draftHour());
    TEST_ASSERT_TRUE(ui.decrement());
    TEST_ASSERT_EQUAL_UINT8(7, ui.draftHour());
    ui.setDraft(23, 59);
    ui.setField(EditField::Hour);
    ui.increment();
    TEST_ASSERT_EQUAL_UINT8(0, ui.draftHour());
    ui.setField(EditField::Minute);
    ui.increment();
    TEST_ASSERT_EQUAL_UINT8(0, ui.draftMinute());

    TEST_ASSERT_FALSE(ui.setMode(static_cast<UiMode>(9)));
}

// ---------------------------------------------------------------------------
// Spiral 4 & 5 - engines
// ---------------------------------------------------------------------------

/** @brief The alarm comparator only fires for the configured minute. */
inline void test_alarm_time_comparator() {
    AlarmLogic logic;
    logic.setAlarm(7, 30, true);

    TEST_ASSERT_TRUE(logic.matches(7, 30));
    TEST_ASSERT_FALSE(logic.matches(7, 31));
    TEST_ASSERT_FALSE(logic.matches(8, 30));
    TEST_ASSERT_FALSE(logic.matches(6, 29));

    // The trigger is latched: one minute produces exactly one event.
    TEST_ASSERT_TRUE(logic.shouldTrigger(7, 30));
    TEST_ASSERT_FALSE(logic.shouldTrigger(7, 30));
    TEST_ASSERT_FALSE(logic.shouldTrigger(7, 30));

    logic.clearTrigger();
    TEST_ASSERT_TRUE(logic.shouldTrigger(7, 30));

    // A disabled alarm never matches.
    logic.setEnabled(false);
    TEST_ASSERT_FALSE(logic.matches(7, 30));
    TEST_ASSERT_FALSE(logic.shouldTrigger(7, 30));
}

/** @brief Dashboard flavour of the alarm comparator test. */
inline void Alarm_Comparator_Trigger_Match() {
    Test::FakeBuzzer buzzer;
    AlarmEngine engine(buzzer);

    engine.setAlarm(7, 30, true);
    TEST_ASSERT_TRUE(engine.isEnabled());
    TEST_ASSERT_EQUAL_UINT8(7, engine.hour());
    TEST_ASSERT_EQUAL_UINT8(30, engine.minute());
    TEST_ASSERT_FALSE(engine.isRinging());

    // The wrong minute does not trigger the buzzer.
    TEST_ASSERT_FALSE(engine.check(7, 29));
    TEST_ASSERT_FALSE(engine.isRinging());
    TEST_ASSERT_EQUAL_UINT32(0, buzzer.toneCalls);

    // The matching minute starts the buzzer exactly once.
    TEST_ASSERT_TRUE(engine.check(7, 30));
    TEST_ASSERT_TRUE(engine.isRinging());
    TEST_ASSERT_TRUE(buzzer.isActive());
    TEST_ASSERT_EQUAL_UINT32(1, buzzer.toneCalls);
    TEST_ASSERT_EQUAL_UINT16(AlarmEngine::kAlarmToneHz, buzzer.frequency);
    TEST_ASSERT_FALSE(engine.check(7, 30));
    TEST_ASSERT_EQUAL_UINT32(1, buzzer.toneCalls);

    // LAP / SET silences the alarm.
    engine.stopRinging();
    TEST_ASSERT_FALSE(engine.isRinging());
    TEST_ASSERT_FALSE(buzzer.isActive());
    TEST_ASSERT_EQUAL_UINT32(1, buzzer.stopCalls);
}

/** @brief Lap times and split times of the stopwatch. */
inline void test_stopwatch_lap_split_calc() {
    Test::FakeTimeSource time;
    StopwatchLogic stopwatch(time);

    stopwatch.start();
    TEST_ASSERT_TRUE(stopwatch.isRunning());
    TEST_ASSERT_EQUAL_UINT64(0, stopwatch.elapsedMs());

    time.set(1000);
    TEST_ASSERT_EQUAL_UINT64(1000, stopwatch.recordLap());
    TEST_ASSERT_EQUAL_UINT32(1, static_cast<uint32_t>(stopwatch.lapCount()));

    time.set(3500);
    TEST_ASSERT_EQUAL_UINT64(3500, stopwatch.recordLap());
    TEST_ASSERT_EQUAL_UINT32(2, static_cast<uint32_t>(stopwatch.lapCount()));

    // Cumulative times.
    TEST_ASSERT_EQUAL_UINT64(1000, stopwatch.lapTotalMs(0));
    TEST_ASSERT_EQUAL_UINT64(3500, stopwatch.lapTotalMs(1));

    // Split (delta) times.
    TEST_ASSERT_EQUAL_UINT64(1000, stopwatch.lapDeltaMs(0));
    TEST_ASSERT_EQUAL_UINT64(2500, stopwatch.lapDeltaMs(1));

    // Out of range access is safe.
    TEST_ASSERT_EQUAL_UINT64(0, stopwatch.lapTotalMs(9));
    TEST_ASSERT_EQUAL_UINT64(0, stopwatch.lapDeltaMs(9));

    // Pause freezes the counter, resume continues it.
    time.set(5000);
    stopwatch.pause();
    TEST_ASSERT_EQUAL_UINT64(5000, stopwatch.elapsedMs());
    time.set(9000);
    TEST_ASSERT_EQUAL_UINT64(5000, stopwatch.elapsedMs());
    stopwatch.start();
    TEST_ASSERT_EQUAL_UINT64(5000, stopwatch.elapsedMs());
    time.set(9500);
    TEST_ASSERT_EQUAL_UINT64(5500, stopwatch.elapsedMs());

    // Reset drops the lap memory.
    stopwatch.reset();
    TEST_ASSERT_EQUAL_UINT64(0, stopwatch.elapsedMs());
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(stopwatch.lapCount()));
}

/** @brief Dashboard flavour of the high resolution stopwatch test. */
inline void Stopwatch_HighRes_Timer_Precision() {
    Test::FakeTimeSource time;
    StopwatchEngine stopwatch(time);

    stopwatch.start();
    time.set(1);
    TEST_ASSERT_EQUAL_UINT64(1, stopwatch.elapsedMs());
    time.set(10);
    TEST_ASSERT_EQUAL_UINT64(10, stopwatch.elapsedMs());
    time.set(59999);
    TEST_ASSERT_EQUAL_UINT64(59999, stopwatch.elapsedMs());
    time.set(60000);
    TEST_ASSERT_EQUAL_UINT64(60000, stopwatch.elapsedMs());

    // The resolution is one millisecond - no rounding is involved.
    stopwatch.pause();
    TEST_ASSERT_EQUAL_UINT64(60000, stopwatch.elapsedMs());
    time.set(70000);
    TEST_ASSERT_EQUAL_UINT64(60000, stopwatch.elapsedMs());
}

/** @brief Dashboard flavour of the lap memory buffer test. */
inline void Lap_Memory_Buffer_Push() {
    Test::FakeTimeSource time;
    Stopwatch stopwatch(time);

    stopwatch.start();
    time.set(500);
    TEST_ASSERT_EQUAL_UINT64(500, stopwatch.recordLap());
    time.set(1500);
    TEST_ASSERT_EQUAL_UINT64(1500, stopwatch.recordLap());
    time.set(3000);
    TEST_ASSERT_EQUAL_UINT64(3000, stopwatch.recordLap());

    TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(stopwatch.lapCount()));
    TEST_ASSERT_EQUAL_UINT32(3, static_cast<uint32_t>(stopwatch.laps().size()));
    TEST_ASSERT_EQUAL_UINT64(500, stopwatch.laps()[0]);
    TEST_ASSERT_EQUAL_UINT64(1500, stopwatch.laps()[1]);
    TEST_ASSERT_EQUAL_UINT64(3000, stopwatch.laps()[2]);

    // The split times are derived from the cumulative times.
    TEST_ASSERT_EQUAL_UINT64(500, stopwatch.lapDeltaMs(0));
    TEST_ASSERT_EQUAL_UINT64(1000, stopwatch.lapDeltaMs(1));
    TEST_ASSERT_EQUAL_UINT64(1500, stopwatch.lapDeltaMs(2));

    // The lap memory is bounded (five entries by default).
    time.set(3500);
    stopwatch.recordLap();
    time.set(4000);
    stopwatch.recordLap();
    time.set(4500);
    stopwatch.recordLap();
    TEST_ASSERT_EQUAL_UINT32(5, static_cast<uint32_t>(stopwatch.lapCount()));

    stopwatch.reset();
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(stopwatch.lapCount()));
    TEST_ASSERT_TRUE(stopwatch.laps().empty());
}
