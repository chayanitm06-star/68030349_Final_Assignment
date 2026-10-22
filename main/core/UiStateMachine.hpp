/**
 * @file UiStateMachine.hpp
 * @brief Finite state machine of the display modes and the edit cursor.
 */
#pragma once

#include <cstdint>

namespace Clock {

/** @brief The four operating modes of the digital clock. */
enum class UiMode : uint8_t {
    MainClock = 0,  ///< Large clock + date.
    AlarmSet = 1,   ///< Alarm hour/minute editor.
    Stopwatch = 2,  ///< Stopwatch with lap memory.
    ManualTime = 3, ///< Manual RTC calibration.
};

/** @brief Field currently selected by the edit cursor. */
enum class EditField : uint8_t {
    None = 0,
    Hour = 1,
    Minute = 2,
};

/** @brief Human readable name of a mode (used by the renderer and the log). */
[[nodiscard]] const char* toString(UiMode mode) noexcept;

/**
 * @brief Cyclic state machine: MAIN_CLOCK -> ALARM_SET -> STOPWATCH -> MANUAL_TIME.
 *
 * The machine also owns the *draft* alarm time that is being edited. The draft
 * is copied into the alarm engine when the user confirms with the SET button,
 * which keeps the editor free of side effects and easy to unit test.
 */
class UiStateMachine {
public:
    static constexpr uint8_t kModeCount = 4;

    [[nodiscard]] UiMode mode() const noexcept { return m_mode; }

    /** @brief Advance to the next mode (cyclic). */
    UiMode nextMode() noexcept;

    /** @brief Force a specific mode. */
    bool setMode(UiMode mode) noexcept;

    [[nodiscard]] EditField field() const noexcept { return m_field; }

    /** @brief Move the edit cursor (None -> Hour -> Minute -> None). */
    EditField nextField() noexcept;

    /** @brief Set the edit cursor explicitly. */
    void setField(EditField field) noexcept { m_field = field; }

    /** @brief Draft alarm hour (0..23). */
    [[nodiscard]] uint8_t draftHour() const noexcept { return m_hour; }

    /** @brief Draft alarm minute (0..59). */
    [[nodiscard]] uint8_t draftMinute() const noexcept { return m_minute; }

    /** @brief Load the draft from an existing alarm value. */
    void setDraft(uint8_t hour, uint8_t minute) noexcept;

    /** @brief Increment the edited field by @p step, wrapping around. */
    bool increment(uint8_t step = 1) noexcept;

    /** @brief Decrement the edited field by @p step, wrapping around. */
    bool decrement(uint8_t step = 1) noexcept;

    /** @brief Reset to the default state (MAIN_CLOCK, no edit cursor). */
    void reset() noexcept;

private:
    UiMode m_mode{UiMode::MainClock};
    EditField m_field{EditField::None};
    uint8_t m_hour{7};
    uint8_t m_minute{0};
};

}  // namespace Clock
