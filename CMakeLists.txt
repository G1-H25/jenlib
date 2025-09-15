cmake_minimum_required(VERSION 3.20)
project(jenlib_gpio C CXX)

option(BUILD_TESTING "Build tests" ON)

# Library: header-only for now
add_library(jenlib_gpio
    src/gpio/GPIO.cpp
    src/gpio/drivers/NativeGpioDriver.cpp
    src/gpio/drivers/ArduinoGpioDriver.cpp
    src/onewire/drivers/NativeOneWireBus.cpp
)
target_include_directories(jenlib_gpio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(jenlib_gpio PUBLIC cxx_std_17)

# Optional Arduino OneWire adapter
option(JENLIB_ENABLE_ARDUINO_ONEWIRE "Enable Arduino OneWire adapter" OFF)
if(JENLIB_ENABLE_ARDUINO_ONEWIRE)
    target_sources(jenlib_gpio PRIVATE
        src/onewire/drivers/ArduinoOneWireBus.cpp
    )
    target_compile_definitions(jenlib_gpio PRIVATE JENLIB_ENABLE_ARDUINO_ONEWIRE=1)
endif()

if(BUILD_TESTING)
    include(FetchContent)
    # Fetch Unity (ThrowTheSwitch/Unity)
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
        ${unity_SOURCE_DIR}/src/unity.c
    )
    target_include_directories(jenlib_gpio_tests PRIVATE ${unity_SOURCE_DIR}/src)
    target_link_libraries(jenlib_gpio_tests PRIVATE jenlib_gpio)

    add_test(NAME jenlib_gpio_tests COMMAND jenlib_gpio_tests)
endif()


