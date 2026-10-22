/**
 * @file Stopwatch.hpp
 * @brief Application level stopwatch service (dashboard compatible API).
 *
 * Wraps the pure @ref Clock::StopwatchEngine and keeps a copy of the lap
 * memory in a @c std::vector, exactly like the reference implementation of
 * the assignment dashboard.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "Interfaces.hpp"
#include "StopwatchEngine.hpp"

namespace Clock {

class Stopwatch {
public:
    explicit Stopwatch(const ITimeSource& timeSource, size_t maxLaps = StopwatchLogic::kDefaultMaxLaps);

    /** @brief Start (or resume) the stopwatch. */
    void start();

    /** @brief Pause the stopwatch, keeping the accumulated time. */
    void pause();

    /** @brief Toggle between running and paused. */
    void toggle();

    /** @brief Reset the stopwatch and drop the lap memory. */
    void reset();

    /** @brief Record a lap and return the cumulative time in milliseconds. */
    uint64_t recordLap();

    /** @brief Total elapsed time in milliseconds. */
    [[nodiscard]] uint64_t getElapsedMs() const;

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] size_t lapCount() const;
    [[nodiscard]] uint64_t lapTotalMs(size_t index) const;
    [[nodiscard]] uint64_t lapDeltaMs(size_t index) const;

    /** @brief Copy of the lap memory (cumulative times in milliseconds). */
    [[nodiscard]] const std::vector<uint64_t>& laps() const noexcept { return m_laps; }

    /** @brief Access to the underlying domain service. */
    [[nodiscard]] StopwatchEngine& engine() noexcept { return m_engine; }
    [[nodiscard]] const StopwatchEngine& engine() const noexcept { return m_engine; }

private:
    StopwatchEngine m_engine;
    std::vector<uint64_t> m_laps;
};

}  // namespace Clock
