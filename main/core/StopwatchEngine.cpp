#include "StopwatchEngine.hpp"

namespace Clock {

StopwatchEngine::StopwatchEngine(const ITimeSource& timeSource) : m_logic(timeSource) {}

void StopwatchEngine::start() noexcept { m_logic.start(); }
void StopwatchEngine::pause() noexcept { m_logic.pause(); }
void StopwatchEngine::toggle() noexcept { m_logic.toggle(); }
void StopwatchEngine::reset() noexcept { m_logic.reset(); }

uint64_t StopwatchEngine::recordLap() noexcept { return m_logic.recordLap(); }

uint64_t StopwatchEngine::elapsedMs() const noexcept { return m_logic.elapsedMs(); }

}  // namespace Clock
