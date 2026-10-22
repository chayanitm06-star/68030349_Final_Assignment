#include "StopwatchLogic.hpp"

namespace Clock {

StopwatchLogic::StopwatchLogic(const ITimeSource& timeSource, size_t maxLaps)
    : m_timeSource(timeSource), m_maxLaps(maxLaps > kDefaultMaxLaps ? kDefaultMaxLaps : maxLaps) {}

void StopwatchLogic::start() noexcept {
    if (m_running) {
        return;
    }
    m_startedAtMs = m_timeSource.nowMs();
    m_running = true;
}

void StopwatchLogic::pause() noexcept {
    if (!m_running) {
        return;
    }
    m_accumulatedMs += m_timeSource.nowMs() - m_startedAtMs;
    m_running = false;
}

void StopwatchLogic::toggle() noexcept {
    if (m_running) {
        pause();
    } else {
        start();
    }
}

void StopwatchLogic::reset() noexcept {
    m_running = false;
    m_startedAtMs = 0;
    m_accumulatedMs = 0;
    m_lapCount = 0;
}

uint64_t StopwatchLogic::recordLap() noexcept {
    if (m_lapCount < m_maxLaps) {
        m_laps[m_lapCount] = elapsedMs();
        ++m_lapCount;
    }
    return elapsedMs();
}

uint64_t StopwatchLogic::elapsedMs() const noexcept {
    if (m_running) {
        return m_accumulatedMs + (m_timeSource.nowMs() - m_startedAtMs);
    }
    return m_accumulatedMs;
}

uint64_t StopwatchLogic::lapTotalMs(size_t index) const noexcept {
    if (index >= m_lapCount) {
        return 0;
    }
    return m_laps[index];
}

uint64_t StopwatchLogic::lapDeltaMs(size_t index) const noexcept {
    if (index >= m_lapCount) {
        return 0;
    }
    if (index == 0) {
        return m_laps[0];
    }
    return m_laps[index] - m_laps[index - 1];
}

}  // namespace Clock
