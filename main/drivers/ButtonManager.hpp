/**
 * @file ButtonManager.hpp
 * @brief Debouncing engine for the four panel buttons.
 *
 * Architecture (bottom-up):
 *   ButtonDriver (GPIO ISR)  ->  raw event queue  ->  vTaskButtonHandler
 *                            ->  DebounceFilter    ->  event queue
 *                            ->  vTaskButtonHandler ->  UiController
 */
#pragma once

#include <cstdint>

#include "Button.hpp"
#include "ButtonEvent.hpp"
#include "ClockConfig.hpp"
#include "Debounce.hpp"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace Clock {

class ButtonManager {
public:
    /** @brief Number of buttons on the panel. */
    static constexpr uint8_t kButtonCount = Clock::kButtonCount;

    /** @brief Scan period of the button task in milliseconds. */
    static constexpr uint32_t kScanPeriodMs = 5;

    ButtonManager();
    ~ButtonManager();

    ButtonManager(const ButtonManager&) = delete;
    ButtonManager& operator=(const ButtonManager&) = delete;

    /** @brief Configure the GPIOs and install the interrupt service. */
    esp_err_t init();

    /** @brief Queue that receives the debounced @ref ButtonEvent messages. */
    [[nodiscard]] QueueHandle_t eventQueue() const noexcept { return m_eventQueue; }

    /** @brief Queue that receives the raw interrupt events. */
    [[nodiscard]] QueueHandle_t rawQueue() const noexcept { return m_rawQueue; }

    /**
     * @brief One iteration of the button engine.
     * @details Drains the raw interrupt queue, updates the debounce filters
     *          (including the long press detection) and waits for the next
     *          debounced event.
     * @param[out] out The debounced event.
     * @param timeoutMs Maximum time to wait for a debounced event.
     * @return True when an event was written to @p out.
     */
    bool poll(ButtonEvent& out, uint32_t timeoutMs = 20);

    /** @brief Number of raw edges seen since boot (diagnostics). */
    [[nodiscard]] uint32_t edgeCount() const noexcept { return m_edgeCount; }

private:
    struct Slot {
        ButtonId id{ButtonId::Mode};
        DebounceFilter filter;
        bool lastLevel{true};  ///< Last sampled level: true = released (pull-up).
    };

    /**
     * @brief Sample the level of every button and turn a level change into
     *        an edge for the debounce filter.
     * @details The interrupt is the fast path, the level is the safety net:
     *          if an edge is lost (simulator, noisy key, disabled interrupt)
     *          the button still reacts because the level is polled as well.
     */
    void sampleLevels();

    Slot m_slots[kButtonCount]{};
    ButtonDriver* m_drivers[kButtonCount]{};

    QueueHandle_t m_rawQueue{nullptr};
    QueueHandle_t m_eventQueue{nullptr};

    volatile uint32_t m_edgeCount{0};
    bool m_isrServiceInstalled{false};
};

}  // namespace Clock
