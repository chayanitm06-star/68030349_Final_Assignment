#pragma once

// ---------------------------------------------------------------------------
// driver/i2c_master.h (host stub)
//
// Mirrors the ESP-IDF v6.0 header:
//   * driver/i2c_master.h -> driver/i2c_types.h -> hal/i2c_types.h
//   * "i2c_port_num_t" is a plain "typedef int" in v6 and the port constants
//     (I2C_NUM_0, I2C_NUM_1, ...) come from hal/i2c_types.h.
//
// The types live in this header on purpose: a source file that uses
// "i2c_port_num_t" or "I2C_NUM_0" must include the I2C driver header, exactly
// like in a real ESP-IDF build. Including only "driver/gpio.h" is not enough.
// ---------------------------------------------------------------------------
#include "esp_idf_stub.hpp"

using i2c_port_num_t = int;
#define I2C_NUM_0 0
#define I2C_NUM_1 1
#define I2C_CLK_SRC_DEFAULT 0

struct i2c_master_bus_t;
using i2c_master_bus_handle_t = struct i2c_master_bus_t*;
struct i2c_master_dev_t;
using i2c_master_dev_handle_t = struct i2c_master_dev_t*;

enum i2c_addr_bit_len_t {
    I2C_ADDR_BIT_LEN_7 = 0,
    I2C_ADDR_BIT_LEN_10 = 1,
};

struct i2c_master_bus_config_t {
    i2c_port_num_t i2c_port;
    int sda_io_num;
    int scl_io_num;
    int clk_source;
    uint8_t glitch_ignore_cnt;
    struct {
        bool enable_internal_pullup;
    } flags;
};

struct i2c_device_config_t {
    i2c_addr_bit_len_t dev_addr_length;
    uint16_t device_address;
    uint32_t scl_speed_hz;
};

inline esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t* cfg, i2c_master_bus_handle_t* out) {
    (void)cfg;
    *out = reinterpret_cast<i2c_master_bus_handle_t>(0x1);
    return ESP_OK;
}
inline esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus) {
    (void)bus;
    return ESP_OK;
}
inline esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus, const i2c_device_config_t* cfg,
                                           i2c_master_dev_handle_t* out) {
    (void)bus;
    (void)cfg;
    *out = reinterpret_cast<i2c_master_dev_handle_t>(0x2);
    return ESP_OK;
}
inline esp_err_t i2c_master_probe(i2c_master_bus_handle_t bus, uint16_t address, int timeoutMs) {
    (void)bus;
    (void)address;
    (void)timeoutMs;
    return ESP_OK;
}
inline esp_err_t i2c_master_transmit(i2c_master_dev_handle_t dev, const uint8_t* buffer, size_t len,
                                      int timeoutMs) {
    (void)dev;
    (void)buffer;
    (void)len;
    (void)timeoutMs;
    return ESP_OK;
}
inline esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t dev, const uint8_t* tx, size_t txLen,
                                             uint8_t* rx, size_t rxLen, int timeoutMs) {
    (void)dev;
    (void)tx;
    (void)txLen;
    (void)rx;
    (void)rxLen;
    (void)timeoutMs;
    return ESP_OK;
}
