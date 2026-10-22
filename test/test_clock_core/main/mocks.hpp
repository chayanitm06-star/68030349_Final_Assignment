/**
 * @file mocks.hpp
 * @brief Lightweight test doubles for every port of the domain layer.
 * @details The mocks are pure C++ (no ESP-IDF), therefore the very same test
 *          bodies can be executed on the host PC and on the ESP32 target.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "DateTime.hpp"
#include "Interfaces.hpp"

namespace Clock {
namespace Test {

/** @brief Manually advanced monotonic millisecond clock. */
class FakeTimeSource : public ITimeSource {
public:
    [[nodiscard]] uint64_t nowMs() const override { return m_nowMs; }

    void set(uint64_t milliseconds) noexcept { m_nowMs = milliseconds; }
    void advance(uint64_t milliseconds) noexcept { m_nowMs += milliseconds; }

private:
    uint64_t m_nowMs{0};
};

/** @brief Battery backed RTC test double. */
class FakeRtc : public IRtcSource {
public:
    bool readDateTime(DateTime& out) override {
        ++readCalls;
        if (!available) {
            return false;
        }
        out = value;
        return true;
    }

    bool writeDateTime(const DateTime& newValue) override {
        ++writeCalls;
        if (!available) {
            return false;
        }
        value = newValue;
        return true;
    }

    bool available{true};
    DateTime value{};
    uint32_t readCalls{0};
    uint32_t writeCalls{0};
};

/** @brief POSIX system clock test double. */
class FakeSystemClock : public ISystemClock {
public:
    bool readUnixTime(int64_t& out) const override {
        ++readCalls;
        if (!available) {
            return false;
        }
        out = unixTime;
        return true;
    }

    bool available{true};
    int64_t unixTime{0};
    mutable uint32_t readCalls{0};
};

/** @brief SNTP client test double. */
class FakeNtpClient : public INtpClient {
public:
    bool connect() override {
        ++connectCalls;
        return connected;
    }

    bool syncTime() override {
        ++syncCalls;
        if (!syncSucceeds) {
            return false;
        }
        m_synced = true;
        return true;
    }

    [[nodiscard]] bool isSynced() const override { return m_synced; }

    bool connected{true};
    bool syncSucceeds{true};
    uint32_t connectCalls{0};
    uint32_t syncCalls{0};

private:
    bool m_synced{false};
};

/** @brief Piezo buzzer test double. */
class FakeBuzzer : public IBuzzer {
public:
    void setTone(uint16_t frequencyHz, uint8_t dutyPercent) override {
        active = true;
        frequency = frequencyHz;
        duty = dutyPercent;
        ++toneCalls;
    }

    void stop() override {
        active = false;
        ++stopCalls;
    }

    [[nodiscard]] bool isActive() const override { return active; }

    bool active{false};
    uint16_t frequency{0};
    uint8_t duty{0};
    uint32_t toneCalls{0};
    uint32_t stopCalls{0};
};

/** @brief Drawing surface test double that records the rendered text. */
class FakeDisplay : public IDisplay {
public:
    void clear() override {
        ++clearCalls;
        lines.clear();
    }

    void drawText(uint8_t x, uint8_t y, const char* text, uint8_t scale) override {
        if (text == nullptr) {
            return;
        }
        lastText = text;
        lastX = x;
        lastY = y;
        lastScale = scale;
        lines.emplace_back(text);
    }

    void drawTextRight(uint8_t y, const char* text, uint8_t scale) override {
        if (text == nullptr) {
            return;
        }
        lastText = text;
        lastY = y;
        lastScale = scale;
        rightAligned.push_back(text);
    }

    void drawHLine(uint8_t y) override {
        ++hLineCalls;
        lastHLine = y;
    }

    void flush() override { ++flushCalls; }

    [[nodiscard]] uint8_t width() const override { return 128; }
    [[nodiscard]] uint8_t height() const override { return 64; }

    std::vector<std::string> lines;
    std::vector<std::string> rightAligned;
    std::string lastText;
    uint8_t lastX{0};
    uint8_t lastY{0};
    uint8_t lastScale{0};
    uint8_t lastHLine{0};
    uint32_t clearCalls{0};
    uint32_t flushCalls{0};
    uint32_t hLineCalls{0};
};

}  // namespace Test
}  // namespace Clock
