/**
 * @file main.cpp
 * @brief Application entry point of the ESP32 modular digital clock.
 *
 * The file wires the drivers of @c main/drivers with the domain services of
 * @c main/core and creates the four FreeRTOS tasks that are described in the
 * architecture documentation:
 *
 *   | Task                | Prio | Stack  | Core | Trigger           |
 *   |---------------------|------|--------|------|-------------------|
 *   | vTaskClockDisplay   |  3   | 4096 B |  1   | periodic 100 ms   |
 *   | vTaskButtonHandler  |  5   | 3072 B |  0   | ISR queue event   |
 *   | vTaskAlarmCheck     |  4   | 2048 B |  1   | periodic 1000 ms  |
 *   | vTaskNTPSync        |  1   | 6144 B |  0   | periodic 3600 s   |
 */
#include <cstdint>
#include <cstdio>

#include "AlarmManager.hpp"
#include "ButtonManager.hpp"
#include "ClockConfig.hpp"
#include "DS1307.hpp"
#include "EspTimerSource.hpp"
#include "I2CBus.hpp"
#include "NTPManager.hpp"
#include "SSD1306.hpp"
#include "Stopwatch.hpp"
#include "SystemClock.hpp"
#include "TimeManager.hpp"
#include "UiController.hpp"
#include "UiRenderer.hpp"
#include "UiStateMachine.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

namespace {

constexpr const char* kTag = "ClockApp";

using namespace Clock;

// ---------------------------------------------------------------------------
// Inter-task event group bits
// ---------------------------------------------------------------------------
constexpr EventBits_t kEventRtcReady = BIT0;
constexpr EventBits_t kEventDisplayReady = BIT1;
constexpr EventBits_t kEventWifiConnected = BIT2;
constexpr EventBits_t kEventNtpSynced = BIT3;
constexpr EventBits_t kEventAlarmTriggered = BIT4;

// ---------------------------------------------------------------------------
// Hardware layer (HAL)
// ---------------------------------------------------------------------------
I2CBus g_bus(Config::kI2cPort, Config::kI2cSdaPin, Config::kI2cSclPin, Config::kI2cClockHz);
DS1307 g_rtc(g_bus, Config::kDs1307Address);
SSD1306 g_oled(g_bus, Config::kSsd1306Address);
ButtonManager g_buttons;
EspTimerSource g_timerSource;
SystemClock g_systemClock;

// ---------------------------------------------------------------------------
// Network layer
// ---------------------------------------------------------------------------
NTPManager g_ntp({Config::kWifiSsid,
                   Config::kWifiPassword,
                   Config::kWifiFallbackSsid,
                   Config::kWifiFallbackPassword,
                   Config::kNtpServer,
                   Config::kNtpServerFallback,
                   Config::kWifiConnectTimeoutMs,
                   10000});

// ---------------------------------------------------------------------------
// Domain layer (pure C++, unit tested on the host)
// ---------------------------------------------------------------------------
TimeManager g_timeManager(g_rtc, g_systemClock, g_ntp);
AlarmManager g_alarm(Config::kBuzzerPin, Config::kAlarmRingingTimeoutMs);
Stopwatch g_stopwatch(g_timerSource);
UiStateMachine g_ui;
UiController g_controller(g_ui, g_alarm.engine(), g_stopwatch.engine(), g_timeManager);
UiRenderer g_renderer(g_oled, g_timeManager, g_alarm.engine(), g_stopwatch.engine(), g_ui);

EventGroupHandle_t g_events = nullptr;

/** @brief Small helper that keeps the log output readable. */
void logTask(const char* name, UBaseType_t priority, uint32_t stackBytes, BaseType_t core) {
    // Every argument is cast to a plain int/unsigned so that the printf style
    // format string can never disagree with the type of the argument.
    ESP_LOGI(kTag, "%-18s prio=%u stack=%uB core=%d", name, static_cast<unsigned>(priority),
             static_cast<unsigned>(stackBytes), static_cast<int>(core));
}

// ---------------------------------------------------------------------------
// Task 1 - render the active mode onto the OLED (10 FPS)
// ---------------------------------------------------------------------------
void vTaskClockDisplay(void* /*arg*/) {
    // Wait until the RTC answered at least once.
    xEventGroupWaitBits(g_events, kEventRtcReady, pdFALSE, pdTRUE, portMAX_DELAY);

    while (true) {
        g_renderer.render();
        xEventGroupSetBits(g_events, kEventDisplayReady);
        vTaskDelay(pdMS_TO_TICKS(Config::kDisplayPeriodMs));
    }
}

// ---------------------------------------------------------------------------
// Task 2 - consume the debounced button events (highest priority)
// ---------------------------------------------------------------------------
void vTaskButtonHandler(void* /*arg*/) {
    ButtonEvent event{};

    while (true) {
        // poll() drains the raw ISR queue, runs the debounce filter and
        // returns the next debounced event.
        if (g_buttons.poll(event, ButtonManager::kScanPeriodMs * 4)) {
            ESP_LOGD(kTag, "button %d %s", static_cast<int>(event.id),
                     event.type == PressType::Long ? "long" : "short");
            g_controller.handleEvent(event);
        }
    }
}

// ---------------------------------------------------------------------------
// Task 3 - compare the wall clock against the alarm (1 Hz)
// ---------------------------------------------------------------------------
void vTaskAlarmCheck(void* /*arg*/) {
    while (true) {
        const DateTime now = g_timeManager.now();

        g_alarm.checkAlarm(now.hour, now.minute);
        g_alarm.update(static_cast<uint32_t>(g_timerSource.nowMs()));

        if (g_alarm.isRinging()) {
            xEventGroupSetBits(g_events, kEventAlarmTriggered);
        }

        vTaskDelay(pdMS_TO_TICKS(Config::kAlarmPeriodMs));
    }
}

// ---------------------------------------------------------------------------
// Task 4 - keep the RTC calibrated through the network (every hour)
// ---------------------------------------------------------------------------
void vTaskNTPSync(void* /*arg*/) {
    bool firstRun = true;

    while (true) {
        if (g_ntp.init() == ESP_OK) {
            if (g_ntp.connect()) {
                xEventGroupSetBits(g_events, kEventWifiConnected);

                if (g_ntp.syncTime()) {
                    xEventGroupSetBits(g_events, kEventNtpSynced);

                    // requestSync() pushes the network time into the DS1307.
                    if (g_timeManager.requestSync()) {
                        ESP_LOGI(kTag, "DS1307 calibrated from the network");
                    }
                }
            }
        }

        if (firstRun) {
            firstRun = false;
            // The first synchronisation attempt happens right after boot, the
            // following ones follow the configured period.
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(Config::kNtpPeriodSeconds * 1000));
    }
}

}  // namespace

extern "C" void app_main() {
    ESP_LOGI(kTag, "ESP32 modular digital clock - ESP-IDF v6 / FreeRTOS");

    g_events = xEventGroupCreate();
    if (g_events == nullptr) {
        ESP_LOGE(kTag, "cannot create the event group");
        return;
    }

    // ---- Hardware abstraction layer -------------------------------------
    if (g_bus.init() != ESP_OK) {
        ESP_LOGE(kTag, "I2C initialisation failed - halting");
        return;
    }
    // "Config::kI2cPort" and the pin numbers are enums / driver specific
    // types - they are cast to int explicitly so that the log call never
    // depends on an implicit type conversion.
    ESP_LOGI(kTag, "I2C bus ready (port=%d, SDA=%d, SCL=%d, %u Hz)",
             static_cast<int>(Config::kI2cPort), static_cast<int>(Config::kI2cSdaPin),
             static_cast<int>(Config::kI2cSclPin), static_cast<unsigned>(Config::kI2cClockHz));

    if (g_rtc.init() == ESP_OK) {
        xEventGroupSetBits(g_events, kEventRtcReady);
        DateTime now{};
        if (g_rtc.readDateTime(now)) {
            char text[24]{};
            now.format(text, sizeof(text));
            ESP_LOGI(kTag, "DS1307 ready, RTC time = %s", text);
        }
    } else {
        ESP_LOGW(kTag, "DS1307 not detected - the clock will run without the RTC");
        xEventGroupSetBits(g_events, kEventRtcReady);
    }

    if (g_oled.init() == ESP_OK) {
        ESP_LOGI(kTag, "SSD1306 ready at 0x%02X", static_cast<unsigned>(Config::kSsd1306Address));
    } else {
        ESP_LOGE(kTag, "SSD1306 not detected");
    }

    if (g_alarm.init() != ESP_OK) {
        ESP_LOGW(kTag, "the piezo buzzer could not be configured");
    }

    if (g_buttons.init() != ESP_OK) {
        ESP_LOGE(kTag, "the button panel could not be configured");
        return;
    }

    // The clock runs in UTC+7 (ICT) - the offset is applied by TimeManager.
    g_timeManager.setTimeZoneOffset(Config::kTimeZoneOffsetSeconds);

    // Default alarm: 07:00, disarmed.
    g_alarm.setAlarm(7, 0, false);
    g_ui.setDraft(7, 0);

    // ---- FreeRTOS tasks --------------------------------------------------
    BaseType_t ok = xTaskCreatePinnedToCore(&vTaskClockDisplay, "vTaskClockDisplay",
                                            Config::kDisplayTaskStackBytes, nullptr,
                                            Config::kDisplayTaskPriority, nullptr, Config::kDisplayTaskCore);
    if (ok != pdPASS) {
        ESP_LOGE(kTag, "cannot create vTaskClockDisplay");
    }

    ok = xTaskCreatePinnedToCore(&vTaskButtonHandler, "vTaskButtonHandler", Config::kButtonTaskStackBytes,
                                 nullptr, Config::kButtonTaskPriority, nullptr, Config::kButtonTaskCore);
    if (ok != pdPASS) {
        ESP_LOGE(kTag, "cannot create vTaskButtonHandler");
    }

    ok = xTaskCreatePinnedToCore(&vTaskAlarmCheck, "vTaskAlarmCheck", Config::kAlarmTaskStackBytes, nullptr,
                                 Config::kAlarmTaskPriority, nullptr, Config::kAlarmTaskCore);
    if (ok != pdPASS) {
        ESP_LOGE(kTag, "cannot create vTaskAlarmCheck");
    }

    ok = xTaskCreatePinnedToCore(&vTaskNTPSync, "vTaskNTPSync", Config::kNtpTaskStackBytes, nullptr,
                                 Config::kNtpTaskPriority, nullptr, Config::kNtpTaskCore);
    if (ok != pdPASS) {
        ESP_LOGE(kTag, "cannot create vTaskNTPSync");
    }

    logTask("vTaskClockDisplay", Config::kDisplayTaskPriority, Config::kDisplayTaskStackBytes,
            Config::kDisplayTaskCore);
    logTask("vTaskButtonHandler", Config::kButtonTaskPriority, Config::kButtonTaskStackBytes,
            Config::kButtonTaskCore);
    logTask("vTaskAlarmCheck", Config::kAlarmTaskPriority, Config::kAlarmTaskStackBytes,
            Config::kAlarmTaskCore);
    logTask("vTaskNTPSync", Config::kNtpTaskPriority, Config::kNtpTaskStackBytes, Config::kNtpTaskCore);

    ESP_LOGI(kTag, "all tasks started, free heap = %u bytes",
             static_cast<unsigned>(g_bus.freeHeapBytes()));
}
