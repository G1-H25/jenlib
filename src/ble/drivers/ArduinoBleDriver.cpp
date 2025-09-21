//! @file src/ble/drivers/ArduinoBleDriver.cpp
//! @brief Arduino BLE driver implementation using ArduinoBLE library.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <jenlib/ble/drivers/ArduinoBleDriver.h>
#include <jenlib/ble/Messages.h>

#ifdef ARDUINO
#include <ArduinoBLE.h>
#include <Arduino.h>
#endif

// Forward declarations for Arduino-specific implementations
namespace jenlib::ble {
    class ArduinoBleCharacteristic;
    class ArduinoBleService;
}

namespace jenlib::ble {

ArduinoBleDriver::ArduinoBleDriver(const char* device_name, DeviceId local_device_id)
    : device_name_(device_name)
    , local_device_id_(local_device_id)
    , message_callback_(nullptr)
    , start_broadcast_callback_(nullptr)
    , reading_callback_(nullptr)
    , receipt_callback_(nullptr)
    , connection_callback_(nullptr)
    , gatt_service_(nullptr)
    , control_char_(nullptr)
    , reading_char_(nullptr)
    , receipt_char_(nullptr)
    , session_char_(nullptr)
    , initialized_(false)
{
}

bool ArduinoBleDriver::begin() {
#ifdef ARDUINO
    if (initialized_) {
        return true;
    }

    // Initialize BLE
    if (!BLE.begin()) {
        return false;
    }

    // Set local name and advertised service
    BLE.setLocalName(device_name_);
    BLE.setAdvertisedService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");

    // Setup GATT service
    setup_gatt_service();

    // Start advertising
    BLE.advertise();
    
    initialized_ = true;
    return true;
#else
    return false;
#endif
}

void ArduinoBleDriver::end() {
#ifdef ARDUINO
    if (initialized_) {
        BLE.stop();
        initialized_ = false;
    }
#endif
}

// initialize/cleanup removed in favor of begin/end

void ArduinoBleDriver::advertise(DeviceId device_id, BlePayload payload) {
#ifdef ARDUINO
    if (!initialized_) {
        return;
    }

    // For advertising, we'll use the reading characteristic to broadcast
    // This is a simplified approach - in a real implementation you might
    // want to use actual BLE advertising data
    if (reading_char_ && is_connected()) {
        send_via_gatt(*reading_char_, payload);
    }
#else
    (void)device_id;
    (void)payload;
#endif
}

void ArduinoBleDriver::send_to(DeviceId device_id, BlePayload payload) {
#ifdef ARDUINO
    if (!initialized_) {
        return;
    }

    // For point-to-point messaging, we need to determine which characteristic
    // to use based on the message type. This is a simplified implementation.
    // In a real system, you'd need to parse the payload to determine message type.
    
    if (!is_connected()) {
        return;
    }

    // For now, use the control characteristic for directed messages
    // In a real implementation, you'd route based on message type
    if (control_char_) {
        send_via_gatt(*control_char_, payload);
    }
#else
    (void)device_id;
    (void)payload;
#endif
}

bool ArduinoBleDriver::receive(DeviceId self_id, BlePayload &out_payload) {
#ifdef ARDUINO
    if (!initialized_) {
        return false;
    }

    // Process BLE events to handle incoming data
    process_ble_events();

    // Check for pending payloads
    return get_pending_payload(self_id, out_payload);
#else
    (void)self_id;
    (void)out_payload;
    return false;
#endif
}

void ArduinoBleDriver::poll() {
#ifdef ARDUINO
    if (!initialized_) {
        return;
    }

    // Process BLE events - this is the Arduino-friendly polling method
    process_ble_events();
#else
    // No-op for non-Arduino platforms
#endif
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
#ifdef ARDUINO
    return initialized_ && BLE.connected();
#else
    return false;
#endif
}

void ArduinoBleDriver::setup_gatt_service() {
#ifdef ARDUINO
    // Create the main service
    gatt_service_ = new BLEService("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    BLE.setAdvertisedService(*gatt_service_);

    // Control characteristic (StartBroadcast) - Write
    control_char_ = new BLECharacteristic("6e400010-b5a3-f393-e0a9-e50e24dcca9e", 
                                         BLEWrite, kMaxPayload);
    gatt_service_->addCharacteristic(*control_char_);

    // Reading characteristic - Notify
    reading_char_ = new BLECharacteristic("6e400011-b5a3-f393-e0a9-e50e24dcca9e", 
                                         BLENotify, kMaxPayload);
    gatt_service_->addCharacteristic(*reading_char_);

    // Receipt characteristic - Write
    receipt_char_ = new BLECharacteristic("6e400012-b5a3-f393-e0a9-e50e24dcca9e", 
                                         BLEWrite, kMaxPayload);
    gatt_service_->addCharacteristic(*receipt_char_);

    // Session characteristic - Read
    session_char_ = new BLECharacteristic("6e400013-b5a3-f393-e0a9-e50e24dcca9e", 
                                         BLERead, kMaxPayload);
    gatt_service_->addCharacteristic(*session_char_);

    // Add service
    BLE.addService(*gatt_service_);

    // Set up characteristic event handlers
    control_char_->setEventHandler(BLEWritten, [this](BLEDevice central, BLECharacteristic characteristic) {
        handle_characteristic_write(characteristic);
    });

    receipt_char_->setEventHandler(BLEWritten, [this](BLEDevice central, BLECharacteristic characteristic) {
        handle_characteristic_write(characteristic);
    });
#endif
}

void ArduinoBleDriver::process_ble_events() {
#ifdef ARDUINO
    if (!initialized_) {
        return;
    }

    // Poll for BLE events
    BLE.poll();
#endif
}

void ArduinoBleDriver::handle_characteristic_write(BleCharacteristic& characteristic) {
#ifdef ARDUINO
    if (characteristic.valueLength() == 0) {
        return;
    }

    // Create payload from characteristic data
    BlePayload payload;
    if (!payload.append_raw(characteristic.value(), characteristic.valueLength())) {
        return;
    }

    // Extract sender ID from BLE connection context
    DeviceId sender_id = extract_sender_id_from_connection(characteristic);

    // Try type-specific callbacks first
    if (try_type_specific_callbacks(sender_id, payload)) {
        return; // Handled by type-specific callback
    }

    // Fallback to generic callback
    if (message_callback_) {
        message_callback_(sender_id, payload);
    } else {
        // Fallback to queuing for polling-based access
        queue_received_payload(std::move(payload));
    }
#else
    (void)characteristic;
#endif
}

void ArduinoBleDriver::send_via_advertising(const BlePayload& payload) {
#ifdef ARDUINO
    // In a real implementation, you'd set advertising data
    // For now, this is a placeholder
    (void)payload;
#endif
}

void ArduinoBleDriver::send_via_gatt(BleCharacteristic& characteristic, const BlePayload& payload) {
#ifdef ARDUINO
    if (payload.size > 0) {
        characteristic.writeValue(payload.bytes.data(), payload.size);
    }
#else
    (void)characteristic;
    (void)payload;
#endif
}

void ArduinoBleDriver::queue_received_payload(BlePayload payload) {
    received_payloads_.push(std::move(payload));
}

bool ArduinoBleDriver::has_pending_payload(DeviceId device_id) const {
    // For simplicity, we don't filter by device ID in this implementation
    // In a real system, you'd need to track which device sent each payload
    (void)device_id;
    return !received_payloads_.empty();
}

bool ArduinoBleDriver::get_pending_payload(DeviceId device_id, BlePayload& out_payload) {
    // For simplicity, we don't filter by device ID in this implementation
    // In a real system, you'd need to track which device sent each payload
    (void)device_id;
    return received_payloads_.pop(out_payload);
}

DeviceId ArduinoBleDriver::extract_sender_id_from_connection(BleCharacteristic& characteristic) {
#ifdef ARDUINO
    // In a real implementation, you would extract the sender ID from the BLE connection
    // For now, we'll use a placeholder that could be enhanced with:
    // 1. Connection handle lookup
    // 2. Device address mapping
    // 3. Characteristic-specific routing
    (void)characteristic;
    return DeviceId(0x00000000); // Placeholder
#else
    (void)characteristic;
    return DeviceId(0x00000000);
#endif
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

    return false; // No type-specific callback handled this message
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

} // namespace jenlib::ble
