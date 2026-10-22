/**
 * @file Interfaces.hpp
 * @brief Hardware abstractions (ports) implemented by the ESP-IDF driver layer.
 * @details The domain layer only talks to these interfaces, which keeps the
 *          business logic free of any `driver/...` include and therefore unit
 *          testable on a host PC.
 */
#pragma once

#include <cstdint>

#include "DateTime.hpp"

namespace Clock {

/** @brief Monotonic millisecond clock (implemented by `esp_timer_get_time`). */
class ITimeSource {
public:
    virtual ~ITimeSource() = default;
    [[nodiscard]] virtual uint64_t nowMs() const = 0;
};

/** @brief Read/write access to a battery backed real time clock. */
class IRtcSource {
public:
    virtual ~IRtcSource() = default;
    [[nodiscard]] virtual bool readDateTime(DateTime& out) = 0;
    virtual bool writeDateTime(const DateTime& value) = 0;
};

/** @brief POSIX system clock of the ESP-IDF time keeping subsystem. */
class ISystemClock {
public:
    virtual ~ISystemClock() = default;
    [[nodiscard]] virtual bool readUnixTime(int64_t& out) const = 0;
};

/** @brief Network time client (Wi-Fi station + SNTP). */
class INtpClient {
public:
    virtual ~INtpClient() = default;
    virtual bool connect() = 0;
    virtual bool syncTime() = 0;
    [[nodiscard]] virtual bool isSynced() const = 0;
};

/** @brief Drawing surface used by the UI renderer. */
class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual void clear() = 0;
    virtual void drawText(uint8_t x, uint8_t y, const char* text, uint8_t scale = 1) = 0;
    virtual void drawTextRight(uint8_t y, const char* text, uint8_t scale = 1) = 0;
    virtual void drawHLine(uint8_t y) = 0;
    virtual void flush() = 0;
    [[nodiscard]] virtual uint8_t width() const = 0;
    [[nodiscard]] virtual uint8_t height() const = 0;
};

/** @brief Piezo buzzer / passive tone generator. */
class IBuzzer {
public:
    virtual ~IBuzzer() = default;
    virtual void setTone(uint16_t frequencyHz, uint8_t dutyPercent) = 0;
    virtual void stop() = 0;
    [[nodiscard]] virtual bool isActive() const = 0;
};

}  // namespace Clock
