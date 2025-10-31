//! @file include/jenlib/ble/drivers/ArduinoBleDriver.h
//! @brief Arduino BLE driver implementation using ArduinoBLE library.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_BLE_DRIVERS_ARDUINOBLEDRIVER_H_
#define INCLUDE_JENLIB_BLE_DRIVERS_ARDUINOBLEDRIVER_H_

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>
#include "jenlib/ble/BleDriver.h"
#include "jenlib/ble/Payload.h"
#include "jenlib/ble/Ids.h"
#include "jenlib/ble/PayloadBuffer.h"
// No service/characteristic adapter includes: Arduino path uses ArduinoBLE directly in .cpp

#ifdef ARDUINO
#include <ArduinoBLE.h>
#endif

namespace jenlib::ble {

//! @brief Arduino BLE driver implementation using ArduinoBLE library.
//! @details
//! Implements BleDriver interface using ArduinoBLE library.
//! Supports GATT characteristics for messaging and advertising for broadcasting.
class ArduinoBleDriver : public BleDriver {
 public:
    //! @brief Constructor.
    //! @param device_name Name to advertise for this BLE device.
    //! @param local_device_id Local device identifier for this instance.
    //! @brief Construct with name and local id.
    ArduinoBleDriver(std::string_view device_name, DeviceId local_device_id);

    //! @brief Construct and bind callbacks via aggregate.
    ArduinoBleDriver(std::string_view device_name, DeviceId local_device_id, const BleCallbacks& callbacks);

    //! @brief Destructor.
    ~ArduinoBleDriver() override = default;

    //! @brief Initialize the BLE driver and establish connections.
    //! @return true if initialization succeeded, false otherwise.
    //! @pre None. @post Driver ready for messaging operations.
    bool begin() override;

    //! @brief Cleanup BLE driver resources and close connections.
    //! @pre Driver initialized. @post Driver can be re-initialized.
    void end() override;

    // initialize/cleanup removed in favor of begin/end

    //! @brief Send a best-effort broadcast from a device.
    //! @param device_id Logical sender identity.
    //! @param payload Serialized message bytes (moved into driver).
    //! @pre Driver initialized and connected.
    void advertise(DeviceId device_id, BlePayload payload) override;

    //! @brief Send a directed, point-to-point message.
    //! @param device_id Destination identity.
    //! @param payload Serialized message bytes (moved into driver).
    //! @pre Driver initialized and connected.
    void send_to(DeviceId device_id, BlePayload payload) override;

    //! @brief Poll next received payload for a local device.
    //! @param self_id Local identity being polled.
    //! @param out_payload Destination buffer for the payload.
    //! @return True if a payload was returned, false if none available.
    //! @pre Driver initialized.
    bool receive(DeviceId self_id, BlePayload &out_payload) override;

    //! @brief Process BLE events.
    //! @pre Driver initialized. Call regularly in main loop.
    void poll() override;

    //! @brief Set callback function for received messages.
    //! @param callback Function to call when a message is received.
    //! @pre Driver initialized.
    void set_message_callback(BleMessageCallback callback) override;

    //! @brief Remove the message callback.
    //! @pre Driver initialized.
    void clear_message_callback() override;

    //! @brief Set callback function for StartBroadcast messages.
    //! @param callback Function to call when a StartBroadcast message is received.
    //! @pre Driver initialized.
    void set_start_broadcast_callback(StartBroadcastCallback callback) override;

    //! @brief Set callback function for Reading messages.
    //! @param callback Function to call when a Reading message is received.
    //! @pre Driver initialized.
    void set_reading_callback(ReadingCallback callback) override;

    //! @brief Set callback function for Receipt messages.
    //! @param callback Function to call when a Receipt message is received.
    //! @pre Driver initialized.
    void set_receipt_callback(ReceiptCallback callback) override;

    //! @brief Remove all type-specific callbacks.
    //! @pre Driver initialized.
    void clear_type_specific_callbacks() override;

    //! @brief Set callback function for connection state changes.
    //! @param callback Function to call when connection state changes.
    //! @pre Driver initialized.
    void set_connection_callback(ConnectionCallback callback) override;

    //! @brief Remove the connection state callback.
    //! @pre Driver initialized.
    void clear_connection_callback() override;

    //! @brief Check if the driver is connected and ready for communication.
    //! @return true if connected and ready, false otherwise.
    bool is_connected() const override;

    //! @brief Get the local device identifier for this driver instance.
    //! @return The device ID that identifies this driver instance.
    DeviceId get_local_device_id() const override { return local_device_id_; }

 private:
    //! @brief Setup GATT service and characteristics.
    void setup_gatt_service();

    //! @brief Process incoming BLE events.
    void process_ble_events();

    //! @brief Send payload via advertising.
    //! @param payload The payload to advertise.
    void send_via_advertising(const BlePayload& payload);

    //! @brief Queue a received payload for polling.
    //! @param payload The received payload.
    void queue_received_payload(BlePayload payload);

    //! @brief Check if we have a pending payload for the given device.
    //! @param device_id The device ID to check.
    //! @return true if there's a pending payload, false otherwise.
    bool has_pending_payload(DeviceId device_id) const;

    //! @brief Get the next pending payload for the given device.
    //! @param device_id The device ID.
    //! @param out_payload Destination for the payload.
    //! @return true if a payload was retrieved, false otherwise.
    bool get_pending_payload(DeviceId device_id, BlePayload& out_payload);

    //! @brief Extract sender ID from BLE connection context (Arduino placeholder).
    //! @return The sender device ID.
    DeviceId extract_sender_id_from_connection();

    //! @brief Try to handle payload with type-specific callbacks.
    //! @param sender_id The sender device ID.
    //! @param payload The received payload.
    //! @return true if handled by type-specific callback, false otherwise.
    bool try_type_specific_callbacks(DeviceId sender_id, const BlePayload& payload);

    // Uses common PayloadBuffer

    static constexpr std::size_t kMaxDeviceNameLen = 31;
    std::string_view device_name_;  //!<  Non-owning; copied to stack buffer in begin()
    DeviceId local_device_id_;  //!<  Local device identifier.
    PayloadBuffer received_payloads_;  //!<  Buffer for received payloads.
    BleMessageCallback message_callback_;  //!<  Callback for received messages.
    StartBroadcastCallback start_broadcast_callback_;  //!<  Callback for StartBroadcast messages.
    ReadingCallback reading_callback_;  //!<  Callback for Reading messages.
    ReceiptCallback receipt_callback_;  //!<  Callback for Receipt messages.
    ConnectionCallback connection_callback_;  //!<  Callback for connection state changes.

    // ArduinoBLE service/characteristics are defined as function-local statics in the .cpp.
    bool initialized_;  //!<  Initialization state.
    bool last_connected_state_ = false;  //!<  Track last connection state for edge detection.
};

}  //  namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_DRIVERS_ARDUINOBLEDRIVER_H_
