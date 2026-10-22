# Unity test application of the ESP32 modular digital clock.
#
# The production sources of the "main" component are compiled directly into
# the test application so that the tests always run against the shipping code.

get_filename_component(CLOCK_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

idf_component_register(
    SRCS
        "unit_tests.cpp"
        # --- pure domain layer -------------------------------------------
        "${CLOCK_PROJECT_ROOT}/main/core/DateTime.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/Debounce.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/UiStateMachine.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/AlarmLogic.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/StopwatchLogic.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/TimeManager.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/AlarmEngine.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/StopwatchEngine.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/UiController.cpp"
        "${CLOCK_PROJECT_ROOT}/main/core/UiRenderer.cpp"
        # --- hardware abstraction layer ----------------------------------
        "${CLOCK_PROJECT_ROOT}/main/drivers/I2CBus.cpp"
        "${CLOCK_PROJECT_ROOT}/main/drivers/SSD1306.cpp"
        "${CLOCK_PROJECT_ROOT}/main/drivers/Button.cpp"
        "${CLOCK_PROJECT_ROOT}/main/drivers/NTPManager.cpp"
        "${CLOCK_PROJECT_ROOT}/main/drivers/Stopwatch.cpp"
    INCLUDE_DIRS
        "."
        "${CLOCK_PROJECT_ROOT}/main/core"
        "${CLOCK_PROJECT_ROOT}/main/drivers"
    REQUIRES
        unity
        esp_driver_i2c
        esp_driver_gpio
        esp_driver_ledc
        esp_timer
        esp_wifi
        esp_netif
        esp_event
        freertos
)
