cmake_minimum_required(VERSION 3.20)
project(jenlib_gpio C CXX)

option(BUILD_TESTING "Build tests" ON)

# Detect build environment
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME STREQUAL "Generic")
    # Likely Arduino toolchain
    set(ARDUINO_BUILD TRUE)
    set(ESP_IDF_BUILD FALSE)
    message(STATUS "Arduino build detected")
elseif(DEFINED IDF_PATH OR DEFINED ENV{IDF_PATH})
    # ESP-IDF build environment
    set(ARDUINO_BUILD FALSE)
    set(ESP_IDF_BUILD TRUE)
    message(STATUS "ESP-IDF build detected")
else()
    # Native build environment
    set(ARDUINO_BUILD FALSE)
    set(ESP_IDF_BUILD FALSE)
    message(STATUS "Native build detected")
endif()

# Base sources (always included)
set(JENLIB_SOURCES
    src/gpio/GPIO.cpp
    src/ble/Ids.cpp
    src/ble/Messages.cpp
    src/measurement/Measurement.cpp
    src/events/EventDispatcher.cpp
    src/time/Time.cpp
    src/state/SensorStateMachine.cpp
    src/state/BrokerStateMachine.cpp
)

# Platform-specific sources
if(ESP_IDF_BUILD)
    # ESP-IDF specific sources
    list(APPEND JENLIB_SOURCES
        src/gpio/drivers/EspIdfGpioDriver.cpp
        src/ble/drivers/EspIdfBleDriver.cpp
        src/time/drivers/EspIdfTimeDriver.cpp
        src/onewire/drivers/EspIdfOneWireBus.cpp
    )
    message(STATUS "Including ESP-IDF drivers")
elseif(ARDUINO_BUILD)
    # Arduino specific sources
    list(APPEND JENLIB_SOURCES
        src/gpio/drivers/ArduinoGpioDriver.cpp
        src/ble/drivers/ArduinoBleDriver.cpp
        src/time/drivers/ArduinoTimeDriver.cpp
    )
    message(STATUS "Including Arduino drivers")
else()
    # Native-only sources (excluded on Arduino and ESP-IDF)
    list(APPEND JENLIB_SOURCES
        src/gpio/drivers/NativeGpioDriver.cpp
        src/onewire/drivers/NativeOneWireBus.cpp
        src/ble/drivers/NativeBleDriver.cpp
        src/ble/drivers/NativeBleCharacteristic.cpp
        src/ble/drivers/NativeBleService.cpp
        src/time/drivers/NativeTimeDriver.cpp
    )
    message(STATUS "Including native drivers")
endif()

add_library(jenlib_gpio ${JENLIB_SOURCES})
target_include_directories(jenlib_gpio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(jenlib_gpio PUBLIC cxx_std_17)

# Only use for Arduino when you need to use the OneWire library
option(JENLIB_ENABLE_ARDUINO_ONEWIRE "Enable Arduino OneWire adapter" OFF)
if(JENLIB_ENABLE_ARDUINO_ONEWIRE)
    target_sources(jenlib_gpio PRIVATE
        src/onewire/drivers/ArduinoOneWireBus.cpp
    )
    target_compile_definitions(jenlib_gpio PRIVATE JENLIB_ENABLE_ARDUINO_ONEWIRE=1)
endif()

if(BUILD_TESTING)
    include(FetchContent)
    # Fetch Unity
    FetchContent_Declare(
        Unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.6.0
    )
    FetchContent_MakeAvailable(Unity)

    enable_testing()

    add_executable(jenlib_gpio_tests
        tests/AllTests.cpp
        tests/PinTests.cpp
        tests/PinTypeTests.cpp
        tests/NativeDriverTests.cpp
        tests/Tmp36Tests.cpp
        tests/PayloadTests.cpp
        tests/BleTests.cpp
        tests/MeasurementTests.cpp
        tests/StateMachineTests.cpp
        tests/TimeDriverTests.cpp
        ${unity_SOURCE_DIR}/src/unity.c
    )
    target_include_directories(jenlib_gpio_tests PRIVATE ${unity_SOURCE_DIR}/src)
    target_link_libraries(jenlib_gpio_tests PRIVATE jenlib_gpio)

    add_test(NAME jenlib_gpio_tests COMMAND jenlib_gpio_tests)

    # Smoke Tests - Integration tests that exercise the full example path
    add_executable(jenlib_smoke_tests
        smoke_tests/FinalSmokeTest.cpp
        ${unity_SOURCE_DIR}/src/unity.c
    )
    target_include_directories(jenlib_smoke_tests PRIVATE
        ${unity_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(jenlib_smoke_tests PRIVATE jenlib_gpio)

    add_test(NAME jenlib_smoke_tests COMMAND jenlib_smoke_tests)
endif()


