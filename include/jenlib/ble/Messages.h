//! @file include/jenlib/ble/Messages.h
//! @brief BLE-friendly message formats and serialization utilities.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_BLE_MESSAGES_H_
#define INCLUDE_JENLIB_BLE_MESSAGES_H_

#include <cstdint>
#include "jenlib/ble/Payload.h"
#include "jenlib/ble/Ids.h"

namespace jenlib::ble {

//! @brief BLE message types used over advertisements or GATT values.
//!
//! The library uses compact, BLE-friendly binary layouts. Identifiers are
//! strongly typed ("DeviceId", "SessionId"). "DeviceId" is serialized with an
//! embedded CRC-8 for quick integrity checks without requiring a full-message
//! checksum. Numeric fields are little-endian. Temperature and humidity are
//! scaled to fixed-point units for portability.
enum class MessageType : std::uint8_t {
    StartBroadcast = 0x01,
    Reading        = 0x02,
    Receipt        = 0x03,
};

//! @brief Broker to Sensor command to begin a measurement session.
//!
//! The broker chooses a new "SessionId" and targets a specific sensor
//! "DeviceId". Targeting should be based on messages from backend server.
//!  Sensors store the session and begin broadcasting readings.
struct StartBroadcastMsg {
    DeviceId device_id;    //!< target sensor id
    SessionId session_id;  //!< session identifier

    static bool serialize(const StartBroadcastMsg &msg, BlePayload &out);
    static bool deserialize(const BlePayload &buf, StartBroadcastMsg &out);
};

//! @brief Sensor to Broker measurement payload.
//!
//! Temperature is encoded in centi-degrees Celsius (e.g., 2312 => 23.12 C).
//! Humidity is encoded in basis points (0..10000 => 0%..100.00%). The
//! "offset_ms" is the elapsed time since the start message was accepted.
struct ReadingMsg {
    DeviceId sender_id;            //!< sensor id
    SessionId session_id;          //!< session identifier
    std::uint32_t offset_ms;       //!< time from start in milliseconds
    std::int16_t temperature_c_centi;  //!< temperature in centi-degrees C
    std::uint16_t humidity_bp;  //!< humidity in basis points (0..10000)

    static bool serialize(const ReadingMsg &msg, BlePayload &out);
    static bool deserialize(const BlePayload &buf, ReadingMsg &out);
};

//! @brief Broker to Sensor acknowledgement of received readings.
//!
//! Allows a sensor to purge buffered readings up to (and including)
//! "up_to_offset_ms" within the current "SessionId".
struct ReceiptMsg {
    SessionId session_id; //!< session identifier
    std::uint32_t up_to_offset_ms; //!< ack up to (inclusive)

    static bool serialize(const ReceiptMsg &msg, BlePayload &out);
    static bool deserialize(const BlePayload &buf, ReceiptMsg &out);
};

} // namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_MESSAGES_H_
