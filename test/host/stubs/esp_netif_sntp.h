#pragma once

#include "esp_idf_stub.hpp"

// ---------------------------------------------------------------------------
// esp_netif_sntp.h (host stub)
//
// Kept in its own header on purpose: in the real ESP-IDF this is the *only*
// header that mentions "struct timeval", and it does NOT include <sys/time.h>,
// so the POSIX type is merely forward declared there. The host tool chain
// (<ctime> on mingw) declares ::timeval globally, therefore the "Clock::timeval"
// trap cannot be reproduced by the type check alone - it is guarded on the
// source level by test/host/check_timeval_include.py instead.
// ---------------------------------------------------------------------------
struct timeval;

using esp_sntp_time_cb_t = void (*)(struct timeval* tv);

struct esp_sntp_config {
    bool smooth_sync;
    bool server_from_dhcp;
    bool wait_for_sync;
    bool start;
    esp_sntp_time_cb_t sync_cb;
    bool renew_servers_after_new_ip;
    ip_event_t ip_event_to_renew;
    size_t index_of_first_server;
    size_t num_of_servers;
    const char* servers[3];
};
using esp_sntp_config_t = struct esp_sntp_config;

inline esp_err_t esp_netif_sntp_init(const esp_sntp_config_t* config) {
    (void)config;
    return ESP_OK;
}
inline esp_err_t esp_netif_sntp_start() { return ESP_OK; }
inline void esp_netif_sntp_deinit() {}
inline esp_err_t esp_netif_sntp_sync_wait(TickType_t tout) {
    (void)tout;
    return ESP_OK;
}
