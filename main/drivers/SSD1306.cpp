#include "SSD1306.hpp"

#include <cstring>

#include "esp_log.h"

namespace Clock {
namespace {

constexpr const char* kTag = "SSD1306";

// Timeout of a single I2C transfer of the display. The frame buffer is sent
// page by page (129 bytes per transfer), so the timeout only matters if the
// bus is stuck - it is generous enough for the slow simulator.
constexpr int kI2cTimeoutMs = 500;

// Power on sequence of a 128x64 SSD1306 module (page addressing mode).
constexpr uint8_t kInitSequence[] = {
    0xAE,        // display off
    0xD5, 0x80,  // clock divide / oscillator frequency
    0xA8, 0x3F,  // multiplex ratio = 64
    0xD3, 0x00,  // display offset
    0x40,        // start line = 0
    0x8D, 0x14,  // charge pump on
    0x20, 0x00,  // horizontal memory addressing mode
    0xA1,        // segment remap (column 127 -> SEG0)
    0xC8,        // COM scan direction (remapped)
    0xDA, 0x12,  // COM pins configuration for 128x64
    0x81, 0x7F,  // contrast
    0xD9, 0xF1,  // pre-charge period
    0xDB, 0x40,  // VCOMH deselect level
    0xA4,        // output follows RAM
    0xA6,        // normal (not inverted) display
    0x2E,        // deactivate scrolling
    0xAF,        // display on
};

}  // namespace

esp_err_t SSD1306::init() {
    const esp_err_t probeResult = probe();
    if (probeResult != ESP_OK) {
        ESP_LOGE(kTag, "no answer on address 0x%02X", static_cast<unsigned>(m_address));
        return probeResult;
    }

    // [control byte][commands...]
    uint8_t payload[sizeof(kInitSequence) + 1];
    payload[0] = kControlCommand;
    std::memcpy(payload + 1, kInitSequence, sizeof(kInitSequence));

    const esp_err_t err = m_bus.writeRaw(m_address, payload, sizeof(payload), kI2cTimeoutMs);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "init sequence failed: %s", esp_err_to_name(err));
        return err;
    }

    clear();
    flush();

    if (m_flushErrorCount != 0u) {
        // The panel answered but the first frame did not reach it.
        ESP_LOGE(kTag, "the panel answered but the first frame could not be written");
        return ESP_FAIL;
    }

    ESP_LOGI(kTag, "display ready on address 0x%02X (%dx%d, %u byte frame buffer)",
             static_cast<unsigned>(m_address), static_cast<int>(kWidth), static_cast<int>(kHeight),
             static_cast<unsigned>(kBufferSize));
    return ESP_OK;
}

void SSD1306::clear() {
    std::memset(m_buffer, 0, sizeof(m_buffer));
}

void SSD1306::setPixel(int16_t x, int16_t y, bool on) {
    if (x < 0 || x >= static_cast<int16_t>(kWidth) || y < 0 || y >= static_cast<int16_t>(kHeight)) {
        return;  // bounds check - the unit test verifies exactly this
    }
    const uint16_t index = static_cast<uint16_t>(x + (y / 8) * kWidth);
    if (on) {
        m_buffer[index] |= static_cast<uint8_t>(1u << (y % 8));
    } else {
        m_buffer[index] &= static_cast<uint8_t>(~(1u << (y % 8)));
    }
}

void SSD1306::drawChar(uint8_t x, uint8_t y, char value, uint8_t scale) {
    const auto code = static_cast<uint8_t>(value);
    if (code < kFontFirstChar || code > kFontLastChar) {
        return;
    }
    if (scale == 0) {
        scale = 1;
    }

    const uint8_t* glyph = kFont5x7[code - kFontFirstChar];
    for (uint8_t col = 0; col < kGlyphWidth; ++col) {
        for (uint8_t row = 0; row < kGlyphHeight; ++row) {
            if ((glyph[col] & (1u << row)) == 0) {
                continue;
            }
            for (uint8_t sy = 0; sy < scale; ++sy) {
                for (uint8_t sx = 0; sx < scale; ++sx) {
                    setPixel(static_cast<int16_t>(x + col * scale + sx),
                             static_cast<int16_t>(y + row * scale + sy), true);
                }
            }
        }
    }
}

void SSD1306::drawText(uint8_t x, uint8_t y, const char* text, uint8_t scale) {
    if (text == nullptr) {
        return;
    }
    if (scale == 0) {
        scale = 1;
    }

    uint16_t cursor = x;
    for (const char* it = text; *it != '\0'; ++it) {
        if (cursor > kWidth) {
            break;
        }
        drawChar(static_cast<uint8_t>(cursor), y, *it, scale);
        cursor = static_cast<uint16_t>(cursor + (kGlyphWidth + 1) * scale);
    }
}

void SSD1306::drawTextRight(uint8_t y, const char* text, uint8_t scale) {
    if (text == nullptr) {
        return;
    }
    if (scale == 0) {
        scale = 1;
    }

    size_t length = 0;
    while (text[length] != '\0') {
        ++length;
    }
    const uint16_t textWidth = static_cast<uint16_t>(length * (kGlyphWidth + 1) * scale);
    const uint16_t x = (textWidth >= kWidth) ? 0 : static_cast<uint16_t>(kWidth - textWidth);
    drawText(static_cast<uint8_t>(x), y, text, scale);
}

void SSD1306::drawHLine(uint8_t y) {
    for (uint8_t x = 0; x < kWidth; ++x) {
        setPixel(x, y, true);
    }
}

void SSD1306::flush() {
    // The SSD1306 evaluates the control byte (Co / D/C#) ONLY at the beginning
    // of an I2C transfer:
    //   0x00 -> every following byte of this transfer is a command
    //   0x40 -> every following byte of this transfer is pixel data
    // Commands and pixels therefore have to travel in SEPARATE transfers -
    // mixing them makes the controller read the pixels as commands and the
    // panel stays empty (the same rule the Adafruit_SSD1306 library uses).
    const uint8_t addressing[] = {
        kControlCommand,
        0x21, 0x00, 0x7F,  // column address range 0..127
        0x22, 0x00, 0x07,  // page address range 0..7
    };

    esp_err_t err = m_bus.writeRaw(m_address, addressing, sizeof(addressing), kI2cTimeoutMs);

    // One transfer per page: the data pointer continues where the previous
    // page ended (horizontal addressing mode), so the panel is filled from
    // the top left corner to the bottom right corner.
    for (uint8_t page = 0; (page < kPageCount) && (err == ESP_OK); ++page) {
        m_frame[0] = kControlData;
        std::memcpy(m_frame + 1, m_buffer + (page * kPageBytes), kPageBytes);

        err = m_bus.writeRaw(m_address, m_frame, kPageBytes + 1u, kI2cTimeoutMs);
    }

    if (err != ESP_OK) {
        m_flushErrorCount++;
        // Log the first failure and then every tenth one - otherwise the
        // serial monitor would be flooded with one line per frame.
        if ((m_flushErrorCount == 1u) || ((m_flushErrorCount % 10u) == 0u)) {
            ESP_LOGE(kTag, "flush failed (%u times): %s", static_cast<unsigned>(m_flushErrorCount),
                     esp_err_to_name(err));
        }
    }
}

}  // namespace Clock
