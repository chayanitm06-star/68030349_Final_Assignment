#include "UiStateMachine.hpp"

namespace Clock {

const char* toString(UiMode mode) noexcept {
    switch (mode) {
        case UiMode::MainClock:
            return "MAIN_CLOCK";
        case UiMode::AlarmSet:
            return "ALARM_SET";
        case UiMode::Stopwatch:
            return "STOPWATCH";
        case UiMode::ManualTime:
            return "MANUAL_TIME";
    }
    return "UNKNOWN";
}

UiMode UiStateMachine::nextMode() noexcept {
    m_mode = static_cast<UiMode>((static_cast<uint8_t>(m_mode) + 1u) % kModeCount);
    // Leaving the alarm editor always drops the edit cursor.
    if (m_mode != UiMode::AlarmSet) {
        m_field = EditField::None;
    } else if (m_field == EditField::None) {
        m_field = EditField::Hour;
    }
    return m_mode;
}

bool UiStateMachine::setMode(UiMode mode) noexcept {
    const auto raw = static_cast<uint8_t>(mode);
    if (raw >= kModeCount) {
        return false;
    }
    m_mode = mode;
    if (m_mode != UiMode::AlarmSet) {
        m_field = EditField::None;
    }
    return true;
}

EditField UiStateMachine::nextField() noexcept {
    switch (m_field) {
        case EditField::None:
            m_field = EditField::Hour;
            break;
        case EditField::Hour:
            m_field = EditField::Minute;
            break;
        case EditField::Minute:
            m_field = EditField::None;
            break;
    }
    return m_field;
}

void UiStateMachine::setDraft(uint8_t hour, uint8_t minute) noexcept {
    m_hour = static_cast<uint8_t>(hour % 24u);
    m_minute = static_cast<uint8_t>(minute % 60u);
}

bool UiStateMachine::increment(uint8_t step) noexcept {
    if (m_field == EditField::Hour) {
        m_hour = static_cast<uint8_t>((m_hour + step) % 24u);
        return true;
    }
    if (m_field == EditField::Minute) {
        m_minute = static_cast<uint8_t>((m_minute + step) % 60u);
        return true;
    }
    return false;
}

bool UiStateMachine::decrement(uint8_t step) noexcept {
    if (m_field == EditField::Hour) {
        m_hour = static_cast<uint8_t>((m_hour + 24u - (step % 24u)) % 24u);
        return true;
    }
    if (m_field == EditField::Minute) {
        m_minute = static_cast<uint8_t>((m_minute + 60u - (step % 60u)) % 60u);
        return true;
    }
    return false;
}

void UiStateMachine::reset() noexcept {
    m_mode = UiMode::MainClock;
    m_field = EditField::None;
    m_hour = 0;
    m_minute = 0;
}

}  // namespace Clock
