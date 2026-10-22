#pragma once

// ---------------------------------------------------------------------------
// esp_timer.h (host stub)
//
// Mirrors the ESP-IDF v6.0 header: it provides "esp_timer_get_time()".
// The declaration lives in this header on purpose - a source file that calls
// "esp_timer_get_time()" has to include "esp_timer.h", exactly like in a real
// ESP-IDF build.
// ---------------------------------------------------------------------------
#include "esp_idf_stub.hpp"

inline int64_t esp_timer_get_time() { return 0; }
