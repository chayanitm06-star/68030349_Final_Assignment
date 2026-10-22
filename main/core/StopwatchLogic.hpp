/**
 * @file StopwatchLogic.hpp
 * @brief Pure stopwatch / lap timing logic.
 * @details The current time is injected through @ref ITimeSource which keeps
 *          the class deterministic and unit testable.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "Interfaces.hpp"

namespace Clock {

/** @brief Stopwatch with a bounded lap memory. */
class StopwatchLogic {
public:
    /** @brief Default number of laps kept in memory. */
    static constexpr size_t kDefaultMaxLaps = 5;

    explicit StopwatchLogic(const ITimeSource& timeSource, size_t maxLaps = kDefaultMaxLaps);

    /** @brief Start (or resume) the stopwatch. */
    void start() noexcept;

    /** @brief Pause the stopwatch, keeping the accumulated time. */
    void pause() noexcept;

    /** @brief Toggle between running and paused. */
    void toggle() noexcept;

    /** @brief Reset the stopwatch and drop the lap memory. */
    void reset() noexcept;

    /** @brief Record a lap. @return Total elapsed time at the lap. */
    uint64_t recordLap() noexcept;

    /** @brief Total elapsed time in milliseconds. */
    [[nodiscard]] uint64_t elapsedMs() const noexcept;

    [[nodiscard]] bool isRunning() const noexcept { return m_running; }
    [[nodiscard]] size_t lapCount() const noexcept { return m_lapCount; }
    [[nodiscard]] size_t maxLaps() const noexcept { return m_maxLaps; }

    /** @brief Cumulative time of lap @p index (0 based). */
    [[nodiscard]] uint64_t lapTotalMs(size_t index) const noexcept;

    /** @brief Split time (delta) of lap @p index (0 based). */
    [[nodiscard]] uint64_t lapDeltaMs(size_t index) const noexcept;

private:
    const ITimeSource& m_timeSource;
    size_t m_maxLaps;
    bool m_running{false};
    uint64_t m_startedAtMs{0};
    uint64_t m_accumulatedMs{0};
    uint64_t m_laps[kDefaultMaxLaps]{};
    size_t m_lapCount{0};
};

}  // namespace Clock
