//! @file include/jenlib/ble/BleDriver.h
//! @brief BLE driver interface for sensor/broker communication.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_BLE_BLEDRIVER_H_
#define INCLUDE_JENLIB_BLE_BLEDRIVER_H_

#include <cstdint>
#include "jenlib/ble/Payload.h"
#include "jenlib/ble/Ids.h"

namespace ble {

//! @brief Abstract transport for BLE messaging in this library.
//! @details
//! This interface abstracts how messages are carried (advertisements,
//! connections/GATT, native simulation). It is intentionally minimal:
//! - advertise(): one-to-many, best-effort (e.g., ADV or notify to broker)
//! - send_to(): point-to-point, directed to a specific DeviceId
//! - receive(): polling API to fetch next inbound payload for a DeviceId
//!
//! @note
//! - All payloads are compact, little-endian, produced by serializers in
//!   Messages.h. Drivers must not mutate payload bytes.
//! - Implementations should be non-blocking and avoid dynamic allocation
//!   where practical (Arduino targets).
//! - When emulating broadcasts in a native driver, reserve DeviceId(0) as
//!   a logical broker inbox.
class BleDriver {
 public:
    virtual ~BleDriver() = default;

    //! @brief Send a best-effort broadcast from a device.
    //! @param device_id Logical sender identity.
    //! @param payload Serialized message bytes (moved into driver).
    virtual void advertise(DeviceId device_id, BlePayload payload) = 0;

    //! @brief Send a directed, point-to-point message.
    //! @param device_id Destination identity.
    //! @param payload Serialized message bytes (moved into driver).
    virtual void send_to(DeviceId device_id, BlePayload payload) = 0;

    //! @brief Poll next received payload for a local device.
    //! @param self_id Local identity being polled.
    //! @param out_payload Destination buffer for the payload.
    //! @return True if a payload was returned, false if none available.
    virtual bool receive(DeviceId self_id, BlePayload &out_payload) = 0;
};

} // namespace ble

#endif  // INCLUDE_JENLIB_BLE_BLEDRIVER_H_
