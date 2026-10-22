#include "Button.hpp"

#include <cstring>

#include "esp_log.h"
#include "esp_timer.h"

namespace Clock {
namespace {

constexpr const char* kTag = "Button";

}  // namespace

ButtonDriver::ButtonDriver(gpio_num_t pin, ButtonType_t type, QueueHandle_t queue)
    : m_pin(pin), m_type(type), m_queue(queue) {}

esp_err_t ButtonDriver::init() {
    gpio_config_t ioConfig{};
    ioConfig.pin_bit_mask = 1ULL << static_cast<uint64_t>(m_pin);
    ioConfig.mode = GPIO_MODE_INPUT;
    ioConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    ioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ioConfig.intr_type = GPIO_INTR_NEGEDGE;

    const esp_err_t err = gpio_config(&ioConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "gpio_config failed on pin %d: %s", static_cast<int>(m_pin), esp_err_to_name(err));
        return err;
    }

    if (m_queue == nullptr) {
        ESP_LOGE(kTag, "button %d has no event queue", static_cast<int>(m_type));
        return ESP_ERR_INVALID_ARG;
    }

    m_initialised = true;
    ESP_LOGI(kTag, "button %d configured on GPIO %d (active low)", static_cast<int>(m_type),
             static_cast<int>(m_pin));
    return ESP_OK;
}

void ButtonDriver::isrTrampoline(void* arg) {
    auto* self = static_cast<ButtonDriver*>(arg);
    if (self == nullptr || self->m_queue == nullptr) {
        return;
    }

    ButtonEvent_t event{};
    event.type = self->m_type;
    event.pressed = (gpio_get_level(self->m_pin) == 0);  // active low
    event.press = PressType::None;
    event.timestamp = static_cast<uint32_t>(esp_timer_get_time() / 1000);

    // Only a queue post happens in ISR context - no blocking, no logging.
    xQueueSendFromISR(self->m_queue, &event, nullptr);
}

ButtonId ButtonDriver::toButtonId(ButtonType_t type) noexcept {
    switch (type) {
        case BUTTON_INC:
            return ButtonId::Inc;
        case BUTTON_SET:
            return ButtonId::Set;
        case BUTTON_LAP:
            return ButtonId::Lap;
        case BUTTON_MODE:
        default:
            return ButtonId::Mode;
    }
}

ButtonType_t ButtonDriver::toButtonType(ButtonId id) noexcept {
    switch (id) {
        case ButtonId::Inc:
            return BUTTON_INC;
        case ButtonId::Set:
            return BUTTON_SET;
        case ButtonId::Lap:
            return BUTTON_LAP;
        case ButtonId::Mode:
        default:
            return BUTTON_MODE;
    }
}

ButtonEvent_t ButtonDriver::toDashboardEvent(const ButtonEvent& event) noexcept {
    ButtonEvent_t out{};
    out.type = toButtonType(event.id);
    out.press = event.type;
    out.pressed = (event.type != PressType::None);
    out.timestamp = event.timestampMs;
    return out;
}

}  // namespace Clock
