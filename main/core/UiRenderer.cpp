#include "UiRenderer.hpp"

#include <cstdio>

namespace Clock {

UiRenderer::UiRenderer(IDisplay& display, TimeManager& time, AlarmEngine& alarm, StopwatchEngine& stopwatch,
                       UiStateMachine& ui)
    : m_display(display), m_time(time), m_alarm(alarm), m_stopwatch(stopwatch), m_ui(ui) {}

void UiRenderer::render() {
    m_display.clear();
    renderHeader();

    switch (m_ui.mode()) {
        case UiMode::MainClock:
            renderClock();
            break;
        case UiMode::AlarmSet:
            renderAlarmSet();
            break;
        case UiMode::Stopwatch:
            renderStopwatch();
            break;
        case UiMode::ManualTime:
            renderManualTime();
            break;
    }

    m_display.flush();
}

void UiRenderer::renderHeader() {
    m_display.drawText(0, 0, toString(m_ui.mode()), 1);
    m_display.drawTextRight(0, m_time.isNtpSynced() ? "NTP" : "RTC", 1);
    m_display.drawHLine(9);
}

void UiRenderer::renderClock() {
    char line[24]{};

    const DateTime now = m_time.now();
    now.formatTime(line, sizeof(line));
    m_display.drawText(16, 14, line, 2);

    char dateLine[24]{};
    std::snprintf(dateLine, sizeof(dateLine), "%s", now.weekdayName());
    m_display.drawText(0, 34, dateLine, 1);

    char calendar[16]{};
    now.formatDate(calendar, sizeof(calendar));
    m_display.drawText(0, 44, calendar, 1);

    char alarmLine[24]{};
    std::snprintf(alarmLine, sizeof(alarmLine), "ALARM %02u:%02u %s", static_cast<unsigned>(m_alarm.hour()),
                  static_cast<unsigned>(m_alarm.minute()), m_alarm.isEnabled() ? "ON" : "OFF");
    m_display.drawText(0, 54, alarmLine, 1);
}

void UiRenderer::renderAlarmSet() {
    char line[24]{};

    std::snprintf(line, sizeof(line), "%02u:%02u", static_cast<unsigned>(m_ui.draftHour()),
                  static_cast<unsigned>(m_ui.draftMinute()));
    m_display.drawText(34, 16, line, 2);

    m_display.drawText(0, 36, m_alarm.isEnabled() ? "STATE: ARMED" : "STATE: OFF", 1);
    m_display.drawText(0, 46, m_ui.field() == EditField::Hour ? "SEL: HOUR" : "SEL: MIN", 1);
    m_display.drawText(0, 56, "INC:+1 LONG:+5 SET:OK", 1);
}

void UiRenderer::renderStopwatch() {
    char line[24]{};

    formatStopwatch(m_stopwatch.elapsedMs(), line, sizeof(line));
    m_display.drawText(16, 14, line, 2);

    char laps[24]{};
    std::snprintf(laps, sizeof(laps), "LAPS: %u", static_cast<unsigned>(m_stopwatch.lapCount()));
    m_display.drawText(0, 36, laps, 1);

    if (m_stopwatch.lapCount() > 0) {
        char last[24]{};
        formatStopwatch(m_stopwatch.lapDeltaMs(m_stopwatch.lapCount() - 1), last, sizeof(last));
        char lastLine[32]{};
        std::snprintf(lastLine, sizeof(lastLine), "LAST %s", last);
        m_display.drawText(0, 46, lastLine, 1);
    }

    m_display.drawText(0, 56, m_stopwatch.isRunning() ? "RUNNING - LAP:REC" : "STOPPED - LAP:RST", 1);
}

void UiRenderer::renderManualTime() {
    char line[24]{};

    std::snprintf(line, sizeof(line), "%02u:%02u", static_cast<unsigned>(m_ui.draftHour()),
                  static_cast<unsigned>(m_ui.draftMinute()));
    m_display.drawText(34, 18, line, 2);

    m_display.drawText(0, 40, "MANUAL RTC SET", 1);
    m_display.drawText(0, 52, "INC:+1 SET:APPLY", 1);
}

void UiRenderer::formatStopwatch(uint64_t milliseconds, char* buffer, size_t bufferSize) {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    const uint64_t minutes = milliseconds / 60000ULL;
    const uint64_t seconds = (milliseconds % 60000ULL) / 1000ULL;
    const uint64_t hundredths = (milliseconds % 1000ULL) / 10ULL;
    std::snprintf(buffer, bufferSize, "%02llu:%02llu.%02llu", static_cast<unsigned long long>(minutes),
                  static_cast<unsigned long long>(seconds), static_cast<unsigned long long>(hundredths));
}

}  // namespace Clock
