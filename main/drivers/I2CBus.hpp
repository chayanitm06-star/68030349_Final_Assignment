/**
 * @file I2CBus.hpp
 * @brief Thread-safe wrapper around the ESP-IDF v6 I2C master driver.
 * @details Every transfer is protected by a FreeRTOS mutex, therefore the RTC
 *          and the OLED can safely share the very same bus from different
 *          tasks (display task vs. alarm task vs. NTP calibration).
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Clock {

class I2CBus {
public:
    /** @brief Maximum number of devices that can be cached on the bus. */
    static constexpr size_t kMaxCachedDevices = 4;

    I2CBus(i2c_port_num_t port, gpio_num_t sdaPin, gpio_num_t sclPin, uint32_t clockHz = 400000);
    ~I2CBus();

    I2CBus(const I2CBus&) = delete;
    I2CBus& operator=(const I2CBus&) = delete;

    /** @brief Install the I2C master driver. */
    esp_err_t init();

    /** @brief True when the driver has been installed successfully. */
    [[nodiscard]] bool isReady() const noexcept { return m_bus != nullptr; }

    /** @brief Check whether a device answers on the bus. */
    esp_err_t probe(uint16_t address, int timeoutMs = 100);

    /** @brief Write a single register. */
    esp_err_t writeRegister(uint8_t deviceAddress, uint8_t reg, uint8_t value, int timeoutMs = 100);

    /** @brief Write a block of registers. */
    esp_err_t writeRegisters(uint8_t deviceAddress, uint8_t reg, const uint8_t* data, size_t length,
                             int timeoutMs = 100);

    /** @brief Write a raw byte stream (control byte + payload) to a device. */
    esp_err_t writeRaw(uint8_t deviceAddress, const uint8_t* data, size_t length, int timeoutMs = 100);

    /** @brief Read a single register. */
    esp_err_t readRegister(uint8_t deviceAddress, uint8_t reg, uint8_t* value, int timeoutMs = 100);

    /** @brief Read a block of registers. */
    esp_err_t readRegisters(uint8_t deviceAddress, uint8_t reg, uint8_t* buffer, size_t length,
                            int timeoutMs = 100);

    /** @brief Write the register address and read the answer in one transaction. */
    esp_err_t readWriteRegisters(uint8_t deviceAddress, uint8_t reg, uint8_t* buffer, size_t length,
                                  int timeoutMs = 100);

    /** @brief Take the bus mutex. @return True when the lock was acquired. */
    bool lock(uint32_t timeoutMs = portMAX_DELAY);

    /** @brief Release the bus mutex. */
    void unlock();

    /** @brief Free heap in bytes (diagnostics). */
    [[nodiscard]] uint32_t freeHeapBytes() const;

private:
    struct CachedDevice {
        uint16_t address{0};
        i2c_master_dev_handle_t handle{nullptr};
        bool used{false};
    };

    esp_err_t deviceHandle(uint8_t address, i2c_master_dev_handle_t* out);

    i2c_port_num_t m_port;
    gpio_num_t m_sdaPin;
    gpio_num_t m_sclPin;
    uint32_t m_clockHz;

    i2c_master_bus_handle_t m_bus{nullptr};
    SemaphoreHandle_t m_mutex{nullptr};
    CachedDevice m_devices[kMaxCachedDevices]{};
};

}  // namespace Clock
