#include "Buzzer.hpp"

#include "esp_log.h"

namespace Clock {
namespace {
constexpr const char* kTag = "Buzzer";
}  // namespace

Buzzer::Buzzer(gpio_num_t pin, ledc_channel_t channel, ledc_timer_t timer)
    : m_pin(pin), m_channel(channel), m_timer(timer) {}

esp_err_t Buzzer::init() {
    ledc_timer_config_t timerConfig{};
    timerConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig.duty_resolution = static_cast<ledc_timer_bit_t>(kDutyResolutionBits);
    timerConfig.timer_num = m_timer;
    timerConfig.freq_hz = 2700;
    timerConfig.clk_cfg = LEDC_AUTO_CLK;

    esp_err_t err = ledc_timer_config(&timerConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "ledc_timer_config failed: %s", esp_err_to_name(err));
        return err;
    }

    ledc_channel_config_t channelConfig{};
    channelConfig.gpio_num = m_pin;
    channelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig.channel = m_channel;
    channelConfig.timer_sel = m_timer;
    channelConfig.duty = 0;
    channelConfig.hpoint = 0;

    err = ledc_channel_config(&channelConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "ledc_channel_config failed: %s", esp_err_to_name(err));
        return err;
    }

    m_initialised = true;
    ESP_LOGI(kTag, "piezo buzzer ready on GPIO %d", static_cast<int>(m_pin));
    return ESP_OK;
}

void Buzzer::setTone(uint16_t frequencyHz, uint8_t dutyPercent) {
    if (!m_initialised || frequencyHz == 0) {
        return;
    }

    // The LEDC timer frequency is fixed at boot, therefore only the duty cycle
    // is modulated while the tone is active.
    const uint32_t maxDuty = (1u << kDutyResolutionBits) - 1u;
    const uint32_t duty = (maxDuty * static_cast<uint32_t>(dutyPercent)) / 100u;

    ledc_set_duty(LEDC_LOW_SPEED_MODE, m_channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, m_channel);
    m_active = true;
}

void Buzzer::stop() {
    if (!m_initialised) {
        return;
    }
    ledc_set_duty(LEDC_LOW_SPEED_MODE, m_channel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, m_channel);
    m_active = false;
}

}  // namespace Clock
