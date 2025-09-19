//! @file include/jenlib/ble/Ble.h
//! @brief Simple BLE facade to set driver and send/receive typed messages.
//! @copyright 2025 Jennifer Gott
//!
//! Permission is hereby granted, free of charge, to any person obtaining
//! a copy of this software and associated documentation files (the
//! "Software"), to deal in the Software without restriction, including
//! without limitation the rights to use, copy, modify, merge, publish,
//! distribute, sublicense, and/or sell copies of the Software, and to
//! permit persons to whom the Software is furnished to do so, subject to
//! the following conditions:
//!
//! The above copyright notice and this permission notice shall be
//! included in all copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//! EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//! MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//! IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//! CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//! TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//! SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef INCLUDE_JENLIB_BLE_BLE_H_
#define INCLUDE_JENLIB_BLE_BLE_H_

#include <memory>
#include "jenlib/ble/BleDriver.h"
#include "jenlib/ble/Messages.h"

namespace ble {

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

 private:
    static inline BleDriver *driver_ = nullptr;
};

}  // namespace ble


#endif  // INCLUDE_JENLIB_BLE_BLE_H_