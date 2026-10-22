# ESP32 Modular Digital Clock - main component
#
# The component is split into two layers:
#   core/    - pure C++ domain logic, no ESP-IDF dependency, host testable
#   drivers/ - ESP-IDF v6 adapters (I2C, GPIO, LEDC, Wi-Fi/SNTP, timers)
#
# Include directories are exported so that both layers can include each other
# with a flat include path ("Bcd.hpp", "I2CBus.hpp", ...).

idf_component_register(
    SRCS
        "main.cpp"
        # --- pure domain layer -------------------------------------------
        "core/DateTime.cpp"
        "core/Debounce.cpp"
        "core/UiStateMachine.cpp"
        "core/AlarmLogic.cpp"
        "core/StopwatchLogic.cpp"
        "core/TimeManager.cpp"
        "core/AlarmEngine.cpp"
        "core/StopwatchEngine.cpp"
        "core/UiController.cpp"
        "core/UiRenderer.cpp"
        # --- hardware abstraction layer ----------------------------------
        "drivers/I2CBus.cpp"
        "drivers/DS1307.cpp"
        "drivers/SSD1306.cpp"
        "drivers/Button.cpp"
        "drivers/ButtonManager.cpp"
        "drivers/Buzzer.cpp"
        "drivers/NTPManager.cpp"
        "drivers/AlarmManager.cpp"
        "drivers/Stopwatch.cpp"
        "drivers/SystemClock.cpp"
    INCLUDE_DIRS
        "."
        "core"
        "drivers"
    REQUIRES
        esp_driver_i2c
        esp_driver_gpio
        esp_driver_ledc
        esp_timer
        esp_wifi
        esp_netif
        esp_event
        freertos
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Wno-unused-parameter)
