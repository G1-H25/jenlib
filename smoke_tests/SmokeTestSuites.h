//! @file smoke_tests/SmokeTestSuites.h
//! @brief Smoke test suite organization macros for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef SMOKE_TESTS_SMOKETESTSUITES_H_
#define SMOKE_TESTS_SMOKETESTSUITES_H_

#include "unity.h"

//! @brief Macro to run a group of smoke tests with a suite name
#define RUN_SMOKE_TEST_SUITE(suite_name, ...) \
    do { \
        __VA_ARGS__; \
    } while(0)

//! @brief Macro to run Event System smoke tests
#define RUN_EVENT_SYSTEM_SMOKE_TESTS() \
    RUN_SMOKE_TEST_SUITE("Event System Smoke Tests", \
        RUN_TEST(test_event_dispatcher_initialization); \
        RUN_TEST(test_event_registration_and_dispatch); \
        RUN_TEST(test_event_processing_loop); \
        RUN_TEST(test_event_queue_overflow_handling); \
        RUN_TEST(test_multiple_callback_registration); \
    )

//! @brief Macro to run Time Service smoke tests
#define RUN_TIME_SERVICE_SMOKE_TESTS() \
    RUN_SMOKE_TEST_SUITE("Time Service Smoke Tests", \
        RUN_TEST(test_time_service_initialization); \
        RUN_TEST(test_timer_scheduling_and_cancellation); \
        RUN_TEST(test_timer_processing_loop); \
        RUN_TEST(test_repeating_timer_functionality); \
        RUN_TEST(test_one_shot_timer_functionality); \
        RUN_TEST(test_timer_overflow_handling); \
    )

//! @brief Macro to run BLE System smoke tests
#define RUN_BLE_SYSTEM_SMOKE_TESTS() \
    RUN_SMOKE_TEST_SUITE("BLE System Smoke Tests", \
        RUN_TEST(test_ble_driver_initialization); \
        RUN_TEST(test_ble_connection_state_management); \
        RUN_TEST(test_ble_message_callback_registration); \
        RUN_TEST(test_ble_message_sending_and_receiving); \
        RUN_TEST(test_ble_event_processing); \
    )

//! @brief Macro to run State Machine smoke tests
#define RUN_STATE_MACHINE_SMOKE_TESTS() \
    RUN_SMOKE_TEST_SUITE("State Machine Smoke Tests", \
        RUN_TEST(test_sensor_state_machine_initialization); \
        RUN_TEST(test_sensor_state_transitions); \
        RUN_TEST(test_sensor_session_management); \
        RUN_TEST(test_sensor_measurement_handling); \
        RUN_TEST(test_sensor_error_handling); \
    )

//! @brief Macro to run Integration smoke tests
#define RUN_INTEGRATION_SMOKE_TESTS() \
    RUN_SMOKE_TEST_SUITE("Integration Smoke Tests", \
        RUN_TEST(test_full_sensor_lifecycle); \
        RUN_TEST(test_sensor_broker_communication_flow); \
        RUN_TEST(test_measurement_broadcasting_flow); \
        RUN_TEST(test_session_start_stop_flow); \
        RUN_TEST(test_connection_loss_recovery); \
        RUN_TEST(test_error_recovery_flow); \
    )

//! @brief Macro to run Platform Mock smoke tests
#define RUN_PLATFORM_MOCK_SMOKE_TESTS() \
    RUN_SMOKE_TEST_SUITE("Platform Mock Smoke Tests", \
        RUN_TEST(test_native_time_driver_mock); \
        RUN_TEST(test_native_ble_driver_mock); \
        RUN_TEST(test_udp_ble_simulation); \
        RUN_TEST(test_mock_sensor_readings); \
        RUN_TEST(test_mock_broker_behavior); \
    )

#endif // SMOKE_TESTS_SMOKETESTSUITES_H_

