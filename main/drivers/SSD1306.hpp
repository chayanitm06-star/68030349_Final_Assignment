/**
 * @file SSD1306.hpp
 * @brief Driver for the 128x64 monochrome SSD1306 OLED display.
 * @details Owns a 1024 byte frame buffer, renders into it and flushes the
 *          whole page over I2C (page addressing mode).
 */
#pragma once

#include <cstdint>

#include "DisplayGeometry.hpp"
#include "Font5x7.hpp"
#include "I2CDevice.hpp"
#include "Interfaces.hpp"

namespace Clock {

class SSD1306 : public I2CDevice, public IDisplay {
public:
    static constexpr uint8_t kWidth = Display::kWidth;
    static constexpr uint8_t kHeight = Display::kHeight;
    static constexpr uint16_t kBufferSize = Display::kFramebufferBytes;  // 1024 bytes
    static constexpr uint8_t kDefaultAddress = 0x3C;

    /** @brief SSD1306 control bytes. */
    enum ControlByte : uint8_t {
        kControlCommand = 0x00,  ///< Co = 0, D/C# = 0 -> the following bytes are commands.
        kControlData = 0x40,     ///< Co = 1, D/C# = 1 -> the following bytes are pixels.
    };

    explicit SSD1306(I2CBus& bus, uint8_t address = kDefaultAddress) : I2CDevice(bus, address) {}

    /** @brief Send the power-on sequence and switch the panel on. */
    esp_err_t init() override;

    /** @brief Erase the frame buffer. */
    void clear() override;

    /** @brief Draw a NUL terminated string, top left corner at (x, y). */
    void drawText(uint8_t x, uint8_t y, const char* text, uint8_t scale = 1) override;

    /** @brief Draw a string that is right aligned to the display edge. */
    void drawTextRight(uint8_t y, const char* text, uint8_t scale = 1) override;

    /** @brief Draw a horizontal separator line. */
    void drawHLine(uint8_t y) override;

    /** @brief Transmit the frame buffer to the panel. */
    void flush() override;

    [[nodiscard]] uint8_t width() const override { return kWidth; }
    [[nodiscard]] uint8_t height() const override { return kHeight; }

    /** @brief Set or clear a single pixel (bounds checked). */
    void setPixel(int16_t x, int16_t y, bool on);

    /** @brief Draw a single character. */
    void drawChar(uint8_t x, uint8_t y, char value, uint8_t scale = 1);

    /** @brief Direct access to the frame buffer (used by the unit tests). */
    [[nodiscard]] const uint8_t* buffer() const noexcept { return m_buffer; }

private:
    uint8_t m_buffer[kBufferSize]{};
    uint8_t m_frame[kBufferSize + 8]{};  ///< Command prefix + frame buffer used by flush().
};

}  // namespace Clock
