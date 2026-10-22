#include "I2CBus.hpp"

#include <cinttypes>

#include "esp_heap_caps.h"
#include "esp_log.h"

namespace Clock {
namespace {
constexpr const char* kTag = "I2CBus";
}  // namespace

I2CBus::I2CBus(i2c_port_num_t port, gpio_num_t sdaPin, gpio_num_t sclPin, uint32_t clockHz)
    : m_port(port), m_sdaPin(sdaPin), m_sclPin(sclPin), m_clockHz(clockHz) {
    m_mutex = xSemaphoreCreateMutex();
}

I2CBus::~I2CBus() {
    if (m_bus != nullptr) {
        i2c_del_master_bus(m_bus);
        m_bus = nullptr;
    }
    if (m_mutex != nullptr) {
        vSemaphoreDelete(m_mutex);
        m_mutex = nullptr;
    }
}

esp_err_t I2CBus::init() {
    if (m_bus != nullptr) {
        return ESP_OK;
    }
    if (m_mutex == nullptr) {
        m_mutex = xSemaphoreCreateMutex();
        if (m_mutex == nullptr) {
            ESP_LOGE(kTag, "cannot create the I2C mutex");
            return ESP_ERR_NO_MEM;
        }
    }

    i2c_master_bus_config_t busConfig{};
    busConfig.i2c_port = m_port;
    busConfig.sda_io_num = m_sdaPin;
    busConfig.scl_io_num = m_sclPin;
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;

    const esp_err_t err = i2c_new_master_bus(&busConfig, &m_bus);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "i2c_new_master_bus failed: %s", esp_err_to_name(err));
        m_bus = nullptr;
        return err;
    }

    ESP_LOGI(kTag, "I2C master ready (port=%d, SDA=%d, SCL=%d, %" PRIu32 " Hz)", static_cast<int>(m_port),
             static_cast<int>(m_sdaPin), static_cast<int>(m_sclPin), m_clockHz);
    return ESP_OK;
}

bool I2CBus::lock(uint32_t timeoutMs) {
    if (m_mutex == nullptr) {
        return false;
    }
    const TickType_t ticks = (timeoutMs == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs);
    return xSemaphoreTake(m_mutex, ticks) == pdTRUE;
}

void I2CBus::unlock() {
    if (m_mutex != nullptr) {
        xSemaphoreGive(m_mutex);
    }
}

esp_err_t I2CBus::deviceHandle(uint8_t address, i2c_master_dev_handle_t* out) {
    if (m_bus == nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    for (auto& device : m_devices) {
        if (device.used && device.address == address) {
            *out = device.handle;
            return ESP_OK;
        }
    }

    i2c_device_config_t deviceConfig{};
    deviceConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    deviceConfig.device_address = address;
    deviceConfig.scl_speed_hz = m_clockHz;

    i2c_master_dev_handle_t handle = nullptr;
    const esp_err_t err = i2c_master_bus_add_device(m_bus, &deviceConfig, &handle);
    if (err != ESP_OK) {
        return err;
    }

    for (auto& device : m_devices) {
        if (!device.used) {
            device.used = true;
            device.address = address;
            device.handle = handle;
            break;
        }
    }

    *out = handle;
    return ESP_OK;
}

esp_err_t I2CBus::probe(uint16_t address, int timeoutMs) {
    if (!lock(100)) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t err = ESP_ERR_INVALID_STATE;
    if (m_bus != nullptr) {
        err = i2c_master_probe(m_bus, address, timeoutMs);
    }
    unlock();
    return err;
}

esp_err_t I2CBus::writeRegister(uint8_t deviceAddress, uint8_t reg, uint8_t value, int timeoutMs) {
    return writeRegisters(deviceAddress, reg, &value, 1, timeoutMs);
}

esp_err_t I2CBus::writeRegisters(uint8_t deviceAddress, uint8_t reg, const uint8_t* data, size_t length,
                                 int timeoutMs) {
    if (data == nullptr || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!lock(100)) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_master_dev_handle_t handle = nullptr;
    esp_err_t err = deviceHandle(deviceAddress, &handle);
    if (err == ESP_OK) {
        uint8_t payload[32];
        if (length + 1u > sizeof(payload)) {
            unlock();
            return ESP_ERR_INVALID_SIZE;
        }
        payload[0] = reg;
        for (size_t i = 0; i < length; ++i) {
            payload[i + 1] = data[i];
        }
        err = i2c_master_transmit(handle, payload, length + 1u, timeoutMs);
    }

    unlock();
    return err;
}

esp_err_t I2CBus::writeRaw(uint8_t deviceAddress, const uint8_t* data, size_t length, int timeoutMs) {
    if (data == nullptr || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!lock(100)) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_master_dev_handle_t handle = nullptr;
    esp_err_t err = deviceHandle(deviceAddress, &handle);
    if (err == ESP_OK) {
        err = i2c_master_transmit(handle, data, length, timeoutMs);
    }

    unlock();
    return err;
}

esp_err_t I2CBus::readRegister(uint8_t deviceAddress, uint8_t reg, uint8_t* value, int timeoutMs) {
    return readRegisters(deviceAddress, reg, value, 1, timeoutMs);
}

esp_err_t I2CBus::readRegisters(uint8_t deviceAddress, uint8_t reg, uint8_t* buffer, size_t length,
                                 int timeoutMs) {
    if (buffer == nullptr || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!lock(100)) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_master_dev_handle_t handle = nullptr;
    esp_err_t err = deviceHandle(deviceAddress, &handle);
    if (err == ESP_OK) {
        err = i2c_master_transmit_receive(handle, &reg, 1, buffer, length, timeoutMs);
    }

    unlock();
    return err;
}

esp_err_t I2CBus::readWriteRegisters(uint8_t deviceAddress, uint8_t reg, uint8_t* buffer, size_t length,
                                     int timeoutMs) {
    return readRegisters(deviceAddress, reg, buffer, length, timeoutMs);
}

uint32_t I2CBus::freeHeapBytes() const {
    return static_cast<uint32_t>(heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

}  // namespace Clock
