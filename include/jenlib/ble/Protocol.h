//! @file include/jenlib/ble/Protocol.h
//! @brief Protocol-level definitions for BLE sensor/broker communication.
//! @details
//! Defines roles, opcodes, versioning, and basic wire constraints for the
//! jenlib BLE protocol. Transport specifics (GATT UUIDs) live in GattProfile.h,
//! while payload layouts live in Messages.h.

#ifndef INCLUDE_JENLIB_BLE_PROTOCOL_H_
#define INCLUDE_JENLIB_BLE_PROTOCOL_H_

#include <cstdint>
#include <string_view>
#include <utility>

namespace jenlib::ble::protocol {

//! @brief Protocol version (semantic-style: major.minor)
inline constexpr std::uint8_t kVersionMajor = 1;
inline constexpr std::uint8_t kVersionMinor = 0;

//! @brief Device role in the protocol.
enum class Role : std::uint8_t {
    Sensor = 0x01,  //!< Produces readings, receives control/receipt
    Broker = 0x02   //!< Issues control/receipts, receives readings
};

//! @brief Message operation codes (first byte of payload in Messages.h)
enum class OpCode : std::uint8_t {
    StartBroadcast = 0x01, //!< Broker→Sensor: start a session
    Reading        = 0x02, //!< Sensor→Broker: a measurement reading
    Receipt        = 0x03  //!< Broker→Sensor: receipt/ack for readings
};

//! @brief Payload limits and timing guidance
namespace limits {
    //! @brief Maximum payload size in bytes for a single message value
    //! @note Matches kMaxPayload in Payload.h to keep protocol consistent.
    inline constexpr std::size_t kMaxPayloadBytes = 64u;

    //! @brief Recommended notify interval in milliseconds for readings
    inline constexpr std::uint32_t kRecommendedReadingIntervalMs = 1000u;
}

//! @brief High-level contract summary
namespace contract {
    //! @brief Directionality summary for each opcode
    inline constexpr bool kStartBroadcastBrokerToSensor = true;
    inline constexpr bool kReadingSensorToBroker = true;
    inline constexpr bool kReceiptBrokerToSensor = true;
}

}  // namespace jenlib::ble::protocol

#endif // INCLUDE_JENLIB_BLE_PROTOCOL_H_



