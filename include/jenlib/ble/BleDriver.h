//! @file include/jenlib/ble/BleDriver.h
//! @brief BLE driver interface for sensor/broker communication.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_BLE_BLEDRIVER_H_
#define INCLUDE_JENLIB_BLE_BLEDRIVER_H_

#include <cstdint> //!< For uint8_t
#include <functional> //!< For function callbacks
#include "jenlib/ble/Payload.h"
#include "jenlib/ble/Ids.h"

namespace ble {

// Forward declarations
struct StartBroadcastMsg;
struct ReadingMsg;
struct ReceiptMsg;

//! @brief Callback function type for received BLE messages.
//! @param sender_id The device ID that sent the message.
//! @param payload The received message payload.
using BleMessageCallback = std::function<void(DeviceId sender_id, const BlePayload& payload)>;

//! @brief Type-specific callback function types.
using StartBroadcastCallback = std::function<void(DeviceId sender_id, const StartBroadcastMsg& msg)>;
using ReadingCallback = std::function<void(DeviceId sender_id, const ReadingMsg& msg)>;
using ReceiptCallback = std::function<void(DeviceId sender_id, const ReceiptMsg& msg)>;

//! @brief Connection state callback function type.
//! @param connected true if connected, false if disconnected.
using ConnectionCallback = std::function<void(bool connected)>;

//! @brief Abstract transport for BLE messaging in this library.
//! @details
//! Provides both Arduino-friendly methods and complete contract for BLE drivers.
//! All payloads are compact, little-endian from Messages.h serializers.
//! Native drivers reserve DeviceId(0) as broker inbox.
class BleDriver {
 public:
    virtual ~BleDriver() = default;

    //! @brief Initialize the BLE driver and establish connections.
    //! @return true if initialization succeeded, false otherwise.
    //! @pre None. @post Driver ready for messaging operations.
    virtual bool begin() = 0;

    //! @brief Cleanup BLE driver resources and close connections.
    //! @pre Driver initialized. @post Driver can be re-initialized.
    virtual void end() = 0;

    //! @brief Initialize the BLE driver and establish connections.
    //! @return true if initialization succeeded, false otherwise.
    //! @pre None. @post Driver ready for messaging operations.
    virtual bool initialize() = 0;

    //! @brief Cleanup BLE driver resources and close connections.
    //! @pre Driver initialized. @post Driver can be re-initialized.
    virtual void cleanup() = 0;

    //! @brief Check if the driver is connected and ready for communication.
    //! @return true if connected and ready, false otherwise.
    virtual bool is_connected() const = 0;

    //! @brief Get the local device identifier for this driver instance.
    //! @return The device ID that identifies this driver instance.
    virtual DeviceId get_local_device_id() const = 0;

    //! @brief Send a best-effort broadcast from a device.
    //! @param device_id Logical sender identity.
    //! @param payload Serialized message bytes (moved into driver).
    //! @pre Driver initialized and connected.
    virtual void advertise(DeviceId device_id, BlePayload payload) = 0;

    //! @brief Send a directed, point-to-point message.
    //! @param device_id Destination identity.
    //! @param payload Serialized message bytes (moved into driver).
    //! @pre Driver initialized and connected.
    virtual void send_to(DeviceId device_id, BlePayload payload) = 0;

    //! @brief Poll next received payload for a local device.
    //! @param self_id Local identity being polled.
    //! @param out_payload Destination buffer for the payload.
    //! @return True if a payload was returned, false if none available.
    //! @pre Driver initialized.
    virtual bool receive(DeviceId self_id, BlePayload &out_payload) = 0;

    //! @brief Process BLE events.
    //! @pre Driver initialized. Call regularly in main loop.
    virtual void poll() = 0;

    //! @brief Set callback function for received messages.
    //! @param callback Function to call when a message is received.
    //! @pre Driver initialized.
    virtual void set_message_callback(BleMessageCallback callback) = 0;

    //! @brief Remove the message callback.
    //! @pre Driver initialized.
    virtual void clear_message_callback() = 0;

    //! @brief Set callback function for StartBroadcast messages.
    //! @param callback Function to call when a StartBroadcast message is received.
    //! @pre Driver initialized.
    virtual void set_start_broadcast_callback(StartBroadcastCallback callback) = 0;

    //! @brief Set callback function for Reading messages.
    //! @param callback Function to call when a Reading message is received.
    //! @pre Driver initialized.
    virtual void set_reading_callback(ReadingCallback callback) = 0;

    //! @brief Set callback function for Receipt messages.
    //! @param callback Function to call when a Receipt message is received.
    //! @pre Driver initialized.
    virtual void set_receipt_callback(ReceiptCallback callback) = 0;

    //! @brief Remove all type-specific callbacks.
    //! @pre Driver initialized.
    virtual void clear_type_specific_callbacks() = 0;

    //! @brief Set callback function for connection state changes.
    //! @param callback Function to call when connection state changes.
    //! @pre Driver initialized.
    virtual void set_connection_callback(ConnectionCallback callback) = 0;

    //! @brief Remove the connection state callback.
    //! @pre Driver initialized.
    virtual void clear_connection_callback() = 0;
};

} // namespace ble

#endif  // INCLUDE_JENLIB_BLE_BLEDRIVER_H_
