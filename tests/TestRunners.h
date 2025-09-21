//! @file tests/TestRunners.h
//! @brief Test runner functions for organizing tests by module
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef TESTS_TESTRUNNERS_H_
#define TESTS_TESTRUNNERS_H_

//! @brief Run all GPIO-related tests
void run_gpio_tests(void);

//! @brief Run all BLE-related tests
void run_ble_tests(void);

//! @brief Run all BLE callback tests
void run_ble_callback_tests(void);

//! @brief Run all BLE interface contract tests
void run_ble_interface_tests(void);

//! @brief Run all BLE integration tests
void run_ble_integration_tests(void);

//! @brief Run all state machine tests
void run_state_machine_tests(void);

//! @brief Run all time driver tests
void run_time_driver_tests(void);

//! @brief Run all measurement tests (defined in MeasurementTests.cpp)
void run_measurement_tests(void);

#endif // TESTS_TESTRUNNERS_H_
