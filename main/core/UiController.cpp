#include "UiController.hpp"

namespace Clock {

UiController::UiController(UiStateMachine& ui, AlarmEngine& alarm, StopwatchEngine& stopwatch, TimeManager& time)
    : m_ui(ui), m_alarm(alarm), m_stopwatch(stopwatch), m_time(time) {}

void UiController::handleEvent(const ButtonEvent& event) {
    ++m_handledEvents;
    switch (event.id) {
        case ButtonId::Mode:
            handleMode(event);
            break;
        case ButtonId::Inc:
            handleInc(event);
            break;
        case ButtonId::Set:
            handleSet(event);
            break;
        case ButtonId::Lap:
            handleLap(event);
            break;
    }
}

void UiController::handleMode(const ButtonEvent& event) {
    if (event.type == PressType::Long) {
        // Long press of MODE forces an NTP synchronisation.
        m_time.requestSync();
        return;
    }
    m_ui.nextMode();
    if (m_ui.mode() == UiMode::AlarmSet) {
        m_ui.setDraft(m_alarm.hour(), m_alarm.minute());
    }
}

void UiController::handleInc(const ButtonEvent& event) {
    switch (m_ui.mode()) {
        case UiMode::AlarmSet:
            m_ui.increment(event.type == PressType::Long ? 5u : 1u);
            break;
        case UiMode::ManualTime:
            m_ui.increment(event.type == PressType::Long ? 5u : 1u);
            break;
        case UiMode::MainClock:
            m_time.requestSync();
            break;
        case UiMode::Stopwatch:
            break;
    }
}

void UiController::handleSet(const ButtonEvent& event) {
    switch (m_ui.mode()) {
        case UiMode::AlarmSet: {
            if (event.type == PressType::Long) {
                // Long press moves the edit cursor between hour and minute.
                m_ui.nextField();
                return;
            }
            // Commit the draft and toggle the armed state.
            m_alarm.setAlarm(m_ui.draftHour(), m_ui.draftMinute(), !m_alarm.isEnabled());
            break;
        }
        case UiMode::ManualTime: {
            if (event.type == PressType::Long) {
                m_ui.nextField();
                return;
            }
            DateTime manual{};
            manual.year = m_time.now().year;
            manual.month = m_time.now().month;
            manual.day = m_time.now().day;
            manual.dayOfWeek = m_time.now().dayOfWeek;
            manual.hour = m_ui.draftHour();
            manual.minute = m_ui.draftMinute();
            manual.second = 0;
            m_time.applyManualTime(manual);
            break;
        }
        case UiMode::Stopwatch:
            m_stopwatch.toggle();
            break;
        case UiMode::MainClock:
            break;
    }
}

void UiController::handleLap(const ButtonEvent& event) {
    (void)event;

    if (m_ui.mode() == UiMode::Stopwatch) {
        if (m_stopwatch.isRunning()) {
            m_stopwatch.recordLap();
        } else {
            m_stopwatch.reset();
        }
        return;
    }
    // Everywhere else LAP silences a ringing alarm.
    m_alarm.stopRinging();
}

}  // namespace Clock
