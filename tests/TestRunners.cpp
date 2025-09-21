//! @file tests/TestRunners.cpp
//! @brief Test runner implementations for organizing tests by module
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "TestRunners.h"
#include "unity.h"

// Forward declarations for all test functions
// GPIO Tests
extern void test_digital_roundtrip(void);
extern void test_analog_roundtrip(void);
extern void test_resolution_forwarding(void);
extern void test_typed_pin_construction(void);
extern void test_typed_pin_conversion(void);
extern void test_onewire_type_safety(void);
extern void test_onewire_basic_operations(void);
extern void test_pin_operations_through_typed_pin(void);
extern void test_different_pin_types(void);
extern void test_native_driver_roundtrip(void);
extern void test_native_driver_analog(void);
extern void test_voltage_levels_and_tmp36(void);
extern void test_tmp36_conversion_10bit_5v(void);
extern void test_tmp36_conversion_12bit_3v3(void);

// BLE Tests
extern void test_crc8_atm_known_vectors(void);
extern void test_crc8_device_id_values(void);
extern void test_crc8_error_detection(void);
extern void test_crc8_varying_lengths(void);
extern void test_crc8_edge_cases(void);
extern void test_startbroadcast_serialization_roundtrip(void);
extern void test_reading_serialization_roundtrip(void);
extern void test_receipt_serialization_roundtrip(void);
extern void test_ble_checksum_tamper_detection(void);
extern void test_ble_point_to_point_delivery(void);
extern void test_ble_broadcast_delivery_with_sender_id(void);
extern void test_ble_multiple_broadcast_ordering(void);

// BLE Callback Tests
extern void test_type_specific_callback_registration(void);
extern void test_start_broadcast_callback_routing(void);
extern void test_reading_callback_routing(void);
extern void test_receipt_callback_routing(void);
extern void test_callback_priority_type_specific_over_generic(void);
extern void test_fallback_to_generic_callback(void);
extern void test_callback_clearing(void);
extern void test_multiple_message_types_different_callbacks(void);
extern void test_callback_with_invalid_message_data(void);
extern void test_interface_contract_compliance(void);
extern void test_sender_id_extraction(void);
extern void test_callback_error_handling(void);
extern void test_concurrent_callback_access(void);

// BLE Interface Contract Tests
extern void test_driver_initialization_state(void);
extern void test_driver_lifecycle_reinitialization(void);
extern void test_messaging_round_trip_with_payload(void);
extern void test_callback_invocation_on_message(void);
extern void test_callback_not_invoked_after_clearing(void);
extern void test_initial_driver_state(void);
extern void test_driver_state_after_initialization(void);
extern void test_driver_state_after_cleanup(void);
extern void test_send_fails_when_not_initialized(void);
extern void test_receive_fails_when_not_initialized(void);
extern void test_concurrent_messaging(void);
extern void test_messaging_with_zero_device_id(void);
extern void test_messaging_with_max_device_id(void);

// BLE Integration Tests
extern void test_ble_integration_full_roundtrip(void);
extern void test_ble_integration_multiple_devices(void);
extern void test_ble_integration_error_handling(void);
extern void test_ble_integration_performance(void);

// State Machine Tests
extern void test_state_machine_initialization(void);
extern void test_sensor_disconnected_to_waiting_transition(void);
extern void test_sensor_waiting_to_running_transition(void);
extern void test_sensor_running_to_waiting_transition(void);
extern void test_broker_no_session_to_session_started_transition(void);
extern void test_broker_session_started_to_no_session_transition(void);
extern void test_invalid_start_broadcast_while_disconnected(void);
extern void test_state_entry_exit_actions(void);
extern void test_start_broadcast_rejected_when_disconnected(void);
extern void test_start_broadcast_accepted_when_waiting(void);
extern void test_start_broadcast_device_id_validation(void);
extern void test_state_machine_error_transition(void);
extern void test_state_machine_error_recovery(void);

// Time Driver Tests
extern void test_driver_injection_native(void);
extern void test_driver_no_op_behavior(void);
extern void test_driver_switching(void);
extern void test_driver_clear(void);

// Measurement Tests
extern void test_temperature_conversion(void);
extern void test_humidity_conversion(void);
extern void test_measurement_serialize(void);
extern void test_measurement_deserialize(void);
extern void test_measurement_roundtrip(void);
extern void test_measurement_deserialize_errors(void);
extern void test_payload_consumption(void);
extern void test_conversion_boundaries(void);

void run_gpio_tests(void) {
    RUN_TEST(test_digital_roundtrip);
    RUN_TEST(test_analog_roundtrip);
    RUN_TEST(test_resolution_forwarding);
    RUN_TEST(test_typed_pin_construction);
    RUN_TEST(test_typed_pin_conversion);
    RUN_TEST(test_onewire_type_safety);
    RUN_TEST(test_onewire_basic_operations);
    RUN_TEST(test_pin_operations_through_typed_pin);
    RUN_TEST(test_different_pin_types);
    RUN_TEST(test_native_driver_roundtrip);
    RUN_TEST(test_native_driver_analog);
    RUN_TEST(test_voltage_levels_and_tmp36);
    RUN_TEST(test_tmp36_conversion_10bit_5v);
    RUN_TEST(test_tmp36_conversion_12bit_3v3);
}

void run_ble_tests(void) {
    RUN_TEST(test_crc8_atm_known_vectors);
    RUN_TEST(test_crc8_device_id_values);
    RUN_TEST(test_crc8_error_detection);
    RUN_TEST(test_crc8_varying_lengths);
    RUN_TEST(test_crc8_edge_cases);
    RUN_TEST(test_startbroadcast_serialization_roundtrip);
    RUN_TEST(test_reading_serialization_roundtrip);
    RUN_TEST(test_receipt_serialization_roundtrip);
    RUN_TEST(test_ble_checksum_tamper_detection);
    RUN_TEST(test_ble_point_to_point_delivery);
    RUN_TEST(test_ble_broadcast_delivery_with_sender_id);
    RUN_TEST(test_ble_multiple_broadcast_ordering);
}

void run_ble_callback_tests(void) {
    RUN_TEST(test_type_specific_callback_registration);
    RUN_TEST(test_start_broadcast_callback_routing);
    RUN_TEST(test_reading_callback_routing);
    RUN_TEST(test_receipt_callback_routing);
    RUN_TEST(test_callback_priority_type_specific_over_generic);
    RUN_TEST(test_fallback_to_generic_callback);
    RUN_TEST(test_callback_clearing);
    RUN_TEST(test_multiple_message_types_different_callbacks);
    RUN_TEST(test_callback_with_invalid_message_data);
    RUN_TEST(test_interface_contract_compliance);
    RUN_TEST(test_sender_id_extraction);
    RUN_TEST(test_callback_error_handling);
    RUN_TEST(test_concurrent_callback_access);
}

void run_ble_interface_tests(void) {
    RUN_TEST(test_driver_initialization_state);
    RUN_TEST(test_driver_lifecycle_reinitialization);
    RUN_TEST(test_messaging_round_trip_with_payload);
    RUN_TEST(test_callback_invocation_on_message);
    RUN_TEST(test_callback_not_invoked_after_clearing);
    RUN_TEST(test_initial_driver_state);
    RUN_TEST(test_driver_state_after_initialization);
    RUN_TEST(test_driver_state_after_cleanup);
    RUN_TEST(test_send_fails_when_not_initialized);
    RUN_TEST(test_receive_fails_when_not_initialized);
    RUN_TEST(test_concurrent_messaging);
    RUN_TEST(test_messaging_with_zero_device_id);
    RUN_TEST(test_messaging_with_max_device_id);
}

void run_ble_integration_tests(void) {
    RUN_TEST(test_ble_integration_full_roundtrip);
    RUN_TEST(test_ble_integration_multiple_devices);
    RUN_TEST(test_ble_integration_error_handling);
    RUN_TEST(test_ble_integration_performance);
}

void run_state_machine_tests(void) {
    RUN_TEST(test_state_machine_initialization);
    RUN_TEST(test_sensor_disconnected_to_waiting_transition);
    RUN_TEST(test_sensor_waiting_to_running_transition);
    RUN_TEST(test_sensor_running_to_waiting_transition);
    RUN_TEST(test_broker_no_session_to_session_started_transition);
    RUN_TEST(test_broker_session_started_to_no_session_transition);
    RUN_TEST(test_invalid_start_broadcast_while_disconnected);
    RUN_TEST(test_state_entry_exit_actions);
    RUN_TEST(test_start_broadcast_rejected_when_disconnected);
    RUN_TEST(test_start_broadcast_accepted_when_waiting);
    RUN_TEST(test_start_broadcast_device_id_validation);
    RUN_TEST(test_state_machine_error_transition);
    RUN_TEST(test_state_machine_error_recovery);
}

void run_time_driver_tests(void) {
    RUN_TEST(test_driver_injection_native);
    RUN_TEST(test_driver_no_op_behavior);
    RUN_TEST(test_driver_switching);
    RUN_TEST(test_driver_clear);
}

// run_measurement_tests() is already defined in MeasurementTests.cpp
