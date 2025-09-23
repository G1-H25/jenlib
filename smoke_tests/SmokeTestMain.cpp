//! @file smoke_tests/SmokeTestMain.cpp
//! @brief Main smoke test runner for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "unity.h"
#include "SmokeTestSuites.h"

// Test function declarations from smoke test files
extern void test_event_dispatcher_initialization(void);
extern void test_event_registration_and_dispatch(void);
extern void test_event_processing_loop(void);
extern void test_event_queue_overflow_handling(void);
extern void test_multiple_callback_registration(void);

extern void test_time_service_initialization(void);
extern void test_timer_scheduling_and_cancellation(void);
extern void test_timer_processing_loop(void);
extern void test_repeating_timer_functionality(void);
extern void test_one_shot_timer_functionality(void);
extern void test_timer_overflow_handling(void);

extern void test_ble_driver_initialization(void);
extern void test_ble_connection_state_management(void);
extern void test_ble_message_callback_registration(void);
extern void test_ble_message_sending_and_receiving(void);
extern void test_ble_event_processing(void);

extern void test_sensor_state_machine_initialization(void);
extern void test_sensor_state_transitions(void);
extern void test_sensor_session_management(void);
extern void test_sensor_measurement_handling(void);
extern void test_sensor_error_handling(void);
extern void test_sensor_invalid_transitions(void);

extern void test_full_sensor_lifecycle(void);
extern void test_sensor_broker_communication_flow(void);
extern void test_measurement_broadcasting_flow(void);
extern void test_session_start_stop_flow(void);
extern void test_connection_loss_recovery(void);
extern void test_error_recovery_flow(void);

extern void test_native_time_driver_mock(void);
extern void test_native_ble_driver_mock(void);
extern void test_udp_ble_simulation(void);
extern void test_mock_sensor_readings(void);
extern void test_mock_broker_behavior(void);

void setUp(void) {
    // Global setup for smoke tests
}

void tearDown(void) {
    // Global cleanup for smoke tests
}

int main() {
    UNITY_BEGIN();

    // Run Event System Smoke Tests
    RUN_EVENT_SYSTEM_SMOKE_TESTS();

    // Run Time Service Smoke Tests
    RUN_TIME_SERVICE_SMOKE_TESTS();

    // Run BLE System Smoke Tests
    RUN_BLE_SYSTEM_SMOKE_TESTS();

    // Run State Machine Smoke Tests
    RUN_STATE_MACHINE_SMOKE_TESTS();

    // Run Integration Smoke Tests
    RUN_INTEGRATION_SMOKE_TESTS();

    // Run Platform Mock Smoke Tests
    RUN_PLATFORM_MOCK_SMOKE_TESTS();

    return UNITY_END();
}

