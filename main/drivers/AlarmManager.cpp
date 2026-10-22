#include "AlarmManager.hpp"

#include "esp_log.h"

namespace Clock {
namespace {

constexpr const char* kTag = "ALARM";

}  // namespace

AlarmManager::AlarmManager(gpio_num_t buzzerPin, uint32_t ringingTimeoutMs)
    : m_buzzer(buzzerPin), m_engine(m_buzzer, ringingTimeoutMs) {}

esp_err_t AlarmManager::init() { return m_buzzer.init(); }

void AlarmManager::setAlarm(int hour, int minute, bool enable) {
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        ESP_LOGW(kTag, "invalid alarm time %02d:%02d - ignored", hour, minute);
        return;
    }

    m_engine.setAlarm(static_cast<uint8_t>(hour), static_cast<uint8_t>(minute), enable);
    ESP_LOGI(kTag, "alarm set to %02d:%02d [%s]", hour, minute, enable ? "ARMED" : "OFF");
}

void AlarmManager::checkAlarm(int currentHour, int currentMinute) {
    if (currentHour < 0 || currentHour > 23 || currentMinute < 0 || currentMinute > 59) {
        return;
    }

    if (m_engine.check(static_cast<uint8_t>(currentHour), static_cast<uint8_t>(currentMinute))) {
        ESP_LOGW(kTag, "alarm triggered at %02d:%02d - piezo active", currentHour, currentMinute);
    }
}

void AlarmManager::stopRinging() {
    if (m_engine.isRinging()) {
        m_engine.stopRinging();
        ESP_LOGI(kTag, "alarm silenced");
    }
}

void AlarmManager::update(uint32_t nowMs) { m_engine.update(nowMs); }

bool AlarmManager::isRinging() const { return m_engine.isRinging(); }

bool AlarmManager::isEnabled() const { return m_engine.isEnabled(); }

int AlarmManager::alarmHour() const { return static_cast<int>(m_engine.hour()); }

int AlarmManager::alarmMinute() const { return static_cast<int>(m_engine.minute()); }

uint32_t AlarmManager::triggerCount() const { return m_engine.triggerCount(); }

}  // namespace Clock
