/**
 * @file StopwatchEngine.hpp
 * @brief Domain service wrapping the pure stopwatch logic.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "Interfaces.hpp"
#include "StopwatchLogic.hpp"

namespace Clock {

class StopwatchEngine {
public:
    explicit StopwatchEngine(const ITimeSource& timeSource);

    void start() noexcept;
    void pause() noexcept;
    void toggle() noexcept;
    void reset() noexcept;

    /** @brief Record a lap and return the cumulative time. */
    uint64_t recordLap() noexcept;

    [[nodiscard]] uint64_t elapsedMs() const noexcept;
    [[nodiscard]] bool isRunning() const noexcept { return m_logic.isRunning(); }
    [[nodiscard]] size_t lapCount() const noexcept { return m_logic.lapCount(); }
    [[nodiscard]] size_t maxLaps() const noexcept { return m_logic.maxLaps(); }
    [[nodiscard]] uint64_t lapTotalMs(size_t index) const noexcept { return m_logic.lapTotalMs(index); }
    [[nodiscard]] uint64_t lapDeltaMs(size_t index) const noexcept { return m_logic.lapDeltaMs(index); }

private:
    StopwatchLogic m_logic;
};

}  // namespace Clock
