#include "DS1307.hpp"

#include "esp_log.h"

namespace Clock {
namespace {
constexpr const char* kTag = "DS1307";
}  // namespace

esp_err_t DS1307::init() {
    const esp_err_t probeResult = probe();
    if (probeResult != ESP_OK) {
        ESP_LOGE(kTag, "no answer on address 0x%02X", static_cast<unsigned>(m_address));
        return probeResult;
    }

    uint8_t seconds = 0;
    esp_err_t err = readRegisters(kRegSeconds, &seconds, 1);
    if (err != ESP_OK) {
        return err;
    }

    if ((seconds & kBitClockHalt) != 0) {
        // The oscillator was stopped (e.g. by a flat battery) -> restart it.
        ESP_LOGW(kTag, "clock halt bit set, restarting the oscillator");
        err = writeRegister(kRegSeconds, static_cast<uint8_t>(seconds & ~kBitClockHalt));
        if (err != ESP_OK) {
            return err;
        }
    }

    // Enable the 24 hour mode so that the hour register always uses 0..23.
    uint8_t control = 0;
    err = readRegisters(kRegControl, &control, 1);
    if (err != ESP_OK) {
        return err;
    }
    if ((control & kBit24HourMode) == 0) {
        err = writeRegister(kRegControl, static_cast<uint8_t>(control | kBit24HourMode));
        if (err != ESP_OK) {
            return err;
        }
    }

    m_running = true;
    ESP_LOGI(kTag, "RTC ready on address 0x%02X", static_cast<unsigned>(m_address));
    return ESP_OK;
}

bool DS1307::readDateTime(DateTime& out) {
    uint8_t raw[7] = {0};
    if (readRegisters(kRegSeconds, raw, sizeof(raw)) != ESP_OK) {
        return false;
    }

    // 0x00 seconds | 0x01 minutes | 0x02 hours | 0x03 day | 0x04 date | 0x05 month | 0x06 year
    out.second = bcdToDecimal(static_cast<uint8_t>(raw[0] & 0x7F));
    out.minute = bcdToDecimal(raw[1]);
    out.hour = bcdToDecimal(static_cast<uint8_t>(raw[2] & 0x3F));
    out.dayOfWeek = bcdToDecimal(static_cast<uint8_t>(raw[3] & 0x07));
    out.day = bcdToDecimal(static_cast<uint8_t>(raw[4] & 0x3F));
    out.month = bcdToDecimal(static_cast<uint8_t>(raw[5] & 0x1F));
    out.year = static_cast<uint16_t>(2000 + bcdToDecimal(raw[6]));

    return out.isValid();
}

bool DS1307::writeDateTime(const DateTime& value) {
    if (!value.isValid()) {
        return false;
    }

    uint8_t payload[7];
    payload[0] = decimalToBcd(value.second);
    payload[1] = decimalToBcd(value.minute);
    payload[2] = decimalToBcd(value.hour);   // 24h mode -> bit 6 is the mode flag
    payload[3] = decimalToBcd(value.dayOfWeek);
    payload[4] = decimalToBcd(value.day);
    payload[5] = decimalToBcd(value.month);
    payload[6] = decimalToBcd(static_cast<uint8_t>(value.year % 100));

    if (writeRegisters(kRegSeconds, payload, sizeof(payload)) != ESP_OK) {
        return false;
    }
    return true;
}

}  // namespace Clock
