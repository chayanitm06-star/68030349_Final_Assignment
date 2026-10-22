/**
 * @file Bcd.hpp
 * @brief Pure BCD <-> decimal conversion helpers.
 * @details Part of the bottom-up architecture: this header has **zero** ESP-IDF
 *          dependencies, therefore it can be compiled and unit-tested on the
 *          host PC as well as on the ESP32 target.
 */
#pragma once

#include <cstdint>

namespace Clock {

/**
 * @brief Convert a packed BCD byte into its decimal value.
 * @param bcd Packed BCD byte, e.g. 0x23.
 * @return Decimal value, e.g. 23.
 */
[[nodiscard]] constexpr uint8_t bcdToDecimal(uint8_t bcd) noexcept {
    return static_cast<uint8_t>((static_cast<uint8_t>(bcd >> 4) * 10u) + static_cast<uint8_t>(bcd & 0x0Fu));
}

/**
 * @brief Convert a decimal value into a packed BCD byte.
 * @param value Decimal value 0..99, e.g. 45.
 * @return Packed BCD byte, e.g. 0x45.
 */
[[nodiscard]] constexpr uint8_t decimalToBcd(uint8_t value) noexcept {
    return static_cast<uint8_t>(((value / 10u) << 4) | (value % 10u));
}

/**
 * @brief Check that every nibble of @p bcd is a legal BCD digit (0..9).
 */
[[nodiscard]] constexpr bool isValidBcd(uint8_t bcd) noexcept {
    return (static_cast<uint8_t>(bcd >> 4) <= 9u) && (static_cast<uint8_t>(bcd & 0x0Fu) <= 9u);
}

}  // namespace Clock
