//! @file tests/AllTests_Clean.cpp
//! @brief All tests for the jenlib library - clean organized version
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <unity.h>
#include "tests/TestRunners.h"

void setUp(void) {}
void tearDown(void) {}

int main() {
    UNITY_BEGIN();

    // Run all test suites
    run_gpio_tests();
    run_ble_tests();
    run_ble_callback_tests();
    run_ble_interface_tests();
    run_ble_integration_tests();
    run_state_machine_tests();
    run_time_driver_tests();
    run_measurement_tests();

    return UNITY_END();
}

