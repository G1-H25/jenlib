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
extern void test_ble_receipt_acknowledgment_flow(void);
extern void test_ble_multiple_broadcast_ordering(void);

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
    return UNITY_END();
}


