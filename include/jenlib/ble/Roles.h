//! @file include/jenlib/ble/BleRoles.h
//! @brief Minimal role-based API for student-facing apps (Sensor/Broker).

#ifndef INCLUDE_JENLIB_BLE_ROLES_H_
#define INCLUDE_JENLIB_BLE_ROLES_H_

#include <cstdint>
#include <string_view>
#include "jenlib/ble/Ble.h"
#include "jenlib/ble/BleDriver.h"
#include "jenlib/ble/Messages.h"
#include "jenlib/ble/Ids.h"

namespace jenlib::ble {

//! @brief Simple Sensor application facade.
class Sensor {
public:
    explicit Sensor(DeviceId self_id) : self_id_(self_id) {}

    //! @brief Start BLE (forwards to driver).
    bool begin() { return BLE::begin(); }

    //! @brief Stop BLE (forwards to driver).
    void end() { BLE::end(); }

    //! @brief Configure callbacks once.
    void configure_callbacks(const BleCallbacks& cbs) {
        if (cbs.on_connection) BLE::set_connection_callback(cbs.on_connection);
        if (cbs.on_start) BLE::set_start_broadcast_callback(cbs.on_start);
        if (cbs.on_receipt) BLE::set_receipt_callback(cbs.on_receipt);
        if (cbs.on_generic) BLE::set_message_callback(cbs.on_generic);
        // Reading callback is typically not used on sensors (incoming), so omitted intentionally
    }

    //! @brief Broadcast a reading.
    void broadcast_reading(const ReadingMsg& msg) {
        BLE::broadcast_reading(self_id_, msg);
    }

    //! @brief Process events (call in loop).
    void process_events() { BLE::process_events(); }

private:
    DeviceId self_id_;
};

//! @brief Simple Broker application facade.
class Broker {
public:
    Broker() = default;

    bool begin() { return BLE::begin(); }
    void end() { BLE::end(); }

    void configure_callbacks(const BleCallbacks& cbs) {
        if (cbs.on_connection) BLE::set_connection_callback(cbs.on_connection);
        if (cbs.on_reading) BLE::set_reading_callback(cbs.on_reading);
        if (cbs.on_generic) BLE::set_message_callback(cbs.on_generic);
        // Start/Receipt are typically outgoing for broker; omit to reduce confusion
    }

    //! @brief Command a sensor to start broadcasting (assigns a session).
    void send_start(DeviceId sensor, const StartBroadcastMsg& msg) {
        BLE::send_start(sensor, msg);
    }

    //! @brief Acknowledge received readings up to an offset in a session.
    void send_receipt(DeviceId sensor, const ReceiptMsg& msg) {
        BLE::send_receipt(sensor, msg);
    }

    void process_events() { BLE::process_events(); }
};

}  // namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_ROLES_H_



