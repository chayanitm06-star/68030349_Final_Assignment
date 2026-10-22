/**
 * @file NTPManager.hpp
 * @brief Wi-Fi station driver and SNTP client (ESP-IDF v6).
 *
 * Responsibilities:
 *   1. Bring up the TCP/IP stack, the default event loop and the Wi-Fi
 *      station interface.
 *   2. Join the access point (SSID "iotexpo") and wait for a DHCP lease.
 *   3. Configure the lwIP SNTP client (pool.ntp.org) and wait for the first
 *      synchronisation; the POSIX system clock is updated by the SNTP client.
 *
 * The class implements @ref Clock::INtpClient so that the domain layer stays
 * completely free of ESP-IDF specifics.
 */
#pragma once

#include <cstdint>
#include <string>

#include "Interfaces.hpp"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace Clock {

class NTPManager : public INtpClient {
public:
    /** @brief Everything the client needs to talk to the access point. */
    struct Settings {
        const char* ssid{nullptr};
        const char* password{nullptr};
        /** @brief Second network tried when the primary one is unreachable. */
        const char* fallbackSsid{nullptr};
        const char* fallbackPassword{nullptr};
        const char* primaryServer{nullptr};
        const char* secondaryServer{nullptr};
        uint32_t connectTimeoutMs{15000};
        uint32_t syncTimeoutMs{10000};
    };

    explicit NTPManager(Settings settings);
    ~NTPManager() override;

    NTPManager(const NTPManager&) = delete;
    NTPManager& operator=(const NTPManager&) = delete;

    /** @brief Initialise the network stack and start the Wi-Fi station. */
    esp_err_t init();

    /** @brief Join the access point and wait until an IP address is assigned. */
    bool connect() override;

    /** @brief Name of the access point the station is configured for. */
    [[nodiscard]] const char* activeSsid() const noexcept { return m_activeSsid; }

    /** @brief Start the SNTP client and wait for a synchronised system time. */
    bool syncTime() override;

    [[nodiscard]] bool isSynced() const override { return m_synced; }

    [[nodiscard]] bool isWifiConnected() const noexcept { return m_wifiConnected; }
    [[nodiscard]] bool isInitialised() const noexcept { return m_initialised; }

    /** @brief Dotted quad of the assigned station address ("0.0.0.0" if none). */
    [[nodiscard]] std::string ipAddress() const;

    /** @brief Unix timestamp of the last successful synchronisation. */
    [[nodiscard]] int64_t lastUnixTime() const noexcept { return m_lastUnixTime; }

    /** @brief Result of the last failed operation. */
    [[nodiscard]] esp_err_t lastError() const noexcept { return m_lastError; }

    /** @brief lwIP SNTP callback (the API does not allow a user context). */
    static void onSntpTimeSync(struct timeval* tv);

private:
    static void eventTrampoline(void* arg, esp_event_base_t base, int32_t id, void* data);

    void handleWifiEvent(int32_t id, void* data);
    void handleIpEvent(int32_t id, void* data);

    /** @brief Program the station configuration (SSID / password) into the driver. */
    esp_err_t applyStationConfig(const char* ssid, const char* password);

    /** @brief One connection attempt; returns false on timeout or driver error. */
    bool tryConnect();

    Settings m_settings;
    esp_netif_t* m_staNetif{nullptr};
    EventGroupHandle_t m_events{nullptr};
    const char* m_activeSsid{nullptr};

    bool m_initialised{false};
    bool m_wifiConnected{false};
    bool m_synced{false};
    bool m_sntpInitialised{false};

    esp_err_t m_lastError{ESP_OK};
    int64_t m_lastUnixTime{0};
};

}  // namespace Clock
