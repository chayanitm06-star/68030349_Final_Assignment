/**
 * @file SystemClock.hpp
 * @brief Adapter that exposes the POSIX system clock of ESP-IDF.
 */
#pragma once

#include <cstdint>

#include "Interfaces.hpp"

namespace Clock {

class SystemClock : public ISystemClock {
public:
    /** @brief Read the POSIX system clock (seconds since 1970-01-01Z). */
    [[nodiscard]] bool readUnixTime(int64_t& out) const override;
};

}  // namespace Clock
