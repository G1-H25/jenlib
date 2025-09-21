#ifndef INCLUDE_JENLIB_MEASUREMENT_MEASUREMENT_H_
#define INCLUDE_JENLIB_MEASUREMENT_MEASUREMENT_H_

#include <cstdint>
#include "jenlib/ble/Payload.h"


//! @brief Shared common data type for the measurements sensors record and broker collects.
//! As data is collected, it is stored in this format. Then it is serialized to a BLE message for transmission.
//! Broker deserializes into this format for aggregation and json serialization for backend transfer.

//! @namespace measurement
//! @brief Namespace for measurement types.
namespace measurement {

struct Measurement {
    std::uint32_t timestamp_ms; //!< Offset since start of session in milliseconds.
    float temperature_c; //!< Temperature in Celsius.
    float humidity_bp; //!< Humidity in percentage.

    //! @brief Serialize a Measurement to a BLE payload.
    //! @param measurement The measurement to serialize.
    //! @param payload The output payload buffer.
    //! @return true if serialization succeeded, false if payload buffer is too small.
    static bool serialize(const Measurement &measurement, jenlib::ble::BlePayload &payload);

    //! @brief Deserialize a Measurement from a BLE payload (consumes the payload).
    //! @param payload The input payload buffer (will be consumed/moved from).
    //! @param measurement The output measurement.
    //! @return true if deserialization succeeded, false if payload format is invalid.
    //! @note The payload will be consumed (size set to 0) after successful deserialization.
    static bool deserialize(jenlib::ble::BlePayload &&payload, Measurement &measurement);
};

//! @brief Convert temperature from float Celsius to centi-degrees (int16).
//! @param temp_c Temperature in Celsius.
//! @return Temperature in centi-degrees (e.g., 23.12°C -> 2312).
inline std::int16_t temperature_to_centi(float temp_c) {
    if (temp_c >= 0.0f) {
        return static_cast<std::int16_t>(temp_c * 100.0f + 0.5f);
    } else {
        return static_cast<std::int16_t>(temp_c * 100.0f - 0.5f);
    }
}

//! @brief Convert temperature from centi-degrees (int16) to float Celsius.
//! @param temp_centi Temperature in centi-degrees.
//! @return Temperature in Celsius.
inline float temperature_from_centi(std::int16_t temp_centi) {
    return static_cast<float>(temp_centi) / 100.0f;
}

//! @brief Convert humidity from float percentage to basis points (uint16).
//! @param humidity_bp Humidity as percentage (0.0-100.0).
//! @return Humidity in basis points (0-10000).
inline std::uint16_t humidity_to_basis_points(float humidity_bp) {
    return static_cast<std::uint16_t>(humidity_bp * 100.0f + 0.5f);
}

//! @brief Convert humidity from basis points (uint16) to float percentage.
//! @param humidity_bp Humidity in basis points (0-10000).
//! @return Humidity as percentage (0.0-100.0).
inline float humidity_from_basis_points(std::uint16_t humidity_bp) {
    return static_cast<float>(humidity_bp) / 100.0f;
}

} // namespace measurement

#endif // INCLUDE_JENLIB_MEASUREMENT_MEASUREMENT_H_