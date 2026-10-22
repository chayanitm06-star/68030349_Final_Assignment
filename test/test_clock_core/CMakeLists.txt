# ESP32 Modular Digital Clock - Unity unit test application
#
# The test application reuses the production sources of the main component so
# that the very same code that runs on the clock is verified.
#
#   cd test/test_clock_core
#   idf.py set-target esp32
#   idf.py build flash monitor
#
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(clock_core_test)
