
//! @file tests/MeasurementTests.cpp
//! @brief Unit tests for Measurement serialization/deserialization.
//! @test Serialization and deserialization of Measurement structs to/from BLE payloads.
//! @test Unit conversion functions for temperature and humidity.

#include <unity.h>
#include <utility>
#include "jenlib/measurement/Measurement.h"

namespace {

//! @test Test temperature conversion functions.
void test_temperature_conversion() {
    // Test basic conversion
    TEST_ASSERT_EQUAL_INT16(2312, measurement::temperature_to_centi(23.12f));
    TEST_ASSERT_EQUAL_INT16(-500, measurement::temperature_to_centi(-5.0f));
    TEST_ASSERT_EQUAL_INT16(0, measurement::temperature_to_centi(0.0f));

    // Test rounding
    TEST_ASSERT_EQUAL_INT16(2313, measurement::temperature_to_centi(23.125f));
    TEST_ASSERT_EQUAL_INT16(2312, measurement::temperature_to_centi(23.124f));

    // Test reverse conversion
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 23.12f, measurement::temperature_from_centi(2312));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -5.0f, measurement::temperature_from_centi(-500));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, measurement::temperature_from_centi(0));
}

//! @test Test humidity conversion functions.
void test_humidity_conversion() {
    // Test basic conversion
    TEST_ASSERT_EQUAL_UINT16(4567, measurement::humidity_to_basis_points(45.67f));
    TEST_ASSERT_EQUAL_UINT16(10000, measurement::humidity_to_basis_points(100.0f));
    TEST_ASSERT_EQUAL_UINT16(0, measurement::humidity_to_basis_points(0.0f));

    // Test rounding
    TEST_ASSERT_EQUAL_UINT16(4568, measurement::humidity_to_basis_points(45.675f));
    TEST_ASSERT_EQUAL_UINT16(4567, measurement::humidity_to_basis_points(45.674f));

    // Test reverse conversion
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 45.67f, measurement::humidity_from_basis_points(4567));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 100.0f, measurement::humidity_from_basis_points(10000));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, measurement::humidity_from_basis_points(0));
}

//! @test Test serialization of a complete measurement.
void test_measurement_serialize() {
    measurement::Measurement test_measurement;
    test_measurement.timestamp_ms = 12345678U;
    test_measurement.temperature_c = 23.45f;
    test_measurement.humidity_bp = 67.89f;

    jenlib::ble::BlePayload payload;
    TEST_ASSERT_TRUE(measurement::Measurement::serialize(test_measurement, payload));

    // Verify payload size
    TEST_ASSERT_EQUAL_UINT32(8, payload.size);

    // Verify payload contents manually
    auto it = payload.cbegin();
    const auto end = payload.cend();

    // Check timestamp (little-endian)
    std::uint32_t timestamp = 0;
    TEST_ASSERT_TRUE(jenlib::ble::read_u32le(it, end, timestamp));
    TEST_ASSERT_EQUAL_UINT32(12345678U, timestamp);

    // Check temperature (little-endian)
    std::int16_t temp_centi = 0;
    TEST_ASSERT_TRUE(jenlib::ble::read_i16le(it, end, temp_centi));
    TEST_ASSERT_EQUAL_INT16(2345, temp_centi);  // 23.45 * 100

    // Check humidity (little-endian)
    std::uint16_t humidity_bp = 0;
    TEST_ASSERT_TRUE(jenlib::ble::read_u16le(it, end, humidity_bp));
    TEST_ASSERT_EQUAL_UINT16(6789, humidity_bp);  // 67.89 * 100

    // Should have consumed all data
    TEST_ASSERT_TRUE(it == end);
}

//! @test Test deserialization of a complete measurement.
void test_measurement_deserialize() {
    // Create a payload manually
    jenlib::ble::BlePayload payload;
    payload.append_u32le(87654321U);  // timestamp
    payload.append_i16le(-1234);      // temperature (-12.34°C)
    payload.append_u16le(3456);       // humidity (34.56%)

    measurement::Measurement result;
    TEST_ASSERT_TRUE(measurement::Measurement::deserialize(std::move(payload), result));

    // Verify deserialized values
    TEST_ASSERT_EQUAL_UINT32(87654321U, result.timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -12.34f, result.temperature_c);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 34.56f, result.humidity_bp);

    // Verify payload was consumed
    TEST_ASSERT_TRUE(payload.is_consumed());
}

//! @test Test round-trip serialization (serialize then deserialize).
void test_measurement_roundtrip() {
    measurement::Measurement original;
    original.timestamp_ms = 987654321U;
    original.temperature_c = -45.67f;
    original.humidity_bp = 89.12f;

    jenlib::ble::BlePayload payload;
    TEST_ASSERT_TRUE(measurement::Measurement::serialize(original, payload));

    measurement::Measurement restored;
    TEST_ASSERT_TRUE(measurement::Measurement::deserialize(std::move(payload), restored));

    // Verify round-trip accuracy
    TEST_ASSERT_EQUAL_UINT32(original.timestamp_ms, restored.timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original.temperature_c, restored.temperature_c);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original.humidity_bp, restored.humidity_bp);

    // Verify payload was consumed
    TEST_ASSERT_TRUE(payload.is_consumed());
}

//! @test Test error handling for invalid payload sizes.
void test_measurement_deserialize_errors() {
    measurement::Measurement result;

    // Test empty payload
    jenlib::ble::BlePayload empty_payload;
    TEST_ASSERT_FALSE(measurement::Measurement::deserialize(std::move(empty_payload), result));

    // Test too small payload
    jenlib::ble::BlePayload small_payload;
    small_payload.append_u32le(12345U);
    TEST_ASSERT_FALSE(measurement::Measurement::deserialize(std::move(small_payload), result));

    // Test too large payload
    jenlib::ble::BlePayload large_payload;
    large_payload.append_u32le(12345U);
    large_payload.append_i16le(2345);
    large_payload.append_u16le(3456);
    large_payload.append_u8(0xFF);  // Extra byte
    TEST_ASSERT_FALSE(measurement::Measurement::deserialize(std::move(large_payload), result));
}

//! @test Test payload consumption behavior.
void test_payload_consumption() {
    measurement::Measurement original;
    original.timestamp_ms = 12345678U;
    original.temperature_c = 25.5f;
    original.humidity_bp = 60.0f;

    // Serialize measurement
    jenlib::ble::BlePayload payload;
    TEST_ASSERT_TRUE(measurement::Measurement::serialize(original, payload));
    TEST_ASSERT_FALSE(payload.is_consumed());  // Should not be consumed yet

    // Deserialize (should consume the payload)
    measurement::Measurement result;
    TEST_ASSERT_TRUE(measurement::Measurement::deserialize(std::move(payload), result));
    TEST_ASSERT_TRUE(payload.is_consumed());  // Should be consumed now

    // Verify data integrity
    TEST_ASSERT_EQUAL_UINT32(original.timestamp_ms, result.timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original.temperature_c, result.temperature_c);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, original.humidity_bp, result.humidity_bp);

    // Verify payload is truly empty
    TEST_ASSERT_EQUAL_UINT32(0, payload.size);
}

//! @test Test boundary values for conversions.
void test_conversion_boundaries() {
    // Test temperature boundaries
    TEST_ASSERT_EQUAL_INT16(32767, measurement::temperature_to_centi(327.67f));
    TEST_ASSERT_EQUAL_INT16(-32768, measurement::temperature_to_centi(-327.68f));

    // Test humidity boundaries
    TEST_ASSERT_EQUAL_UINT16(0, measurement::humidity_to_basis_points(0.0f));
    TEST_ASSERT_EQUAL_UINT16(10000, measurement::humidity_to_basis_points(100.0f));
    TEST_ASSERT_EQUAL_UINT16(65535, measurement::humidity_to_basis_points(655.35f));
}

}  // anonymous namespace

//! @brief Run all Measurement tests.
void run_measurement_tests() {
    RUN_TEST(test_temperature_conversion);
    RUN_TEST(test_humidity_conversion);
    RUN_TEST(test_measurement_serialize);
    RUN_TEST(test_measurement_deserialize);
    RUN_TEST(test_measurement_roundtrip);
    RUN_TEST(test_measurement_deserialize_errors);
    RUN_TEST(test_payload_consumption);
    RUN_TEST(test_conversion_boundaries);
}

