//! @file src/measurement/Measurement.cpp
//! @brief Implementation of Measurement serialization/deserialization.

#include "jenlib/measurement/Measurement.h"

namespace measurement {

bool Measurement::serialize(const Measurement &measurement, jenlib::ble::BlePayload &payload) {
    payload.clear();

    // Serialize timestamp (4 bytes, little-endian)
    if (!payload.append_u32le(measurement.timestamp_ms)) {
        return false;
    }

    // Convert and serialize temperature as centi-degrees (2 bytes, little-endian)
    const auto temp_centi = temperature_to_centi(measurement.temperature_c);
    if (!payload.append_i16le(temp_centi)) {
        return false;
    }

    // Convert and serialize humidity as basis points (2 bytes, little-endian)
    const auto humidity_bp = humidity_to_basis_points(measurement.humidity_bp);
    if (!payload.append_u16le(humidity_bp)) {
        return false;
    }

    return true;
}

bool Measurement::deserialize(jenlib::ble::BlePayload &&payload, Measurement &measurement) {
    // Expected payload size: 4 + 2 + 2 = 8 bytes
    constexpr std::size_t expected_size = 8;
    if (payload.size != expected_size) {
        return false;
    }

    auto it = payload.cbegin();
    const auto end = payload.cend();

    // Deserialize timestamp
    std::uint32_t timestamp_ms = 0;
    if (!jenlib::ble::read_u32le(it, end, timestamp_ms)) {
        return false;
    }

    // Deserialize temperature (centi-degrees to float)
    std::int16_t temp_centi = 0;
    if (!jenlib::ble::read_i16le(it, end, temp_centi)) {
        return false;
    }

    // Deserialize humidity (basis points to float)
    std::uint16_t humidity_bp = 0;
    if (!jenlib::ble::read_u16le(it, end, humidity_bp)) {
        return false;
    }

    // Verify we consumed all data
    if (it != end) {
        return false;
    }

    // Convert and assign to measurement
    measurement.timestamp_ms = timestamp_ms;
    measurement.temperature_c = temperature_from_centi(temp_centi);
    measurement.humidity_bp = humidity_from_basis_points(humidity_bp);

    // Consume the payload by clearing it
    payload.clear();

    return true;
}

} // namespace measurement

