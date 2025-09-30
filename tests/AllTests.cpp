//! @file tests/AllTests.cpp
//! @brief All tests for the GPIO library.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <cstdint>

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
extern void test_crc8_atm_empty_data(void);
extern void test_crc8_atm_single_zero_byte(void);
extern void test_crc8_atm_test_pattern_12345678(void);
extern void test_crc8_atm_four_zeros(void);
extern void test_crc8_atm_four_ff(void);
extern void test_crc8_atm_alternating_aa55(void);
extern void test_crc8_device_id_0(void);
extern void test_crc8_device_id_1(void);
extern void test_crc8_device_id_7(void);
extern void test_crc8_device_id_255(void);
extern void test_crc8_device_id_256(void);
extern void test_crc8_device_id_0x12345678(void);
extern void test_crc8_device_id_max(void);
extern void test_crc8_detects_single_bit_flip_b0_bit0(void);
extern void test_crc8_detects_byte_swap_0_3(void);
extern void test_crc8_detects_adjacent_swap_1_2(void);
extern void test_crc8_length_1(void);
extern void test_crc8_length_2(void);
extern void test_crc8_length_3(void);
extern void test_crc8_length_4(void);
extern void test_crc8_length_5(void);
extern void test_crc8_length_6(void);
extern void test_crc8_length_7(void);
extern void test_crc8_length_8(void);
extern void test_crc8_zero_length_nullptr(void);
extern void test_crc8_zero_length_valid_buffer(void);
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
    RUN_TEST(test_crc8_atm_empty_data);
    RUN_TEST(test_crc8_atm_single_zero_byte);
    RUN_TEST(test_crc8_atm_test_pattern_12345678);
    RUN_TEST(test_crc8_atm_four_zeros);
    RUN_TEST(test_crc8_atm_four_ff);
    RUN_TEST(test_crc8_atm_alternating_aa55);
    RUN_TEST(test_crc8_device_id_0);
    RUN_TEST(test_crc8_device_id_1);
    RUN_TEST(test_crc8_device_id_7);
    RUN_TEST(test_crc8_device_id_255);
    RUN_TEST(test_crc8_device_id_256);
    RUN_TEST(test_crc8_device_id_0x12345678);
    RUN_TEST(test_crc8_device_id_max);
    RUN_TEST(test_crc8_detects_single_bit_flip_b0_bit0);
    RUN_TEST(test_crc8_detects_byte_swap_0_3);
    RUN_TEST(test_crc8_detects_adjacent_swap_1_2);
    RUN_TEST(test_crc8_length_1);
    RUN_TEST(test_crc8_length_2);
    RUN_TEST(test_crc8_length_3);
    RUN_TEST(test_crc8_length_4);
    RUN_TEST(test_crc8_length_5);
    RUN_TEST(test_crc8_length_6);
    RUN_TEST(test_crc8_length_7);
    RUN_TEST(test_crc8_length_8);
    RUN_TEST(test_crc8_zero_length_nullptr);
    RUN_TEST(test_crc8_zero_length_valid_buffer);
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

    // State Machine Tests
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

    run_measurement_tests();

    // Time Driver Tests
    RUN_TEST(test_driver_injection_native);
    RUN_TEST(test_driver_no_op_behavior);
    RUN_TEST(test_driver_switching);
    RUN_TEST(test_driver_clear);

    return UNITY_END();
}



