/**
 * @file DS1307.hpp
 * @brief Driver for the DS1307 battery backed real time clock.
 */
#pragma once

#include <cstdint>

#include "Bcd.hpp"
#include "DateTime.hpp"
#include "I2CDevice.hpp"
#include "Interfaces.hpp"

namespace Clock {

class DS1307 : public I2CDevice, public IRtcSource {
public:
    /** @brief Default I2C address of the DS1307. */
    static constexpr uint8_t kDefaultAddress = 0x68;

    /** @brief Register map of the DS1307. */
    enum Register : uint8_t {
        kRegSeconds = 0x00,
        kRegMinutes = 0x01,
        kRegHours = 0x02,
        kRegDay = 0x03,
        kRegDate = 0x04,
        kRegMonth = 0x05,
        kRegYear = 0x06,
        kRegControl = 0x07,
    };

    /** @brief Bit masks of the control register. */
    enum ControlBits : uint8_t {
        kBitClockHalt = 0x80,
        kBitSquareWaveEnable = 0x10,
        kBit24HourMode = 0x40,
    };

    explicit DS1307(I2CBus& bus, uint8_t address = kDefaultAddress) : I2CDevice(bus, address) {}

    /** @brief Clear the clock-halt bit and verify that the device answers. */
    esp_err_t init() override;

    /** @brief Read the current date and time. */
    bool readDateTime(DateTime& out) override;

    /** @brief Write the date and time into the RTC. */
    bool writeDateTime(const DateTime& value) override;

    /** @brief True when the oscillator is running. */
    [[nodiscard]] bool isRunning() const noexcept { return m_running; }

    // --- Static helpers (also used by the unit tests) ---------------------
    [[nodiscard]] static uint8_t bcdToDec(uint8_t value) { return bcdToDecimal(value); }
    [[nodiscard]] static uint8_t decToBcd(uint8_t value) { return decimalToBcd(value); }

private:
    bool m_running{false};
};

}  // namespace Clock
