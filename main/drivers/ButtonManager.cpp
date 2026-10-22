#include "ButtonManager.hpp"

#include <cinttypes>

#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/task.h"

namespace Clock {
namespace {

constexpr const char* kTag = "Button";

uint32_t nowMs() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

}  // namespace

ButtonManager::ButtonManager() {
    // The panel layout is defined in ClockConfig.hpp (single source of truth).
    for (uint8_t i = 0; i < kButtonCount; ++i) {
        m_slots[i].id = Config::kButtonIds[i];
        m_slots[i].filter.configure(Config::kButtonDebounceMs, Config::kLongPressMs);
    }
}

ButtonManager::~ButtonManager() {
    for (auto*& driver : m_drivers) {
        delete driver;
        driver = nullptr;
    }

    if (m_rawQueue != nullptr) {
        vQueueDelete(m_rawQueue);
        m_rawQueue = nullptr;
    }
    if (m_eventQueue != nullptr) {
        vQueueDelete(m_eventQueue);
        m_eventQueue = nullptr;
    }
}

esp_err_t ButtonManager::init() {
    m_rawQueue = xQueueCreate(Config::kButtonQueueLength * 2, sizeof(ButtonEvent_t));
    m_eventQueue = xQueueCreate(Config::kButtonQueueLength, sizeof(ButtonEvent));
    if (m_rawQueue == nullptr || m_eventQueue == nullptr) {
        ESP_LOGE(kTag, "cannot create the button queues");
        return ESP_ERR_NO_MEM;
    }

    if (!m_isrServiceInstalled) {
        const esp_err_t err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(kTag, "gpio_install_isr_service failed: %s", esp_err_to_name(err));
            return err;
        }
        m_isrServiceInstalled = true;
    }

    for (uint8_t i = 0; i < kButtonCount; ++i) {
        m_drivers[i] = new ButtonDriver(Config::kButtonPins[i],
                                        ButtonDriver::toButtonType(Config::kButtonIds[i]), m_rawQueue);
        if (m_drivers[i] == nullptr) {
            ESP_LOGE(kTag, "out of memory while creating the button driver");
            return ESP_ERR_NO_MEM;
        }

        const esp_err_t err = m_drivers[i]->init();
        if (err != ESP_OK) {
            return err;
        }

        const esp_err_t isrErr =
            gpio_isr_handler_add(Config::kButtonPins[i], &ButtonDriver::isrTrampoline, m_drivers[i]);
        if (isrErr != ESP_OK) {
            ESP_LOGE(kTag, "gpio_isr_handler_add failed: %s", esp_err_to_name(isrErr));
            return isrErr;
        }
    }

    ESP_LOGI(kTag, "%u buttons ready (debounce %" PRIu32 " ms, long press %" PRIu32 " ms)", kButtonCount,
             Config::kButtonDebounceMs, Config::kLongPressMs);
    return ESP_OK;
}

bool ButtonManager::poll(ButtonEvent& out, uint32_t timeoutMs) {
    ButtonEvent_t raw{};

    // 1) Consume the raw edges produced by the interrupt handlers.
    while (xQueueReceive(m_rawQueue, &raw, 0) == pdTRUE) {
        const ButtonId id = ButtonDriver::toButtonId(raw.type);
        uint8_t slot = 0;
        for (; slot < kButtonCount; ++slot) {
            if (m_slots[slot].id == id) {
                break;
            }
        }
        if (slot >= kButtonCount) {
            continue;
        }

        ++m_edgeCount;

        if (raw.pressed) {
            m_slots[slot].filter.notifyPressEdge(raw.timestamp);
        } else {
            m_slots[slot].filter.notifyReleaseEdge(raw.timestamp);
        }

        const PressType type = m_slots[slot].filter.takeEvent();
        if (type != PressType::None) {
            const ButtonEvent event{id, type, raw.timestamp};
            xQueueSend(m_eventQueue, &event, 0);
        }
    }

    // 2) Long press detection needs a periodic evaluation.
    const uint32_t now = nowMs();
    for (uint8_t i = 0; i < kButtonCount; ++i) {
        const PressType type = m_slots[i].filter.update(now);
        if (type != PressType::None) {
            const ButtonEvent event{m_slots[i].id, type, now};
            xQueueSend(m_eventQueue, &event, 0);
        }
    }

    // 3) Wait for the next debounced event.
    return xQueueReceive(m_eventQueue, &out, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

}  // namespace Clock
