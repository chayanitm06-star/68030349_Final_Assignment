/**
 * @file I2CDevice.hpp
 * @brief Abstract base class of every device that sits on the I2C bus.
 * @details Mirrors the UML class diagram of the assignment (DS1307 and SSD1306
 *          derive from I2CDevice).
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "I2CBus.hpp"
#include "esp_err.h"

namespace Clock {

class I2CDevice {
public:
    I2CDevice(I2CBus& bus, uint8_t address) : m_bus(bus), m_address(address) {}
    virtual ~I2CDevice() = default;

    I2CDevice(const I2CDevice&) = delete;
    I2CDevice& operator=(const I2CDevice&) = delete;

    /** @brief Initialise the device (send the reset/initialisation sequence). */
    virtual esp_err_t init() = 0;

    /** @brief Probe the device on the bus. */
    esp_err_t probe() { return m_bus.probe(m_address); }

    [[nodiscard]] uint8_t address() const noexcept { return m_address; }
    [[nodiscard]] I2CBus& bus() const noexcept { return m_bus; }

protected:
    esp_err_t writeRegister(uint8_t reg, uint8_t value, int timeoutMs = 100) {
        return m_bus.writeRegister(m_address, reg, value, timeoutMs);
    }

    esp_err_t writeRegisters(uint8_t reg, const uint8_t* data, size_t length, int timeoutMs = 100) {
        return m_bus.writeRegisters(m_address, reg, data, length, timeoutMs);
    }

    esp_err_t readRegisters(uint8_t reg, uint8_t* buffer, size_t length, int timeoutMs = 100) {
        return m_bus.readRegisters(m_address, reg, buffer, length, timeoutMs);
    }

    I2CBus& m_bus;
    uint8_t m_address;
};

}  // namespace Clock
