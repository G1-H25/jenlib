//! @file tests/AllTests.cpp
//! @brief All tests for the GPIO library.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <cstdint>
#include "unity.h"

// Declarations of test functions from other translation units
extern void test_digital_roundtrip(void);
extern void test_analog_roundtrip(void);
extern void test_resolution_forwarding(void);
extern void test_native_driver_roundtrip(void);
extern void test_native_driver_analog(void);
extern void test_voltage_levels_and_tmp36(void);
extern void test_tmp36_conversion_10bit_5v(void);
extern void test_tmp36_conversion_12bit_3v3(void);

void setUp(void) {}
void tearDown(void) {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_digital_roundtrip);
    RUN_TEST(test_analog_roundtrip);
    RUN_TEST(test_resolution_forwarding);
    RUN_TEST(test_native_driver_roundtrip);
    RUN_TEST(test_native_driver_analog);
    RUN_TEST(test_voltage_levels_and_tmp36);
    RUN_TEST(test_tmp36_conversion_10bit_5v);
    RUN_TEST(test_tmp36_conversion_12bit_3v3);
    return UNITY_END();
}


