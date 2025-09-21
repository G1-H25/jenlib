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
extern void test_messaging_with_normal_device_id(void);
extern void test_callback_parameters_passed_correctly(void);
extern void test_initialization_return_value_consistency(void);
extern void test_connection_state_return_value_consistency(void);
extern void test_device_id_return_value_consistency(void);

// BLE Integration Tests
extern void test_complete_sensor_broker_communication_flow(void);
extern void test_multiple_sensors_single_broker(void);
extern void test_session_management_and_cleanup(void);
extern void test_callback_performance_under_load(void);
extern void test_callback_reliability_with_message_loss(void);
extern void test_callback_error_recovery(void);
extern void test_callback_with_mixed_message_types(void);
extern void test_callback_with_concurrent_access(void);
extern void test_ble_receipt_acknowledgment_flow(void);
extern void test_ble_multiple_broadcast_ordering(void);
extern void run_measurement_tests(void);

void setUp(void) {}
void tearDown(void) {}

int main() {
    UNITY_BEGIN();
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
    RUN_TEST(test_ble_receipt_acknowledgment_flow);
    RUN_TEST(test_ble_multiple_broadcast_ordering);
    
    // BLE Callback Tests - TODO: Implement these tests
    // RUN_TEST(test_type_specific_callback_registration);
    // RUN_TEST(test_start_broadcast_callback_routing);
    // RUN_TEST(test_reading_callback_routing);
    // RUN_TEST(test_receipt_callback_routing);
    // RUN_TEST(test_callback_priority_type_specific_over_generic);
    // RUN_TEST(test_fallback_to_generic_callback);
    // RUN_TEST(test_callback_clearing);
    // RUN_TEST(test_multiple_message_types_different_callbacks);
    // RUN_TEST(test_callback_with_invalid_message_data);
    // RUN_TEST(test_interface_contract_compliance);
    // RUN_TEST(test_sender_id_extraction);
    // RUN_TEST(test_callback_error_handling);
    // RUN_TEST(test_concurrent_callback_access);
    
    // BLE Interface Contract Tests - TODO: Implement these tests
    // RUN_TEST(test_driver_initialization_state);
    // RUN_TEST(test_driver_lifecycle_reinitialization);
    // RUN_TEST(test_messaging_round_trip_with_payload);
    // RUN_TEST(test_callback_invocation_on_message);
    // RUN_TEST(test_callback_not_invoked_after_clearing);
    // RUN_TEST(test_initial_driver_state);
    // RUN_TEST(test_driver_state_after_initialization);
    // RUN_TEST(test_driver_state_after_cleanup);
    // RUN_TEST(test_send_fails_when_not_initialized);
    // RUN_TEST(test_receive_fails_when_not_initialized);
    // RUN_TEST(test_concurrent_messaging);
    // RUN_TEST(test_messaging_with_zero_device_id);
    // RUN_TEST(test_messaging_with_max_device_id);
    // RUN_TEST(test_messaging_with_normal_device_id);
    // RUN_TEST(test_callback_parameters_passed_correctly);
    // RUN_TEST(test_initialization_return_value_consistency);
    // RUN_TEST(test_connection_state_return_value_consistency);
    // RUN_TEST(test_device_id_return_value_consistency);
    
    // BLE Integration Tests - TODO: Implement these tests
    // RUN_TEST(test_complete_sensor_broker_communication_flow);
    // RUN_TEST(test_multiple_sensors_single_broker);
    // RUN_TEST(test_session_management_and_cleanup);
    // RUN_TEST(test_callback_performance_under_load);
    // RUN_TEST(test_callback_reliability_with_message_loss);
    // RUN_TEST(test_callback_error_recovery);
    // RUN_TEST(test_callback_with_mixed_message_types);
    // RUN_TEST(test_callback_with_concurrent_access);
    
    run_measurement_tests();
    return UNITY_END();
}


