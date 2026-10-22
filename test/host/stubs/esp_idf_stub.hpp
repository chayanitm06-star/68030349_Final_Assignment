/**
 * @file esp_idf_stub.hpp
 * @brief Minimal stand-in for the ESP-IDF v6 API (development only!).
 * @details This header is NOT part of the firmware. It exists so that the
 *          driver layer and main.cpp can be type checked with a plain host
 *          compiler when no ESP-IDF installation is available.
 */
#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
// esp_err.h
// ---------------------------------------------------------------------------
using esp_err_t = int;

#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_NO_MEM 0x101
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_INVALID_STATE 0x103
#define ESP_ERR_INVALID_SIZE 0x104
#define ESP_ERR_INVALID_TIME 0x105
#define ESP_ERR_TIMEOUT 0x107
#define ESP_ERR_NOT_FINISHED 0x108
#define ESP_ERR_NOT_FOUND 0x105
#define ESP_ERR_WIFI_CONN 0x3001

inline const char* esp_err_to_name(esp_err_t err) {
    (void)err;
    return "ESP_ERR";
}

// ---------------------------------------------------------------------------
// esp_attr.h
// ---------------------------------------------------------------------------
#define IRAM_ATTR __attribute__((section(".iram.text")))
#define DRAM_ATTR
#define ESP_INTR_FLAG_IRAM (1 << 10)

// ---------------------------------------------------------------------------
// esp_log.h
// ---------------------------------------------------------------------------
#define ESP_LOG_LEVEL(tag, level, format, ...)                        \
    do {                                                               \
        (void)(tag);                                                   \
        std::printf("%s " format "\n", level, ##__VA_ARGS__);          \
    } while (0)

#define ESP_LOGE(tag, format, ...) ESP_LOG_LEVEL(tag, "E", format, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) ESP_LOG_LEVEL(tag, "W", format, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) ESP_LOG_LEVEL(tag, "I", format, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) ESP_LOG_LEVEL(tag, "D", format, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) ESP_LOG_LEVEL(tag, "V", format, ##__VA_ARGS__)

// ---------------------------------------------------------------------------
// esp_heap_caps.h
// ---------------------------------------------------------------------------
#define MALLOC_CAP_8BIT (1 << 2)
inline size_t heap_caps_get_free_size(int caps) {
    (void)caps;
    return 4096;
}

// ---------------------------------------------------------------------------
// driver/gpio.h
// ---------------------------------------------------------------------------
using gpio_num_t = int;

#define GPIO_NUM_NC (-1)
#define GPIO_NUM_0 0
#define GPIO_NUM_4 4
#define GPIO_NUM_5 5
#define GPIO_NUM_12 12
#define GPIO_NUM_13 13
#define GPIO_NUM_14 14
#define GPIO_NUM_18 18
#define GPIO_NUM_19 19
#define GPIO_NUM_21 21
#define GPIO_NUM_22 22
#define GPIO_NUM_25 25
#define GPIO_NUM_27 27

enum gpio_mode_t {
    GPIO_MODE_INPUT = 1,
    GPIO_MODE_OUTPUT = 2,
};
enum gpio_pullup_t {
    GPIO_PULLUP_DISABLE = 0,
    GPIO_PULLUP_ENABLE = 1,
};
enum gpio_pulldown_t {
    GPIO_PULLDOWN_DISABLE = 0,
    GPIO_PULLDOWN_ENABLE = 1,
};
enum gpio_int_type_t {
    GPIO_INTR_DISABLE = 0,
    GPIO_INTR_POSEDGE = 1,
    GPIO_INTR_NEGEDGE = 2,
    GPIO_INTR_ANYEDGE = 3,
};

struct gpio_config_t {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    gpio_pullup_t pull_up_en;
    gpio_pulldown_t pull_down_en;
    gpio_int_type_t intr_type;
};

inline esp_err_t gpio_config(const gpio_config_t* cfg) {
    (void)cfg;
    return ESP_OK;
}
inline esp_err_t gpio_install_isr_service(int flags) {
    (void)flags;
    return ESP_OK;
}
inline esp_err_t gpio_isr_handler_add(gpio_num_t pin, void (*handler)(void*), void* arg) {
    (void)pin;
    (void)handler;
    (void)arg;
    return ESP_OK;
}
inline int gpio_get_level(gpio_num_t pin) {
    (void)pin;
    return 1;
}

// ---------------------------------------------------------------------------
// driver/ledc.h
// ---------------------------------------------------------------------------
using ledc_channel_t = int;
using ledc_timer_t = int;
#define LEDC_CHANNEL_0 0
#define LEDC_CHANNEL_1 1
#define LEDC_TIMER_0 0
#define LEDC_TIMER_1 1
#define LEDC_LOW_SPEED_MODE 0
#define LEDC_HIGH_SPEED_MODE 1
#define LEDC_AUTO_CLK 0

using ledc_timer_bit_t = int;
using ledc_mode_t = int;

struct ledc_timer_config_t {
    ledc_mode_t speed_mode;
    ledc_timer_bit_t duty_resolution;
    ledc_timer_t timer_num;
    uint32_t freq_hz;
    ledc_mode_t clk_cfg;
};

struct ledc_channel_config_t {
    int gpio_num;
    ledc_mode_t speed_mode;
    ledc_channel_t channel;
    ledc_timer_t timer_sel;
    uint32_t duty;
    int hpoint;
    struct {
        bool capable;
        bool required;
    } pmf_cfg;
};

inline esp_err_t ledc_timer_config(const ledc_timer_config_t* cfg) {
    (void)cfg;
    return ESP_OK;
}
inline esp_err_t ledc_channel_config(const ledc_channel_config_t* cfg) {
    (void)cfg;
    return ESP_OK;
}
inline esp_err_t ledc_set_duty(ledc_mode_t mode, ledc_channel_t channel, uint32_t duty) {
    (void)mode;
    (void)channel;
    (void)duty;
    return ESP_OK;
}
inline esp_err_t ledc_update_duty(ledc_mode_t mode, ledc_channel_t channel) {
    (void)mode;
    (void)channel;
    return ESP_OK;
}

// ---------------------------------------------------------------------------
// FreeRTOS
// ---------------------------------------------------------------------------
using BaseType_t = int;
using UBaseType_t = unsigned int;
using TickType_t = uint32_t;
using configSTACK_DEPTH_TYPE = uint32_t;
using TaskHandle_t = void*;
using QueueHandle_t = void*;
using SemaphoreHandle_t = void*;
using EventGroupHandle_t = void*;
using EventBits_t = uint32_t;
using TaskFunction_t = void (*)(void*);

#define portMAX_DELAY 0xffffffffU
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#define pdFAIL 0
#define configTICK_RATE_HZ 1000
#define BIT0 (1 << 0)
#define BIT1 (1 << 1)
#define BIT2 (1 << 2)
#define BIT3 (1 << 3)
#define BIT4 (1 << 4)
#define BIT5 (1 << 5)

inline TickType_t pdMS_TO_TICKS(uint32_t ms) { return ms; }

// queues
inline QueueHandle_t xQueueCreate(size_t length, size_t itemSize) {
    (void)length;
    (void)itemSize;
    return reinterpret_cast<QueueHandle_t>(0x10);
}
inline void vQueueDelete(QueueHandle_t queue) { (void)queue; }
inline BaseType_t xQueueSend(QueueHandle_t queue, const void* item, TickType_t wait) {
    (void)queue;
    (void)item;
    (void)wait;
    return pdTRUE;
}
inline BaseType_t xQueueSendFromISR(QueueHandle_t queue, const void* item, BaseType_t* higher) {
    (void)queue;
    (void)item;
    if (higher != nullptr) {
        *higher = pdFALSE;
    }
    return pdTRUE;
}
inline BaseType_t xQueueReceive(QueueHandle_t queue, void* item, TickType_t wait) {
    (void)queue;
    (void)item;
    (void)wait;
    return pdTRUE;
}

// semaphores
inline SemaphoreHandle_t xSemaphoreCreateMutex() { return reinterpret_cast<SemaphoreHandle_t>(0x20); }
inline BaseType_t xSemaphoreTake(SemaphoreHandle_t sem, TickType_t wait) {
    (void)sem;
    (void)wait;
    return pdTRUE;
}
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t sem) {
    (void)sem;
    return pdTRUE;
}
inline void vSemaphoreDelete(SemaphoreHandle_t sem) { (void)sem; }

// event groups
inline EventGroupHandle_t xEventGroupCreate() { return reinterpret_cast<EventGroupHandle_t>(0x30); }
inline void vEventGroupDelete(EventGroupHandle_t group) { (void)group; }
inline EventBits_t xEventGroupSetBits(EventGroupHandle_t group, EventBits_t bits) {
    (void)group;
    return bits;
}
inline EventBits_t xEventGroupClearBits(EventGroupHandle_t group, EventBits_t bits) {
    (void)group;
    return bits;
}
inline EventBits_t xEventGroupGetBits(EventGroupHandle_t group) {
    (void)group;
    return 0;
}
inline EventBits_t xEventGroupWaitBits(EventGroupHandle_t group, EventBits_t bits, BaseType_t clearOnExit,
                                      BaseType_t waitAll, TickType_t wait) {
    (void)group;
    (void)bits;
    (void)clearOnExit;
    (void)waitAll;
    (void)wait;
    return bits;
}

// tasks
inline BaseType_t xTaskCreate(TaskFunction_t fn, const char* name, configSTACK_DEPTH_TYPE stack, void* arg,
                              UBaseType_t priority, TaskHandle_t* handle) {
    (void)fn;
    (void)name;
    (void)stack;
    (void)arg;
    (void)priority;
    if (handle != nullptr) {
        *handle = reinterpret_cast<TaskHandle_t>(0x40);
    }
    return pdPASS;
}
inline BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char* name, configSTACK_DEPTH_TYPE stack,
                                           void* arg, UBaseType_t priority, TaskHandle_t* handle,
                                           BaseType_t core) {
    (void)core;
    return xTaskCreate(fn, name, stack, arg, priority, handle);
}
inline void vTaskDelay(TickType_t ticks) { (void)ticks; }
inline void vTaskDelete(TaskHandle_t task) { (void)task; }

// ---------------------------------------------------------------------------
// esp_event.h
// ---------------------------------------------------------------------------
using esp_event_base_t = const char*;
#define ESP_EVENT_ANY_ID -1

// The real event bases are string literals as well; keeping them in variables
// avoids the -Wstring-compare warning of the host compiler.
inline const char* const WIFI_EVENT = "WIFI_EVENT";
inline const char* const IP_EVENT = "IP_EVENT";

using esp_event_handler_t = void (*)(void* arg, esp_event_base_t base, int32_t id, void* data);

inline esp_err_t esp_event_loop_create_default() { return ESP_OK; }
inline esp_err_t esp_event_handler_register(esp_event_base_t base, int32_t id, esp_event_handler_t handler,
                                            void* arg) {
    (void)base;
    (void)id;
    (void)handler;
    (void)arg;
    return ESP_OK;
}

// ---------------------------------------------------------------------------
// esp_netif.h
// ---------------------------------------------------------------------------
struct esp_netif_obj;
using esp_netif_t = struct esp_netif_obj;

typedef struct {
    uint32_t addr;
} esp_ip4_addr_t;

typedef struct {
    esp_ip4_addr_t ip;
    esp_ip4_addr_t netmask;
    esp_ip4_addr_t gw;
} esp_netif_ip_info_t;

typedef enum {
    IP_EVENT_STA_GOT_IP = 0,
    IP_EVENT_STA_LOST_IP = 1,
} ip_event_t;

#define IPSTR "%d.%d.%d.%d"
#define IP2STR(a) (int)(((a)->addr) & 0xff), (int)((((a)->addr) >> 8) & 0xff), \
    (int)((((a)->addr) >> 16) & 0xff), (int)((((a)->addr) >> 24) & 0xff)

inline esp_err_t esp_netif_init() { return ESP_OK; }
inline esp_netif_t* esp_netif_create_default_wifi_sta() {
    return reinterpret_cast<esp_netif_t*>(0x50);
}
inline void esp_netif_destroy(esp_netif_t* netif) { (void)netif; }
inline esp_err_t esp_netif_get_ip_info(esp_netif_t* netif, esp_netif_ip_info_t* info) {
    (void)netif;
    if (info != nullptr) {
        info->ip.addr = 0xC0A80169;  // 192.168.1.105
        info->netmask.addr = 0xFFFFFF00;
        info->gw.addr = 0xC0A80101;
    }
    return ESP_OK;
}

// ---------------------------------------------------------------------------
// esp_wifi.h
// ---------------------------------------------------------------------------
using wifi_interface_t = int;
#define WIFI_IF_STA 0
#define WIFI_IF_AP 1
using wifi_mode_t = int;
#define WIFI_MODE_NULL 0
#define WIFI_MODE_STA 1
#define WIFI_MODE_AP 2
#define WIFI_MODE_APSTA 3
using wifi_storage_t = int;
#define WIFI_STORAGE_RAM 1
#define WIFI_STORAGE_FLASH 2
using wifi_auth_mode_t = int;
#define WIFI_AUTH_OPEN 0
#define WIFI_AUTH_WPA_WPA2_PSK 6

struct wifi_scan_threshold_t {
    wifi_auth_mode_t authmode;
    int rssi;
};

struct wifi_sta_config_t {
    uint8_t ssid[32];
    uint8_t password[64];
    uint8_t ssid_len;
    uint8_t password_len;
    wifi_scan_threshold_t threshold;
    struct {
        bool capable;
        bool required;
    } pmf_cfg;
    int scan_method;
};

struct wifi_config_t {
    wifi_sta_config_t sta;
};

struct wifi_init_config_t {
    uint32_t magic;
    bool wifi_buffer_tx;
};

#define WIFI_INIT_CONFIG_DEFAULT() \
    wifi_init_config_t { 0, false }

inline esp_err_t esp_wifi_init(const wifi_init_config_t* cfg) {
    (void)cfg;
    return ESP_OK;
}
inline esp_err_t esp_wifi_set_storage(wifi_storage_t storage) {
    (void)storage;
    return ESP_OK;
}
inline esp_err_t esp_wifi_set_mode(wifi_mode_t mode) {
    (void)mode;
    return ESP_OK;
}
inline esp_err_t esp_wifi_set_config(wifi_interface_t iface, wifi_config_t* conf) {
    (void)iface;
    (void)conf;
    return ESP_OK;
}
inline esp_err_t esp_wifi_get_config(wifi_interface_t iface, wifi_config_t* conf) {
    (void)iface;
    if (conf != nullptr) {
        std::memset(conf, 0, sizeof(*conf));
        std::strncpy(reinterpret_cast<char*>(conf->sta.ssid), "iotexpo", sizeof(conf->sta.ssid) - 1);
        std::strncpy(reinterpret_cast<char*>(conf->sta.password), "12345678", sizeof(conf->sta.password) - 1);
    }
    return ESP_OK;
}
inline esp_err_t esp_wifi_start() { return ESP_OK; }
inline esp_err_t esp_wifi_stop() { return ESP_OK; }
inline esp_err_t esp_wifi_connect() { return ESP_OK; }
inline esp_err_t esp_wifi_disconnect() { return ESP_OK; }

enum wifi_event_t {
    WIFI_EVENT_WIFI_READY = 0,
    WIFI_EVENT_SCAN_DONE = 1,
    WIFI_EVENT_STA_START = 3,
    WIFI_EVENT_STA_STOP = 4,
    WIFI_EVENT_STA_CONNECTED = 5,
    WIFI_EVENT_STA_DISCONNECTED = 6,
};

// NOTE: the esp_netif_sntp.h API (including the "struct timeval" forward
// declaration) lives in its own stub header, exactly like the real ESP-IDF,
// where only esp_netif_sntp.h mentions that type.

