/**
 * @file ClockConfig.hpp
 * @brief Single source of truth for every board level constant.
 * @details Changing a pin, a credential or a task parameter only requires an
 *          edit in this header - the drivers, the tasks and the Wokwi
 *          `diagram.json` all derive from it.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "ButtonEvent.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

namespace Clock {
namespace Config {

// ---------------------------------------------------------------------------
// I2C bus (shared by the DS1307 RTC and the SSD1306 OLED)
// ---------------------------------------------------------------------------
inline constexpr i2c_port_num_t kI2cPort = I2C_NUM_0;
inline constexpr gpio_num_t kI2cSdaPin = GPIO_NUM_21;
inline constexpr gpio_num_t kI2cSclPin = GPIO_NUM_22;
inline constexpr uint32_t kI2cClockHz = 400000;  ///< Fast mode (400 kHz).
inline constexpr uint8_t kI2cGlitchIgnoreCnt = 7;

// ---------------------------------------------------------------------------
// Peripheral addresses
// ---------------------------------------------------------------------------
inline constexpr uint8_t kDs1307Address = 0x68;
inline constexpr uint8_t kSsd1306Address = 0x3C;

// ---------------------------------------------------------------------------
// Panel buttons (active low, internal pull-up)
// ---------------------------------------------------------------------------
inline constexpr gpio_num_t kButtonModePin = GPIO_NUM_12;  ///< MODE
inline constexpr gpio_num_t kButtonIncPin = GPIO_NUM_13;   ///< INC (+)
inline constexpr gpio_num_t kButtonSetPin = GPIO_NUM_14;   ///< SET
inline constexpr gpio_num_t kButtonLapPin = GPIO_NUM_27;   ///< LAP / RESET

/** @brief Physical order of the buttons (index = slot id). */
inline constexpr gpio_num_t kButtonPins[kButtonCount] = {kButtonModePin, kButtonIncPin, kButtonSetPin,
                                                         kButtonLapPin};

/** @brief Logical order of the buttons (index = slot id). */
inline constexpr ButtonId kButtonIds[kButtonCount] = {ButtonId::Mode, ButtonId::Inc, ButtonId::Set,
                                                      ButtonId::Lap};

inline constexpr uint32_t kButtonDebounceMs = 50;  ///< Debounce window.
inline constexpr uint32_t kLongPressMs = 1000;      ///< Long press threshold.
inline constexpr size_t kButtonQueueLength = 10;

// ---------------------------------------------------------------------------
// Piezo buzzer (LEDC PWM)
// ---------------------------------------------------------------------------
inline constexpr gpio_num_t kBuzzerPin = GPIO_NUM_25;
inline constexpr uint32_t kBuzzerFrequencyHz = 2700;
inline constexpr uint8_t kBuzzerDutyPercent = 50;

// ---------------------------------------------------------------------------
// Wi-Fi / NTP
// ---------------------------------------------------------------------------
inline constexpr const char* kWifiSsid = "iotexpo";
inline constexpr const char* kWifiPassword = "12345678";
inline constexpr const char* kNtpServer = "pool.ntp.org";
inline constexpr const char* kNtpServerFallback = "time.nist.gov";
inline constexpr int32_t kTimeZoneOffsetSeconds = 7 * 3600;  ///< ICT (UTC+7).
inline constexpr uint32_t kNtpSyncPeriodSeconds = 3600;      ///< Resync every hour.
inline constexpr uint32_t kWifiConnectTimeoutMs = 15000;

// ---------------------------------------------------------------------------
// FreeRTOS task parameters (see the architecture documentation)
// ---------------------------------------------------------------------------
inline constexpr uint32_t kDisplayPeriodMs = 100;   ///< 10 FPS refresh.
inline constexpr uint32_t kAlarmPeriodMs = 1000;    ///< 1 Hz alarm check.
inline constexpr uint32_t kNtpPeriodSeconds = 3600;  ///< 1 h NTP resync.

inline constexpr uint32_t kDisplayTaskStackBytes = 4096;
inline constexpr uint32_t kButtonTaskStackBytes = 3072;
inline constexpr uint32_t kAlarmTaskStackBytes = 2048;
inline constexpr uint32_t kNtpTaskStackBytes = 6144;

inline constexpr UBaseType_t kDisplayTaskPriority = 3;
inline constexpr UBaseType_t kButtonTaskPriority = 5;
inline constexpr UBaseType_t kAlarmTaskPriority = 4;
inline constexpr UBaseType_t kNtpTaskPriority = 1;

inline constexpr BaseType_t kDisplayTaskCore = 1;
inline constexpr BaseType_t kButtonTaskCore = 0;
inline constexpr BaseType_t kAlarmTaskCore = 1;
inline constexpr BaseType_t kNtpTaskCore = 0;

// ---------------------------------------------------------------------------
// Miscellaneous
// ---------------------------------------------------------------------------
inline constexpr size_t kMaxLapCount = 5;  ///< Lap memory of the stopwatch.
inline constexpr uint32_t kAlarmRingingTimeoutMs = 60000;

}  // namespace Config
}  // namespace Clock
