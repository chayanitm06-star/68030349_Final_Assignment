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
// The I2C master driver header provides "i2c_port_num_t" and the port
// constants (I2C_NUM_0 / I2C_NUM_1) - it is required for the constants below.
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

namespace Clock {

/**
 * @brief Board level constants of the clock.
 * @details It is a struct with static constexpr members (not a namespace) so
 *          that every constant is a real member: a typo such as
 *          "Config::kI2cProt" is reported by the compiler as
 *          "'struct Clock::Config' has no member named 'kI2cProt'" instead of
 *          silently resolving to something else.
 */
struct Config {
    // -----------------------------------------------------------------------
    // I2C bus (shared by the DS1307 RTC and the SSD1306 OLED)
    // -----------------------------------------------------------------------
    static constexpr i2c_port_num_t kI2cPort = I2C_NUM_0;
    static constexpr gpio_num_t kI2cSdaPin = GPIO_NUM_21;
    static constexpr gpio_num_t kI2cSclPin = GPIO_NUM_22;
    static constexpr uint32_t kI2cClockHz = 400000;  ///< Fast mode (400 kHz).
    static constexpr uint8_t kI2cGlitchIgnoreCnt = 7;

    // -----------------------------------------------------------------------
    // Peripheral addresses
    // -----------------------------------------------------------------------
    static constexpr uint8_t kDs1307Address = 0x68;
    static constexpr uint8_t kSsd1306Address = 0x3C;

    // -----------------------------------------------------------------------
    // Panel buttons (active low, internal pull-up)
    // -----------------------------------------------------------------------
    static constexpr gpio_num_t kButtonModePin = GPIO_NUM_12;  ///< MODE
    static constexpr gpio_num_t kButtonIncPin = GPIO_NUM_13;   ///< INC (+)
    static constexpr gpio_num_t kButtonSetPin = GPIO_NUM_14;   ///< SET
    static constexpr gpio_num_t kButtonLapPin = GPIO_NUM_27;   ///< LAP / RESET

    /** @brief Physical order of the buttons (index = slot id). */
    static constexpr gpio_num_t kButtonPins[kButtonCount] = {kButtonModePin, kButtonIncPin, kButtonSetPin,
                                                             kButtonLapPin};

    /** @brief Logical order of the buttons (index = slot id). */
    static constexpr ButtonId kButtonIds[kButtonCount] = {ButtonId::Mode, ButtonId::Inc, ButtonId::Set,
                                                          ButtonId::Lap};

    static constexpr uint32_t kButtonDebounceMs = 50;  ///< Debounce window.
    static constexpr uint32_t kLongPressMs = 1000;      ///< Long press threshold.
    static constexpr size_t kButtonQueueLength = 10;

    // -----------------------------------------------------------------------
    // Piezo buzzer (LEDC PWM)
    // -----------------------------------------------------------------------
    static constexpr gpio_num_t kBuzzerPin = GPIO_NUM_25;
    static constexpr uint32_t kBuzzerFrequencyHz = 2700;
    static constexpr uint8_t kBuzzerDutyPercent = 50;

    // -----------------------------------------------------------------------
    // Wi-Fi / NTP
    // -----------------------------------------------------------------------
    static constexpr const char* kWifiSsid = "iotexpo";
    static constexpr const char* kWifiPassword = "12345678";
    // เครือข่ายสำรอง: ตัวจำลอง Wokwi เปิดเครือข่าย "Wokwi-GUEST" (ไม่มีรหัส) —
    // ถ้า SSID หลักติดต่อไม่ได้ ฟิร์มแวร์จะลองสลับไปใช้เครือข่ายนี้อัตโนมัติ
    static constexpr const char* kWifiFallbackSsid = "Wokwi-GUEST";
    static constexpr const char* kWifiFallbackPassword = "";
    static constexpr const char* kNtpServer = "pool.ntp.org";
    static constexpr const char* kNtpServerFallback = "time.nist.gov";
    static constexpr int32_t kTimeZoneOffsetSeconds = 7 * 3600;  ///< ICT (UTC+7).
    static constexpr uint32_t kNtpSyncPeriodSeconds = 3600;      ///< Resync every hour.
    static constexpr uint32_t kWifiConnectTimeoutMs = 15000;

    // -----------------------------------------------------------------------
    // FreeRTOS task parameters (see the architecture documentation)
    // -----------------------------------------------------------------------
    static constexpr uint32_t kDisplayPeriodMs = 100;   ///< 10 FPS refresh.
    static constexpr uint32_t kAlarmPeriodMs = 1000;    ///< 1 Hz alarm check.
    static constexpr uint32_t kNtpPeriodSeconds = 3600;  ///< 1 h NTP resync.

    static constexpr uint32_t kDisplayTaskStackBytes = 4096;
    static constexpr uint32_t kButtonTaskStackBytes = 3072;
    static constexpr uint32_t kAlarmTaskStackBytes = 2048;
    static constexpr uint32_t kNtpTaskStackBytes = 6144;

    static constexpr UBaseType_t kDisplayTaskPriority = 3;
    static constexpr UBaseType_t kButtonTaskPriority = 5;
    static constexpr UBaseType_t kAlarmTaskPriority = 4;
    static constexpr UBaseType_t kNtpTaskPriority = 1;

    static constexpr BaseType_t kDisplayTaskCore = 1;
    static constexpr BaseType_t kButtonTaskCore = 0;
    static constexpr BaseType_t kAlarmTaskCore = 1;
    static constexpr BaseType_t kNtpTaskCore = 0;

    // -----------------------------------------------------------------------
    // Miscellaneous
    // -----------------------------------------------------------------------
    static constexpr size_t kMaxLapCount = 5;  ///< Lap memory of the stopwatch.
    static constexpr uint32_t kAlarmRingingTimeoutMs = 60000;
};

}  // namespace Clock
