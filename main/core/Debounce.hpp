/**
 * @file Debounce.hpp
 * @brief Software debouncer with short/long press recognition.
 *
 * The filter is driven by two sources:
 *   - @ref notifyPressEdge / @ref notifyReleaseEdge are called from the GPIO
 *     interrupt handler and carry the exact edge timestamp.
 *   - @ref update is called periodically by the button task and reports the
 *     long press as soon as the threshold is exceeded.
 *
 * Because the timestamps are injected, the behaviour is fully deterministic and
 * can be unit-tested without any sleeping.
 */
#pragma once

#include <cstdint>

namespace Clock {

/** @brief Result of a debounced button evaluation. */
enum class PressType : uint8_t {
    None = 0,  ///< No new event.
    Short = 1, ///< Button released before the long press threshold.
    Long = 2,  ///< Button held at least `longPressMs`.
};

/**
 * @brief Debounce filter for a single active-low push button.
 *
 * The filter rejects the bouncing of a mechanical contact: an edge that
 * follows the previous one within @ref debounceMs is considered a bounce.
 *
 * A *release* edge inside that window is neither accepted nor dropped - it
 * is **deferred** and confirmed by @ref update as soon as the window has
 * passed without a new press edge. Without that rule a click that is
 * shorter than the debounce window (a fast mouse click in the Wokwi
 * simulator!) would be swallowed completely and the button would look
 * broken.
 */
class DebounceFilter {
public:
    DebounceFilter() = default;
    DebounceFilter(uint32_t debounceMs, uint32_t longPressMs) { configure(debounceMs, longPressMs); }

    /** @brief Configure the debounce window and the long press threshold. */
    void configure(uint32_t debounceMs, uint32_t longPressMs) noexcept;

    /** @brief ISR hook: the button was pressed at @p timestampMs. */
    void notifyPressEdge(uint32_t timestampMs) noexcept;

    /** @brief ISR hook: the button was released at @p timestampMs. */
    void notifyReleaseEdge(uint32_t timestampMs) noexcept;

    /**
     * @brief Periodic task hook - reports the long press when the button is
     *        held longer than the configured threshold.
     */
    PressType update(uint32_t nowMs) noexcept;

    /** @brief Fetch and clear the pending event. */
    [[nodiscard]] PressType takeEvent() noexcept;

    /** @brief Forget the current state. */
    void reset() noexcept;

    [[nodiscard]] uint32_t debounceMs() const noexcept { return m_debounceMs; }
    [[nodiscard]] uint32_t longPressMs() const noexcept { return m_longPressMs; }
    [[nodiscard]] bool isPressed() const noexcept { return m_down; }
    [[nodiscard]] uint32_t pressedSinceMs() const noexcept { return m_downSinceMs; }

private:
    void latch(PressType type) noexcept;

    uint32_t m_debounceMs{50};
    uint32_t m_longPressMs{1000};

    bool m_down{false};
    bool m_longFired{false};
    uint32_t m_downSinceMs{0};

    uint32_t m_lastEdgeMs{0};
    bool m_hasLastEdge{false};

    /** @brief A release edge that arrived inside the debounce window. */
    bool m_pendingRelease{false};

    PressType m_pending{PressType::None};
};

}  // namespace Clock
