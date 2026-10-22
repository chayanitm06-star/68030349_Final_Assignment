// <sys/time.h> first, so that "::timeval" always refers to the POSIX type.
#include <sys/time.h>

#include "NTPManager.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <type_traits>

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "sdkconfig.h"

namespace Clock {

// Compile time guard against the "Clock::timeval" trap: the SNTP callback must
// use the POSIX ::timeval exactly as esp_netif_sntp.h declares it. If the
// <sys/time.h> include in NTPManager.hpp is ever dropped, "struct timeval"
// inside this namespace silently declares a brand new Clock::timeval type and
// this assertion reports the mismatch with a readable message.
static_assert(std::is_same_v<decltype(&NTPManager::onSntpTimeSync), esp_sntp_time_cb_t>,
              "NTPManager::onSntpTimeSync must match esp_sntp_time_cb_t (POSIX ::timeval)");

namespace {

constexpr const char* kTag = "NTP_MGR";

constexpr EventBits_t kGotIpBit = BIT0;
constexpr EventBits_t kConnectedBit = BIT1;
constexpr EventBits_t kDisconnectedBit = BIT2;

/** @brief The lwIP SNTP callback has no user context - use a singleton pointer. */
NTPManager* s_activeInstance = nullptr;

}  // namespace

NTPManager::NTPManager(Settings settings) : m_settings(settings) {}

NTPManager::~NTPManager() {
    if (s_activeInstance == this) {
        s_activeInstance = nullptr;
    }

    if (m_sntpInitialised) {
        esp_netif_sntp_deinit();
        m_sntpInitialised = false;
    }

    if (m_initialised) {
        esp_wifi_stop();
    }

    if (m_events != nullptr) {
        vEventGroupDelete(m_events);
        m_events = nullptr;
    }

    if (m_staNetif != nullptr) {
        esp_netif_destroy(m_staNetif);
        m_staNetif = nullptr;
    }
}

esp_err_t NTPManager::init() {
    if (m_initialised) {
        return ESP_OK;
    }

    s_activeInstance = this;

    m_events = xEventGroupCreate();
    if (m_events == nullptr) {
        ESP_LOGE(kTag, "cannot create the event group");
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_netif_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(kTag, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return err;
    }

    m_staNetif = esp_netif_create_default_wifi_sta();
    if (m_staNetif == nullptr) {
        ESP_LOGE(kTag, "esp_netif_create_default_wifi_sta failed");
        return ESP_FAIL;
    }

    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &NTPManager::eventTrampoline, this);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "cannot register the Wi-Fi event handler: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &NTPManager::eventTrampoline, this);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "cannot register the IP event handler: %s", esp_err_to_name(err));
        return err;
    }

    wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&initConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_wifi_init failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_wifi_set_storage failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        return err;
    }

    err = applyStationConfig(m_settings.ssid, m_settings.password);
    if (err != ESP_OK) {
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_wifi_start failed: %s", esp_err_to_name(err));
        return err;
    }

    m_initialised = true;
    ESP_LOGI(kTag, "Wi-Fi station started, joining SSID '%s'", m_settings.ssid ? m_settings.ssid : "");
    return ESP_OK;
}

esp_err_t NTPManager::applyStationConfig(const char* ssid, const char* password) {
    wifi_config_t staConfig{};
    if (ssid != nullptr) {
        std::strncpy(reinterpret_cast<char*>(staConfig.sta.ssid), ssid,
                     sizeof(staConfig.sta.ssid) - 1);
    }
    if (password != nullptr) {
        std::strncpy(reinterpret_cast<char*>(staConfig.sta.password), password,
                     sizeof(staConfig.sta.password) - 1);
    }
    // The access point may be open or protected - accept everything.
    staConfig.sta.threshold.authmode = WIFI_AUTH_OPEN;
    staConfig.sta.pmf_cfg.capable = true;
    staConfig.sta.pmf_cfg.required = false;

    const esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &staConfig);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        return err;
    }

    m_activeSsid = ssid;
    return ESP_OK;
}

bool NTPManager::tryConnect() {
    xEventGroupClearBits(m_events, kGotIpBit | kConnectedBit | kDisconnectedBit);

    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK && err != ESP_ERR_WIFI_CONN) {
        ESP_LOGE(kTag, "esp_wifi_connect failed: %s", esp_err_to_name(err));
        m_lastError = err;
        return false;
    }

    const EventBits_t bits = xEventGroupWaitBits(m_events, kGotIpBit | kConnectedBit, pdFALSE, pdFALSE,
                                                 pdMS_TO_TICKS(m_settings.connectTimeoutMs));
    if ((bits & kGotIpBit) == 0) {
        ESP_LOGW(kTag, "Wi-Fi connection timed out after %u ms",
                 static_cast<unsigned>(m_settings.connectTimeoutMs));
        m_lastError = ESP_ERR_TIMEOUT;
        esp_wifi_disconnect();
        return false;
    }

    m_wifiConnected = true;
    ESP_LOGI(kTag, "Wi-Fi connected, IP address: %s", ipAddress().c_str());
    return true;
}

bool NTPManager::connect() {
    if (!m_initialised) {
        m_lastError = ESP_ERR_INVALID_STATE;
        return false;
    }

    if (tryConnect()) {
        return true;
    }

    // The lab access point may be out of range - the Wokwi simulator for
    // instance exposes an open network called "Wokwi-GUEST". Retry once with
    // the fallback network before giving up on the NTP synchronisation.
    if (m_settings.fallbackSsid != nullptr && m_settings.fallbackSsid[0] != '\0') {
        ESP_LOGW(kTag, "SSID '%s' is unreachable, falling back to '%s'",
                 m_settings.ssid ? m_settings.ssid : "", m_settings.fallbackSsid);
        if (applyStationConfig(m_settings.fallbackSsid, m_settings.fallbackPassword) == ESP_OK) {
            if (tryConnect()) {
                return true;
            }
        }
    }

    ESP_LOGE(kTag, "no Wi-Fi network available - the clock keeps the RTC time");
    return false;
}

bool NTPManager::syncTime() {
    if (!m_wifiConnected && !connect()) {
        return false;
    }

    if (!m_sntpInitialised) {
        esp_sntp_config_t config{};
        config.smooth_sync = false;
        config.server_from_dhcp = false;
        config.wait_for_sync = true;   // required by esp_netif_sntp_sync_wait()
        config.start = true;
        config.sync_cb = &NTPManager::onSntpTimeSync;
        config.renew_servers_after_new_IP = false;
        config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;
        config.index_of_first_server = 0;
        config.num_of_servers = 1;
        config.servers[0] = m_settings.primaryServer;

#if CONFIG_LWIP_SNTP_MAX_SERVERS >= 2
        if (m_settings.secondaryServer != nullptr) {
            config.num_of_servers = 2;
            config.servers[1] = m_settings.secondaryServer;
        }
#endif

        const esp_err_t err = esp_netif_sntp_init(&config);
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
            ESP_LOGE(kTag, "esp_netif_sntp_init failed: %s", esp_err_to_name(err));
            m_lastError = err;
            return false;
        }
        m_sntpInitialised = true;
    }

    // Restart the SNTP client so that every call performs a fresh query.
    esp_err_t err = esp_netif_sntp_start();
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "esp_netif_sntp_start failed: %s", esp_err_to_name(err));
        m_lastError = err;
        return false;
    }

    ESP_LOGI(kTag, "querying NTP server '%s' ...", m_settings.primaryServer ? m_settings.primaryServer : "");

    err = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(m_settings.syncTimeoutMs));
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "SNTP synchronisation failed: %s", esp_err_to_name(err));
        m_lastError = err;
        return false;
    }

    const ::time_t now = ::time(nullptr);
    if (now == static_cast<::time_t>(-1)) {
        m_lastError = ESP_ERR_INVALID_STATE;
        return false;
    }

    m_lastUnixTime = static_cast<int64_t>(now);
    m_synced = true;
    ESP_LOGI(kTag, "SNTP synchronised, unix time = %lld", static_cast<long long>(m_lastUnixTime));
    return true;
}

std::string NTPManager::ipAddress() const {
    if (m_staNetif == nullptr) {
        return "0.0.0.0";
    }

    esp_netif_ip_info_t info{};
    if (esp_netif_get_ip_info(m_staNetif, &info) != ESP_OK || info.ip.addr == 0) {
        return "0.0.0.0";
    }

    char text[16]{};
    snprintf(text, sizeof(text), IPSTR, IP2STR(&info.ip));
    return std::string(text);
}

void NTPManager::onSntpTimeSync(::timeval* tv) {
    if (tv == nullptr || s_activeInstance == nullptr) {
        return;
    }
    s_activeInstance->m_lastUnixTime = static_cast<int64_t>(tv->tv_sec);
    s_activeInstance->m_synced = true;
}

void NTPManager::eventTrampoline(void* arg, esp_event_base_t base, int32_t id, void* data) {
    auto* self = static_cast<NTPManager*>(arg);
    if (self == nullptr) {
        return;
    }

    if (base == WIFI_EVENT) {
        self->handleWifiEvent(id, data);
    } else if (base == IP_EVENT) {
        self->handleIpEvent(id, data);
    }
}

void NTPManager::handleWifiEvent(int32_t id, void* data) {
    (void)data;

    switch (id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(kTag, "station started, connecting ...");
            esp_wifi_connect();
            break;

        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(kTag, "associated with the access point");
            xEventGroupSetBits(m_events, kConnectedBit);
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGW(kTag, "disconnected from the access point");
            m_wifiConnected = false;
            xEventGroupSetBits(m_events, kDisconnectedBit);
            break;

        default:
            break;
    }
}

void NTPManager::handleIpEvent(int32_t id, void* data) {
    (void)data;

    if (id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(kTag, "DHCP lease acquired");
        xEventGroupSetBits(m_events, kGotIpBit);
    }
}

}  // namespace Clock
