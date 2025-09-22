//! @file tests/TestSuites.h
//! @brief Test suite organization macros for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef TESTS_TESTSUITES_H_
#define TESTS_TESTSUITES_H_

#include "unity.h"

//! @brief Macro to run a group of tests with a suite name
#define RUN_TEST_SUITE(suite_name, ...) \
    do { \
        UNITY_PRINT_EOL(); \
        UNITY_PRINT(suite_name); \
        UNITY_PRINT_EOL(); \
        __VA_ARGS__; \
    } while(0)

//! @brief Macro to run GPIO tests
#define RUN_GPIO_TESTS() \
    RUN_TEST_SUITE("GPIO Tests", \
        RUN_TEST(test_digital_roundtrip); \
        RUN_TEST(test_analog_roundtrip); \
        RUN_TEST(test_resolution_forwarding); \
        RUN_TEST(test_typed_pin_construction); \
        RUN_TEST(test_typed_pin_conversion); \
        RUN_TEST(test_onewire_type_safety); \
        RUN_TEST(test_onewire_basic_operations); \
        RUN_TEST(test_pin_operations_through_typed_pin); \
        RUN_TEST(test_different_pin_types); \
        RUN_TEST(test_native_driver_roundtrip); \
        RUN_TEST(test_native_driver_analog); \
        RUN_TEST(test_voltage_levels_and_tmp36); \
        RUN_TEST(test_tmp36_conversion_10bit_5v); \
        RUN_TEST(test_tmp36_conversion_12bit_3v3); \
    )

//! @brief Macro to run BLE tests
#define RUN_BLE_TESTS() \
    RUN_TEST_SUITE("BLE Tests", \
        RUN_TEST(test_crc8_atm_known_vectors); \
        RUN_TEST(test_crc8_device_id_values); \
        RUN_TEST(test_crc8_error_detection); \
        RUN_TEST(test_crc8_varying_lengths); \
        RUN_TEST(test_crc8_edge_cases); \
        RUN_TEST(test_startbroadcast_serialization_roundtrip); \
        RUN_TEST(test_reading_serialization_roundtrip); \
        RUN_TEST(test_receipt_serialization_roundtrip); \
        RUN_TEST(test_ble_checksum_tamper_detection); \
        RUN_TEST(test_ble_point_to_point_delivery); \
        RUN_TEST(test_ble_broadcast_delivery_with_sender_id); \
        RUN_TEST(test_ble_multiple_broadcast_ordering); \
    )

//! @brief Macro to run BLE Callback tests
#define RUN_BLE_CALLBACK_TESTS() \
    RUN_TEST_SUITE("BLE Callback Tests", \
        RUN_TEST(test_type_specific_callback_registration); \
        RUN_TEST(test_start_broadcast_callback_routing); \
        RUN_TEST(test_reading_callback_routing); \
        RUN_TEST(test_receipt_callback_routing); \
        RUN_TEST(test_callback_priority_type_specific_over_generic); \
        RUN_TEST(test_fallback_to_generic_callback); \
        RUN_TEST(test_callback_clearing); \
        RUN_TEST(test_multiple_message_types_different_callbacks); \
        RUN_TEST(test_callback_with_invalid_message_data); \
        RUN_TEST(test_interface_contract_compliance); \
        RUN_TEST(test_sender_id_extraction); \
        RUN_TEST(test_callback_error_handling); \
        RUN_TEST(test_concurrent_callback_access); \
    )

//! @brief Macro to run BLE Interface Contract tests
#define RUN_BLE_INTERFACE_TESTS() \
    RUN_TEST_SUITE("BLE Interface Contract Tests", \
        RUN_TEST(test_driver_initialization_state); \
        RUN_TEST(test_driver_lifecycle_reinitialization); \
        RUN_TEST(test_messaging_round_trip_with_payload); \
        RUN_TEST(test_callback_invocation_on_message); \
        RUN_TEST(test_callback_not_invoked_after_clearing); \
        RUN_TEST(test_initial_driver_state); \
        RUN_TEST(test_driver_state_after_initialization); \
        RUN_TEST(test_driver_state_after_cleanup); \
        RUN_TEST(test_send_fails_when_not_initialized); \
        RUN_TEST(test_receive_fails_when_not_initialized); \
        RUN_TEST(test_concurrent_messaging); \
        RUN_TEST(test_messaging_with_zero_device_id); \
        RUN_TEST(test_messaging_with_max_device_id); \
    )

//! @brief Macro to run BLE Integration tests
#define RUN_BLE_INTEGRATION_TESTS() \
    RUN_TEST_SUITE("BLE Integration Tests", \
        RUN_TEST(test_ble_integration_full_roundtrip); \
        RUN_TEST(test_ble_integration_multiple_devices); \
        RUN_TEST(test_ble_integration_error_handling); \
        RUN_TEST(test_ble_integration_performance); \
    )

//! @brief Macro to run State Machine tests
#define RUN_STATE_MACHINE_TESTS() \
    RUN_TEST_SUITE("State Machine Tests", \
        RUN_TEST(test_state_machine_initialization); \
        RUN_TEST(test_sensor_disconnected_to_waiting_transition); \
        RUN_TEST(test_sensor_waiting_to_running_transition); \
        RUN_TEST(test_sensor_running_to_waiting_transition); \
        RUN_TEST(test_broker_no_session_to_session_started_transition); \
        RUN_TEST(test_broker_session_started_to_no_session_transition); \
        RUN_TEST(test_invalid_start_broadcast_while_disconnected); \
        RUN_TEST(test_state_entry_exit_actions); \
        RUN_TEST(test_start_broadcast_rejected_when_disconnected); \
        RUN_TEST(test_start_broadcast_accepted_when_waiting); \
        RUN_TEST(test_start_broadcast_device_id_validation); \
        RUN_TEST(test_state_machine_error_transition); \
        RUN_TEST(test_state_machine_error_recovery); \
    )

//! @brief Macro to run Time Driver tests
#define RUN_TIME_DRIVER_TESTS() \
    RUN_TEST_SUITE("Time Driver Tests", \
        RUN_TEST(test_driver_injection_native); \
        RUN_TEST(test_driver_no_op_behavior); \
        RUN_TEST(test_driver_switching); \
        RUN_TEST(test_driver_clear); \
    )

//! @brief Macro to run Measurement tests
#define RUN_MEASUREMENT_TESTS() \
    RUN_TEST_SUITE("Measurement Tests", \
        RUN_TEST(test_temperature_conversion); \
        RUN_TEST(test_humidity_conversion); \
        RUN_TEST(test_measurement_serialize); \
        RUN_TEST(test_measurement_deserialize); \
        RUN_TEST(test_measurement_roundtrip); \
        RUN_TEST(test_measurement_deserialize_errors); \
        RUN_TEST(test_payload_consumption); \
        RUN_TEST(test_conversion_boundaries); \
    )

#endif // TESTS_TESTSUITES_H_
