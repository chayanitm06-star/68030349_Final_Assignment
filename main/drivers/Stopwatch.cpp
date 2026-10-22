#include "Stopwatch.hpp"

namespace Clock {

Stopwatch::Stopwatch(const ITimeSource& timeSource, size_t maxLaps)
    : m_engine(timeSource) {
    m_laps.reserve(maxLaps);
}

void Stopwatch::start() { m_engine.start(); }

void Stopwatch::pause() { m_engine.pause(); }

void Stopwatch::toggle() { m_engine.toggle(); }

void Stopwatch::reset() {
    m_engine.reset();
    m_laps.clear();
}

uint64_t Stopwatch::recordLap() {
    const uint64_t total = m_engine.recordLap();
    if (m_laps.size() < m_engine.maxLaps()) {
        m_laps.push_back(total);
    }
    return total;
}

uint64_t Stopwatch::getElapsedMs() const { return m_engine.elapsedMs(); }

bool Stopwatch::isRunning() const { return m_engine.isRunning(); }

size_t Stopwatch::lapCount() const { return m_engine.lapCount(); }

uint64_t Stopwatch::lapTotalMs(size_t index) const { return m_engine.lapTotalMs(index); }

uint64_t Stopwatch::lapDeltaMs(size_t index) const { return m_engine.lapDeltaMs(index); }

}  // namespace Clock
