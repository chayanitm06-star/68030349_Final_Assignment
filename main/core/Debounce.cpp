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
    m_pendingRelease = false;
    m_pending = PressType::None;
}

void DebounceFilter::latch(PressType type) noexcept {
    if (m_pending == PressType::None) {
        m_pending = type;
    }
}

void DebounceFilter::notifyPressEdge(uint32_t timestampMs) noexcept {
    // A press invalidates a deferred release - the button is down again.
    m_pendingRelease = false;

    // Bounce suppression: ignore edges that follow the previous one too fast.
    if (m_hasLastEdge && timestampMs >= m_lastEdgeMs && (timestampMs - m_lastEdgeMs) < m_debounceMs) {
        return;
    }
    m_lastEdgeMs = timestampMs;
    m_hasLastEdge = true;

    if (m_down) {
        // The release of the previous press was missed (or was deferred and
        // got cancelled by this edge). Restart the long press timer so that
        // the filter cannot report a bogus long press later on.
        m_downSinceMs = timestampMs;
        return;
    }
    m_down = true;
    m_longFired = false;
    m_downSinceMs = timestampMs;
}

void DebounceFilter::notifyReleaseEdge(uint32_t timestampMs) noexcept {
    if (m_hasLastEdge && timestampMs >= m_lastEdgeMs && (timestampMs - m_lastEdgeMs) < m_debounceMs) {
        // The release happened inside the debounce window. It is either a
        // bounce or a very fast click, so the decision is deferred until the
        // window has passed without a new press edge (see update()).
        m_pendingRelease = true;
        return;
    }
    m_lastEdgeMs = timestampMs;
    m_hasLastEdge = true;
    m_pendingRelease = false;

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
    // Confirm a deferred release: the button stayed released for the whole
    // debounce window, therefore the release was real and not a bounce.
    if (m_pendingRelease && m_down && m_hasLastEdge && nowMs >= m_lastEdgeMs &&
        (nowMs - m_lastEdgeMs) >= m_debounceMs) {
        m_pendingRelease = false;
        m_down = false;
        if (m_longFired) {
            // The long press has already been reported.
            m_longFired = false;
        } else {
            latch(PressType::Short);
        }
    }

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
