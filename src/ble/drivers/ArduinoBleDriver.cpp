//! @file src/ble/drivers/ArduinoBleDriver.cpp
//! @brief Arduino BLE driver implementation using ArduinoBLE library.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <utility>
#include <array>
#include "jenlib/ble/drivers/ArduinoBleDriver.h"
#include "jenlib/ble/Messages.h"
#include "jenlib/ble/GattProfile.h"

#ifdef ARDUINO
#include <ArduinoBLE.h>
#include <Arduino.h>

namespace {

// Helpers to convert property masks to ArduinoBLE flags
inline int to_arduino_properties(std::uint8_t properties) {
    int flags = 0;
    if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Read)) {
        flags |= BLERead;
    }
    if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write)) {
        flags |= BLEWrite;
    }
    if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Notify)) {
        flags |= BLENotify;
    }
    if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Indicate)) {
        flags |= BLEIndicate;
    }
    if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::WriteWithoutResponse)) {
        flags |= BLEWriteWithoutResponse;
    }
    return flags;
}

// Function-local statics: actual ArduinoBLE objects constructed only when used
inline BLEService& get_service() {
    static BLEService service(jenlib::ble::gatt::kServiceSensor.data());
    return service;
}

inline BLECharacteristic& get_control_chr() {
    static BLECharacteristic chr(jenlib::ble::gatt::kChrControl.data(), to_arduino_properties(
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write)), jenlib::ble::kMaxPayload);
    return chr;
}

inline BLECharacteristic& get_reading_chr() {
    static BLECharacteristic chr(jenlib::ble::gatt::kChrReading.data(), to_arduino_properties(
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Notify)), jenlib::ble::kMaxPayload);
    return chr;
}

inline BLECharacteristic& get_receipt_chr() {
    static BLECharacteristic chr(jenlib::ble::gatt::kChrReceipt.data(), to_arduino_properties(
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write)), jenlib::ble::kMaxPayload);
    return chr;
}

// Non-capturing trampoline functions for ArduinoBLE C API compatibility
void on_control_written(BLEDevice, BLECharacteristic ch) {
    jenlib::ble::BlePayload payload;
    payload.append_raw(ch.value(), ch.valueLength());
    DeviceId sender_id = DeviceId(0x00000000);  // Placeholder - extract from connection

    // Find the driver instance and process the message
    // Note: In a real implementation, you'd need a way to map to the driver instance
    // For now, this is a simplified approach
}

void on_receipt_written(BLEDevice, BLECharacteristic ch) {
    jenlib::ble::BlePayload payload;
    payload.append_raw(ch.value(), ch.valueLength());
    DeviceId sender_id = DeviceId(0x00000000);  // Placeholder - extract from connection

    // Find the driver instance and process the message
    // Note: In a real implementation, you'd need a way to map to the driver instance
    // For now, this is a simplified approach
}

}  // namespace

namespace jenlib::ble {

ArduinoBleDriver::ArduinoBleDriver(std::string_view device_name, DeviceId local_device_id)
    : device_name_(device_name), local_device_id_(local_device_id) {
    message_callback_ = nullptr;
    start_broadcast_callback_ = nullptr;
    reading_callback_ = nullptr;
    receipt_callback_ = nullptr;
    connection_callback_ = nullptr;
    initialized_ = false;
}

ArduinoBleDriver::ArduinoBleDriver(std::string_view device_name, DeviceId local_device_id, const BleCallbacks& cb)
    : ArduinoBleDriver(device_name, local_device_id) {
    if (cb.on_connection) set_connection_callback(cb.on_connection);
    if (cb.on_start) set_start_broadcast_callback(cb.on_start);
    if (cb.on_reading) set_reading_callback(cb.on_reading);
    if (cb.on_receipt) set_receipt_callback(cb.on_receipt);
    if (cb.on_generic) set_message_callback(cb.on_generic);
}

bool ArduinoBleDriver::begin() {
    if (initialized_) {
        return true;
    }

    // Initialize BLE
    if (!BLE.begin()) {
        return false;
    }

    // Setup GATT service
    setup_gatt_service();

    // Start advertising
    BLE.advertise();

    initialized_ = true;
    return true;
}

void ArduinoBleDriver::end() {
    if (initialized_) {
        BLE.end();
        initialized_ = false;
    }
}

// initialize/cleanup removed in favor of begin/end

void ArduinoBleDriver::advertise(DeviceId device_id, BlePayload payload) {
    if (!initialized_) {
        return;
    }

    // For advertising, we'll use the reading characteristic to broadcast
    // This is a simplified approach - in a real implementation you might
    // want to use actual BLE advertising data
    if (is_connected()) {
        BLECharacteristic& reading = get_reading_chr();
        if (payload.size > 0) {
            (void)reading.writeValue(payload.bytes.data(), payload.size);
        }
    }
}

void ArduinoBleDriver::send_to(DeviceId device_id, BlePayload payload) {
    //! @brief No-op on Arduino sensor role.
    //! @details Out of scope for sensor: directed point-to-point sends are handled by the broker.
    (void)device_id;
    (void)payload;
}

bool ArduinoBleDriver::receive(DeviceId self_id, BlePayload &out_payload) {
    if (!initialized_) {
        return false;
    }

    // Process BLE events to handle incoming data
    process_ble_events();

    // Check for pending payloads
    return get_pending_payload(self_id, out_payload);
}

void ArduinoBleDriver::poll() {
    if (!initialized_) {
        return;
    }

    //  Process BLE events - this is the Arduino-friendly polling method
    process_ble_events();
}

void ArduinoBleDriver::set_message_callback(BleMessageCallback callback) {
    message_callback_ = std::move(callback);
}

void ArduinoBleDriver::clear_message_callback() {
    message_callback_ = nullptr;
}

void ArduinoBleDriver::set_start_broadcast_callback(StartBroadcastCallback callback) {
    start_broadcast_callback_ = std::move(callback);
}

void ArduinoBleDriver::set_reading_callback(ReadingCallback callback) {
    reading_callback_ = std::move(callback);
}

void ArduinoBleDriver::set_receipt_callback(ReceiptCallback callback) {
    receipt_callback_ = std::move(callback);
}

void ArduinoBleDriver::clear_type_specific_callbacks() {
    start_broadcast_callback_ = nullptr;
    reading_callback_ = nullptr;
    receipt_callback_ = nullptr;
}

void ArduinoBleDriver::set_connection_callback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

void ArduinoBleDriver::clear_connection_callback() {
    connection_callback_ = nullptr;
}

bool ArduinoBleDriver::is_connected() const {
    return initialized_ && BLE.connected();
}

void ArduinoBleDriver::setup_gatt_service() {
    // Create ArduinoBLE service and characteristics directly
    BLEService& service = get_service();
    BLECharacteristic& control = get_control_chr();
    BLECharacteristic& reading = get_reading_chr();
    BLECharacteristic& receipt = get_receipt_chr();

    service.addCharacteristic(control);
    service.addCharacteristic(reading);
    service.addCharacteristic(receipt);

    // Register event callbacks using non-capturing trampolines for ArduinoBLE C API compatibility
    control.setEventHandler(BLEWritten, on_control_written);
    receipt.setEventHandler(BLEWritten, on_receipt_written);

    // Start advertising using ArduinoBLE directly
    BLE.setAdvertisedService(service);
    BLE.addService(service);
    BLE.advertise();
}

void ArduinoBleDriver::process_ble_events() {
    if (!initialized_) {
        return;
    }

    // Poll for BLE events
    BLE.poll();

    // Detect connection state edges and notify
    const bool now_connected = BLE.connected();
    if (now_connected != last_connected_state_) {
        last_connected_state_ = now_connected;
        if (connection_callback_) {
            connection_callback_(now_connected);
        }
    }
}

void ArduinoBleDriver::send_via_advertising(const BlePayload& payload) {
    //  In a real implementation, you'd set advertising data
    //  For now, this is a placeholder
    (void)payload;
}

//  No separate send_via_gatt helper needed in direct ArduinoBLE path

void ArduinoBleDriver::queue_received_payload(BlePayload payload) {
    received_payloads_.push(std::move(payload));
}

bool ArduinoBleDriver::has_pending_payload(DeviceId device_id) const {
    //  For simplicity, we don't filter by device ID in this implementation
    //  In a real system, you'd need to track which device sent each payload
    (void)device_id;
    return !received_payloads_.empty();
}

bool ArduinoBleDriver::get_pending_payload(DeviceId device_id, BlePayload& out_payload) {
    //  For simplicity, we don't filter by device ID in this implementation
    //  In a real system, you'd need to track which device sent each payload
    (void)device_id;
    return received_payloads_.pop(out_payload);
}

DeviceId ArduinoBleDriver::extract_sender_id_from_connection() {
    //  In a real implementation, you would extract the sender ID from the BLE connection
    //  For now, we'll use a placeholder that could be enhanced with:
    //  1. Connection handle lookup
    //  2. Device address mapping
    //  3. Characteristic-specific routing
    return DeviceId(0x00000000);  //  Placeholder
}

bool ArduinoBleDriver::try_type_specific_callbacks(DeviceId sender_id, const BlePayload& payload) {
    // Try StartBroadcastMsg
    if (start_broadcast_callback_) {
        StartBroadcastMsg start_msg;
        if (StartBroadcastMsg::deserialize(payload, start_msg)) {
            start_broadcast_callback_(sender_id, start_msg);
            return true;
        }
    }

    // Try ReadingMsg
    if (reading_callback_) {
        ReadingMsg reading;
        if (ReadingMsg::deserialize(payload, reading)) {
            reading_callback_(sender_id, reading);
            return true;
        }
    }

    // Try ReceiptMsg
    if (receipt_callback_) {
        ReceiptMsg receipt;
        if (ReceiptMsg::deserialize(payload, receipt)) {
            receipt_callback_(sender_id, receipt);
            return true;
        }
    }

    return false;  //  No type-specific callback handled this message
}

// PayloadBuffer implementation
bool ArduinoBleDriver::PayloadBuffer::push(BlePayload payload) {
    if (full()) {
        return false;
    }

    *write_it = std::move(payload);
    ++write_it;
    if (write_it == payloads.end()) {
        write_it = payloads.begin();
    }
    count++;
    return true;
}

bool ArduinoBleDriver::PayloadBuffer::pop(BlePayload& out_payload) {
    if (empty()) {
        return false;
    }

    out_payload = std::move(*read_it);
    ++read_it;
    if (read_it == payloads.end()) {
        read_it = payloads.begin();
    }
    count--;
    return true;
}

}  // namespace jenlib::ble

#else
// Empty implementation for non-Arduino platforms
// The header file already provides deleted constructors
namespace jenlib::ble {
    // No implementation needed - constructors are deleted in header
}

#endif  // ARDUINO
