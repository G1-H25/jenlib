//! @file include/jenlib/ble/Ble.h
//! @brief Simple BLE facade to set driver and send/receive typed messages.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_BLE_BLE_H_
#define INCLUDE_JENLIB_BLE_BLE_H_

#include <memory>
#include <utility>
#include "jenlib/ble/BleDriver.h"
#include "jenlib/ble/Messages.h"

namespace jenlib::ble {

//! @brief Facade for sending typed BLE messages via a configured driver.
//!
//! This keeps serialization and transport at the edges of the system.
//! Applications set a `BleDriver`, then call these helpers to emit
//! typed messages without worrying about framing. All functions are
//! no-ops when no driver is configured.
class BLE {
 public:
    static void set_driver(BleDriver *driver) { driver_ = driver; }
    static BleDriver * driver() { return driver_; }

    //! @brief Send a message for a device to start broadcasting.
    //! @param device_id The ID of the device to start broadcasting.
    //! @param msg The message to send.
    static void send_start(DeviceId device_id, const StartBroadcastMsg &msg) {
        if (!driver_) {
            return;
        }
        BlePayload p;
        if (!StartBroadcastMsg::serialize(msg, p)) {
            return;
        }
        driver_->send_to(device_id, std::move(p));
    }

    //! @brief Broadcast a sensor reading.
    //! @param sender_id The ID of the device sending the message.
    //! @param msg The message to send.
    static void broadcast_reading(DeviceId sender_id, const ReadingMsg &msg) {
        if (!driver_) {
            return;
        }
        BlePayload p;
        if (!ReadingMsg::serialize(msg, p)) {
            return;
        }
        driver_->advertise(sender_id, std::move(p));
    }

    //! @brief Send a receipt message to a device.
    //! @param device_id The ID of the device to send the message to.
    //! @param msg The message to send.
    static void send_receipt(DeviceId device_id, const ReceiptMsg &msg) {
        if (!driver_) {
            return;
        }
        BlePayload p;
        if (!ReceiptMsg::serialize(msg, p)) {
            return;
        }
        driver_->send_to(device_id, std::move(p));
    }

    //! @brief Poll next received payload for a local device.
    //! @param self_id Local identity being polled.
    //! @param out_payload Destination buffer for the payload.
    //! @return True if a payload was returned, false if none available.
    static bool receive(DeviceId self_id, BlePayload &out_payload) {
        if (!driver_) {
            return false;
        }
        return driver_->receive(self_id, out_payload);
    }

    //! @brief Set callback function for connection state changes.
    //! @param callback Function to call when connection state changes.
    static void set_connection_callback(ConnectionCallback callback) {
        if (driver_) {
            driver_->set_connection_callback(std::move(callback));
        }
    }

    //! @brief Remove the connection state callback.
    static void clear_connection_callback() {
        if (driver_) {
            driver_->clear_connection_callback();
        }
    }

    //! @brief Process BLE events (call in main loop).
    static void process_events() {
        if (driver_) {
            driver_->poll();
        }
    }

    //! @brief Begin BLE driver lifecycle.
    static bool begin() { return driver_ ? driver_->begin() : false; }

    //! @brief End BLE driver lifecycle.
    static void end() { if (driver_) driver_->end(); }

    //! @brief Query connection status.
    static bool is_connected() { return driver_ ? driver_->is_connected() : false; }

    // Forward type-specific callback setters to driver for convenience
    static void set_start_broadcast_callback(StartBroadcastCallback cb) {
        if (driver_) driver_->set_start_broadcast_callback(std::move(cb));
    }
    static void set_reading_callback(ReadingCallback cb) {
        if (driver_) driver_->set_reading_callback(std::move(cb));
    }
    static void set_receipt_callback(ReceiptCallback cb) {
        if (driver_) driver_->set_receipt_callback(std::move(cb));
    }
    static void set_message_callback(BleMessageCallback cb) {
        if (driver_) driver_->set_message_callback(std::move(cb));
    }

 private:
    static inline BleDriver *driver_ = nullptr;
};

}  // namespace jenlib::ble


#endif  // INCLUDE_JENLIB_BLE_BLE_H_
