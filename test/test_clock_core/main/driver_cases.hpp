/**
 * @file driver_cases.hpp
 * @brief Test cases that need the real ESP-IDF drivers (target only).
 *
 * These cases exercise the FreeRTOS primitives and the hardware abstraction
 * layer, therefore they can only be executed on the ESP32 (or in QEMU):
 *
 *   test_i2c_bus_mutex_lock          test_ssd1306_buffer_bounds
 *   test_freertos_event_group_sync   FreeRTOS_Button_Queue_Post_Receive
 *   I2C_Mutex_Thread_Safety_Lock     WiFi_Station_Connect_SSID_iotexpo
 */
#pragma once

#include <cstring>

#include "Button.hpp"
#include "I2CBus.hpp"
#include "NTPManager.hpp"
#include "SSD1306.hpp"

#include "esp_err.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace {

/** @brief State that is shared between the test task and the probe task. */
struct MutexProbe {
    Clock::I2CBus* bus{nullptr};
    volatile bool acquired{false};
    volatile bool done{false};
};

/** @brief Second task that tries to grab the I2C bus while it is locked. */
void mutexProbeTask(void* arg) {
    auto* probe = static_cast<MutexProbe*>(arg);
    if (probe == nullptr || probe->bus == nullptr) {
        vTaskDelete(nullptr);
        return;
    }

    probe->acquired = probe->bus->lock(50);
    probe->done = true;
    vTaskDelete(nullptr);
}

}  // namespace

// ---------------------------------------------------------------------------
// Spiral 1 - hardware layer & bus communication
// ---------------------------------------------------------------------------

/** @brief The I2C bus mutex serialises the concurrent transfers. */
inline void test_i2c_bus_mutex_lock() {
    Clock::I2CBus bus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, 400000);

    // First acquisition succeeds.
    TEST_ASSERT_TRUE(bus.lock(100));

    // The mutex is not recursive - a second attempt must fail immediately.
    TEST_ASSERT_FALSE(bus.lock(0));

    bus.unlock();

    // After the release the bus can be locked again.
    TEST_ASSERT_TRUE(bus.lock(100));
    bus.unlock();
}

/** @brief The SSD1306 frame buffer rejects every out of range pixel. */
inline void test_ssd1306_buffer_bounds() {
    Clock::I2CBus bus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, 400000);
    Clock::SSD1306 display(bus, 0x3C);

    display.clear();
    const uint8_t* buffer = display.buffer();
    TEST_ASSERT_NOT_NULL(buffer);
    TEST_ASSERT_EQUAL_UINT16(1024, Clock::SSD1306::kBufferSize);

    // Out of range pixels must be ignored.
    display.setPixel(-1, 0, true);
    display.setPixel(0, -1, true);
    display.setPixel(128, 0, true);
    display.setPixel(0, 64, true);
    display.setPixel(1000, 1000, true);

    for (uint16_t i = 0; i < Clock::SSD1306::kBufferSize; ++i) {
        TEST_ASSERT_EQUAL_UINT8(0, buffer[i]);
    }

    // The lower right pixel is the last byte of the frame buffer.
    display.setPixel(127, 63, true);
    TEST_ASSERT_EQUAL_UINT8(0x80, buffer[Clock::SSD1306::kBufferSize - 1]);

    // The upper left pixel is the first byte of the frame buffer.
    display.setPixel(0, 0, true);
    TEST_ASSERT_EQUAL_UINT8(0x01, buffer[0]);

    // Writing text must not corrupt the memory outside of the frame buffer.
    display.clear();
    display.drawText(0, 0, "ESP32", 1);
    display.drawTextRight(0, "RTC", 1);
    display.drawHLine(9);
    TEST_ASSERT_EQUAL_UINT16(1024, Clock::SSD1306::kBufferSize);
}

// ---------------------------------------------------------------------------
// Spiral 5 - FreeRTOS integration
// ---------------------------------------------------------------------------

/** @brief The inter-task event group transports the state flags. */
inline void test_freertos_event_group_sync() {
    EventGroupHandle_t group = xEventGroupCreate();
    TEST_ASSERT_NOT_NULL(group);

    // No bit is set yet.
    TEST_ASSERT_EQUAL_UINT32(0, xEventGroupGetBits(group));

    xEventGroupSetBits(group, BIT0 | BIT3);
    const EventBits_t bits = xEventGroupWaitBits(group, BIT0 | BIT3, pdTRUE, pdTRUE, pdMS_TO_TICKS(100));
    TEST_ASSERT_TRUE((bits & BIT0) != 0);
    TEST_ASSERT_TRUE((bits & BIT3) != 0);
    TEST_ASSERT_TRUE((bits & BIT1) == 0);

    // The bits have been consumed (pdTRUE clears them).
    TEST_ASSERT_EQUAL_UINT32(0, xEventGroupGetBits(group));

    // A timeout must be reported when the bit never arrives.
    const EventBits_t timeout = xEventGroupWaitBits(group, BIT5, pdFALSE, pdTRUE, pdMS_TO_TICKS(20));
    TEST_ASSERT_TRUE((timeout & BIT5) == 0);

    vEventGroupDelete(group);
}

/** @brief A button event travels from the ISR queue to the task. */
inline void FreeRTOS_Button_Queue_Post_Receive() {
    QueueHandle_t queue = xQueueCreate(4, sizeof(ButtonEvent_t));
    TEST_ASSERT_NOT_NULL(queue);

    Clock::ButtonEvent_t posted{};
    posted.type = BUTTON_INC;
    posted.pressed = true;
    posted.timestamp = 1234;

    TEST_ASSERT_EQUAL_INT(pdTRUE, xQueueSend(queue, &posted, 0));

    Clock::ButtonEvent_t received{};
    TEST_ASSERT_EQUAL_INT(pdTRUE, xQueueReceive(queue, &received, pdMS_TO_TICKS(100)));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(BUTTON_INC), static_cast<uint8_t>(received.type));
    TEST_ASSERT_TRUE(received.pressed);
    TEST_ASSERT_EQUAL_UINT32(1234, received.timestamp);

    // The queue is empty again.
    TEST_ASSERT_EQUAL_INT(pdFALSE, xQueueReceive(queue, &received, pdMS_TO_TICKS(20)));

    vQueueDelete(queue);
}

/** @brief A second task must not be able to use the bus while it is locked. */
inline void I2C_Mutex_Thread_Safety_Lock() {
    Clock::I2CBus bus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22, 400000);

    MutexProbe probe;
    probe.bus = &bus;

    // The "display task" holds the bus.
    TEST_ASSERT_TRUE(bus.lock(100));

    TaskHandle_t task = nullptr;
    TEST_ASSERT_EQUAL_INT(pdPASS,
                           xTaskCreate(&mutexProbeTask, "mutexProbe", 2048, &probe, 5, &task));

    // Give the probe enough time to run into its timeout.
    vTaskDelay(pdMS_TO_TICKS(300));

    TEST_ASSERT_TRUE(probe.done);
    TEST_ASSERT_FALSE(probe.acquired);

    // After the release the other task gets the bus.
    bus.unlock();
    vTaskDelay(pdMS_TO_TICKS(50));
    TEST_ASSERT_TRUE(bus.lock(100));
    bus.unlock();
}

/** @brief The station joins the access point "iotexpo". */
inline void WiFi_Station_Connect_SSID_iotexpo() {
    Clock::NTPManager::Settings settings;
    settings.ssid = "iotexpo";
    settings.password = "12345678";
    settings.primaryServer = "pool.ntp.org";
    settings.secondaryServer = "time.nist.gov";
    settings.connectTimeoutMs = 15000;
    settings.syncTimeoutMs = 10000;

    Clock::NTPManager manager(settings);
    const esp_err_t err = manager.init();
    TEST_ASSERT_EQUAL_INT(ESP_OK, err);
    TEST_ASSERT_TRUE(manager.isInitialised());

    // The credentials really reached the Wi-Fi driver.
    wifi_config_t config{};
    TEST_ASSERT_EQUAL_INT(ESP_OK, esp_wifi_get_config(WIFI_IF_STA, &config));
    TEST_ASSERT_EQUAL_STRING("iotexpo", reinterpret_cast<const char*>(config.sta.ssid));
    TEST_ASSERT_EQUAL_STRING("12345678", reinterpret_cast<const char*>(config.sta.password));

    // The station is not connected yet, but the driver is up and running.
    TEST_ASSERT_FALSE(manager.isWifiConnected());
    TEST_ASSERT_FALSE(manager.isSynced());
}
