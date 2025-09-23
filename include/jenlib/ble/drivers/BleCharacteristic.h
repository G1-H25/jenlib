//! @file include/jenlib/ble/drivers/BleCharacteristic.h
//! @brief Agnostic BLE characteristic interface for cross-platform compatibility.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_BLE_DRIVERS_BLECHARACTERISTIC_H_
#define INCLUDE_JENLIB_BLE_DRIVERS_BLECHARACTERISTIC_H_

#include <cstdint>
#include <functional>
#include <jenlib/ble/Payload.h>

namespace jenlib::ble {

//! @brief BLE characteristic properties.
enum class BleCharacteristicProperty : std::uint8_t {
    Read = 0x01,
    Write = 0x02,
    Notify = 0x04,
    Indicate = 0x08,
    WriteWithoutResponse = 0x10
};

//! @brief BLE characteristic event types.
enum class BleCharacteristicEvent : std::uint8_t {
    Written = 0x01,
    Subscribed = 0x02,
    Unsubscribed = 0x04
};

//! @brief Callback function type for characteristic events.
//! @param event The event type that occurred.
//! @param payload The payload data associated with the event.
using BleCharacteristicCallback = std::function<void(BleCharacteristicEvent event, const BlePayload& payload)>;

//! @brief Abstract BLE characteristic interface.
//! @details
//! Platform-agnostic interface for BLE characteristics.
class BleCharacteristic {
 public:
    virtual ~BleCharacteristic() = default;

    //! @brief Write data to the characteristic.
    //! @param payload The data to write.
    //! @return true if the write was successful, false otherwise.
    virtual bool write_value(const BlePayload& payload) = 0;

    //! @brief Read data from the characteristic.
    //! @param out_payload Buffer to store the read data.
    //! @return true if the read was successful, false otherwise.
    virtual bool read_value(BlePayload& out_payload) const = 0;

    //! @brief Set the event callback for this characteristic.
    //! @param callback Function to call when events occur.
    virtual void set_event_callback(BleCharacteristicCallback callback) = 0;

    //! @brief Get the characteristic properties.
    //! @return Bitmask of BleCharacteristicProperty values.
    virtual std::uint8_t get_properties() const = 0;

    //! @brief Check if the characteristic has a specific property.
    //! @param property The property to check.
    //! @return true if the characteristic has the property, false otherwise.
    bool has_property(BleCharacteristicProperty property) const {
        return (get_properties() & static_cast<std::uint8_t>(property)) != 0;
    }

    //! @brief Get the maximum payload size for this characteristic.
    //! @return Maximum number of bytes that can be written/read.
    virtual std::size_t get_max_payload_size() const = 0;
};

} // namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_DRIVERS_BLECHARACTERISTIC_H_

