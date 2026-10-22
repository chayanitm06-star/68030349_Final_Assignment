/**
 * @file Button.hpp
 * @brief Single push button driver (GPIO configuration + interrupt service).
 *
 * This is the lowest layer of the input chain:
 *
 *   ButtonDriver (this file)   GPIO -> ISR -> raw event queue
 *          |
 *          v
 *   Clock::ButtonManager        raw queue -> debounce -> debounced event queue
 *          |
 *          v
 *   Clock::UiController         button events -> domain commands
 *
 * The interrupt handler runs in ISR context, therefore it only posts a tiny
 * @ref ButtonEvent_t into a FreeRTOS queue and never touches the display.
 */
#pragma once

#include <cstdint>

#include "ButtonEvent.hpp"
#include "Debounce.hpp"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace Clock {

/** @brief Physical buttons of the panel (dashboard compatible names). */
enum ButtonType_t : uint8_t {
    BUTTON_MODE = 0,  ///< MODE   - cycle through the display modes.
    BUTTON_INC = 1,   ///< INC    - increment / trigger an NTP synchronisation.
    BUTTON_SET = 2,   ///< SET    - commit the draft / start-pause the stopwatch.
    BUTTON_LAP = 3,   ///< LAP    - record a lap / reset / silence the alarm.
};

/** @brief Dashboard compatible button event. */
struct ButtonEvent_t {
    ButtonType_t type{BUTTON_MODE};
    PressType press{PressType::Short};
    bool pressed{false};        ///< True = press edge, false = release edge.
    uint32_t timestamp{0};      ///< Milliseconds since boot.
};

class ButtonDriver {
public:
    ButtonDriver(gpio_num_t pin, ButtonType_t type, QueueHandle_t queue);

    ButtonDriver(const ButtonDriver&) = delete;
    ButtonDriver& operator=(const ButtonDriver&) = delete;

    /** @brief Configure the GPIO and register the falling edge interrupt. */
    esp_err_t init();

    [[nodiscard]] gpio_num_t pin() const noexcept { return m_pin; }
    [[nodiscard]] ButtonType_t type() const noexcept { return m_type; }
    [[nodiscard]] QueueHandle_t queue() const noexcept { return m_queue; }
    [[nodiscard]] bool isInitialised() const noexcept { return m_initialised; }

    // --- Conversion helpers (also used by the unit tests) -------------------
    [[nodiscard]] static ButtonId toButtonId(ButtonType_t type) noexcept;
    [[nodiscard]] static ButtonType_t toButtonType(ButtonId id) noexcept;
    [[nodiscard]] static ButtonEvent_t toDashboardEvent(const ButtonEvent& event) noexcept;

    /** @brief GPIO interrupt handler (registered with @c gpio_isr_handler_add). */
    static void IRAM_ATTR isrTrampoline(void* arg);

private:
    gpio_num_t m_pin;
    ButtonType_t m_type;
    QueueHandle_t m_queue;
    bool m_initialised{false};
};

}  // namespace Clock
