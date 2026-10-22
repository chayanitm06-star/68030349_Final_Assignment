#include "Debounce.hpp"

namespace Clock {

void DebounceFilter::configure(uint32_t debounceMs, uint32_t longPressMs) noexcept {
    m_debounceMs = debounceMs;
    m_longPressMs = longPressMs;
    reset();
}

void DebounceFilter::reset() noexcept {
    m_down = false;
    m_longFired = false;
    m_downSinceMs = 0;
    m_lastEdgeMs = 0;
    m_hasLastEdge = false;
    m_pending = PressType::None;
}

void DebounceFilter::latch(PressType type) noexcept {
    if (m_pending == PressType::None) {
        m_pending = type;
    }
}

void DebounceFilter::notifyPressEdge(uint32_t timestampMs) noexcept {
    // Bounce suppression: ignore edges that follow the previous one too fast.
    if (m_hasLastEdge && timestampMs >= m_lastEdgeMs && (timestampMs - m_lastEdgeMs) < m_debounceMs) {
        return;
    }
    m_lastEdgeMs = timestampMs;
    m_hasLastEdge = true;

    if (m_down) {
        return;
    }
    m_down = true;
    m_longFired = false;
    m_downSinceMs = timestampMs;
}

void DebounceFilter::notifyReleaseEdge(uint32_t timestampMs) noexcept {
    if (m_hasLastEdge && timestampMs >= m_lastEdgeMs && (timestampMs - m_lastEdgeMs) < m_debounceMs) {
        return;
    }
    m_lastEdgeMs = timestampMs;
    m_hasLastEdge = true;

    if (!m_down) {
        return;
    }

    const bool heldLongEnough = (timestampMs >= m_downSinceMs) && ((timestampMs - m_downSinceMs) >= m_longPressMs);
    m_down = false;

    if (m_longFired) {
        // The long press has already been reported by update().
        m_longFired = false;
        return;
    }

    m_longFired = false;
    latch(heldLongEnough ? PressType::Long : PressType::Short);
}

PressType DebounceFilter::update(uint32_t nowMs) noexcept {
    if (m_down && !m_longFired && nowMs >= m_downSinceMs && (nowMs - m_downSinceMs) >= m_longPressMs) {
        m_longFired = true;
        latch(PressType::Long);
    }
    return takeEvent();
}

PressType DebounceFilter::takeEvent() noexcept {
    const PressType type = m_pending;
    m_pending = PressType::None;
    return type;
}

}  // namespace Clock
