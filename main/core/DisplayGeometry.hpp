/**
 * @file DisplayGeometry.hpp
 * @brief Pure geometry constants of the 128x64 SSD1306 panel.
 * @details Kept free of ESP-IDF dependencies so that the unit tests can
 *          verify the frame buffer layout on the host as well.
 */
#pragma once

#include <cstdint>

namespace Clock {
namespace Display {

/** @brief Panel width in pixels. */
inline constexpr uint8_t kWidth = 128;

/** @brief Panel height in pixels. */
inline constexpr uint8_t kHeight = 64;

/** @brief Size of the monochrome frame buffer (128 * 64 / 8). */
inline constexpr uint16_t kFramebufferBytes = static_cast<uint16_t>((kWidth * kHeight) / 8);

}  // namespace Display
}  // namespace Clock
