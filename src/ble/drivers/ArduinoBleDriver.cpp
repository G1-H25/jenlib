//! @file src/ble/drivers/ArduinoBleDriver.cpp
//! @brief Arduino BLE driver implementation using ArduinoBLE library.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <jenlib/ble/drivers/ArduinoBleDriver.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/ble/GattProfile.h>
#include <utility>
#include <vector>

#ifdef ARDUINO
#include <ArduinoBLE.h>
#include <Arduino.h>
#endif

#ifdef ARDUINO
namespace {

class ArduinoBleCharacteristicImpl : public jenlib::ble::BleCharacteristic {
 public:
    ArduinoBleCharacteristicImpl(std::string_view uuid, std::uint8_t properties, std::size_t max_size)
        : uuid_(uuid), properties_(properties), max_size_(max_size), characteristic_() {
        int arduino_properties = 0;
        if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Read)) {
            arduino_properties |= BLERead;
        }
        if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write)) {
            arduino_properties |= BLEWrite;
        }
        if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Notify)) {
            arduino_properties |= BLENotify;
        }
        if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Indicate)) {
            arduino_properties |= BLEIndicate;
        }
        if (properties & static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::WriteWithoutResponse)) {
            arduino_properties |= BLEWriteWithoutResponse;
        }
        characteristic_ = BLECharacteristic(uuid.data(), arduino_properties, max_size);
    }

    bool write_value(const jenlib::ble::BlePayload& payload) override {
        return characteristic_.writeValue(payload.bytes.data(), payload.size);
    }

    bool read_value(jenlib::ble::BlePayload& out_payload) const override {
        out_payload.clear();
        return out_payload.append_raw(characteristic_.value(), characteristic_.valueLength());
    }

    void set_event_callback(jenlib::ble::BleCharacteristicCallback callback) override {
        callback_ = std::move(callback);
        characteristic_.setEventHandler(BLEWritten, [this](BLEDevice, BLECharacteristic ch) {
            if (!callback_) return;
            jenlib::ble::BlePayload payload;
            payload.append_raw(ch.value(), ch.valueLength());
            callback_(jenlib::ble::BleCharacteristicEvent::Written, payload);
        });
    }

    std::uint8_t get_properties() const override { return properties_; }
    std::size_t get_max_payload_size() const override { return max_size_; }

    BLECharacteristic& arduino() { return characteristic_; }

 private:
    std::string_view uuid_;
    std::uint8_t properties_;
    std::size_t max_size_;
    jenlib::ble::BleCharacteristicCallback callback_{};
    BLECharacteristic characteristic_;
};

class ArduinoBleServiceImpl : public jenlib::ble::BleService {
 public:
    explicit ArduinoBleServiceImpl(std::string_view uuid) : uuid_(uuid), service_(uuid.data()) {}

    bool add_characteristic(jenlib::ble::BleCharacteristic* characteristic) override {
        if (!characteristic) return false;
        auto* arduino_char = dynamic_cast<ArduinoBleCharacteristicImpl*>(characteristic);
        if (arduino_char) {
            service_.addCharacteristic(arduino_char->arduino());
        }
        characteristics_.push_back(characteristic);
        return true;
    }

    jenlib::ble::BleCharacteristic* get_characteristic(std::string_view uuid) override {
        (void)uuid;
        return characteristics_.empty() ? nullptr : characteristics_.front();
    }

    std::string_view get_uuid() const override { return uuid_; }

    bool start_advertising() override {
        BLE.setAdvertisedService(service_);
        BLE.addService(service_);
        BLE.advertise();
        return true;
    }

    void stop_advertising() override { BLE.stopAdvertise(); }

 private:
    std::string_view uuid_;
    std::vector<jenlib::ble::BleCharacteristic*> characteristics_;
    BLEService service_;
};

// Static protocol objects
ArduinoBleServiceImpl g_service{jenlib::ble::gatt::kServiceSensor};
ArduinoBleCharacteristicImpl g_control{
jenlib::ble::gatt::kChrControl,
static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write),
jenlib::ble::kMaxPayload};
ArduinoBleCharacteristicImpl g_reading{
jenlib::ble::gatt::kChrReading,
static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Notify),
jenlib::ble::kMaxPayload};
ArduinoBleCharacteristicImpl g_receipt{
jenlib::ble::gatt::kChrReceipt,
static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write),
jenlib::ble::kMaxPayload};
ArduinoBleCharacteristicImpl g_session{
jenlib::ble::gatt::kChrSession,
static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Read),
jenlib::ble::kMaxPayload};

inline jenlib::ble::BleService* make_arduino_service(std::string_view) {
return &g_service;
}
inline jenlib::ble::BleCharacteristic* make_arduino_characteristic(
std::string_view uuid, std::uint8_t, std::size_t) {
if (uuid == jenlib::ble::gatt::kChrControl) return &g_control;
if (uuid == jenlib::ble::gatt::kChrReading) return &g_reading;
if (uuid == jenlib::ble::gatt::kChrReceipt) return &g_receipt;
if (uuid == jenlib::ble::gatt::kChrSession) return &g_session;
return nullptr;
}

}  // namespace
#endif  // ARDUINO

// Forward declarations for Arduino-specific implementations
namespace jenlib::ble {
class ArduinoBleCharacteristic;
class ArduinoBleService;
}

namespace jenlib::ble {

ArduinoBleDriver::ArduinoBleDriver(std::string_view device_name, DeviceId local_device_id)
    : device_name_(device_name), local_device_id_(local_device_id) {
    message_callback_ = nullptr;
    start_broadcast_callback_ = nullptr;
    reading_callback_ = nullptr;
    receipt_callback_ = nullptr;
    connection_callback_ = nullptr;
    gatt_service_ = nullptr;
    control_char_ = nullptr;
    reading_char_ = nullptr;
    receipt_char_ = nullptr;
    session_char_ = nullptr;
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
#ifdef ARDUINO
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

    //  Process BLE events - this is the Arduino-friendly polling method
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
    // Create service via adapters and add abstract characteristics
    gatt_service_ = make_arduino_service(gatt::kServiceSensor);
    control_char_ = make_arduino_characteristic(
        jenlib::ble::gatt::kChrControl,
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write),
        jenlib::ble::kMaxPayload);
    reading_char_ = make_arduino_characteristic(
        jenlib::ble::gatt::kChrReading,
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Notify),
        jenlib::ble::kMaxPayload);
    receipt_char_ = make_arduino_characteristic(
        jenlib::ble::gatt::kChrReceipt,
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Write),
        jenlib::ble::kMaxPayload);
    session_char_ = make_arduino_characteristic(
        jenlib::ble::gatt::kChrSession,
        static_cast<std::uint8_t>(jenlib::ble::BleCharacteristicProperty::Read),
        jenlib::ble::kMaxPayload);

    gatt_service_->add_characteristic(control_char_);
    gatt_service_->add_characteristic(reading_char_);
    gatt_service_->add_characteristic(receipt_char_);
    gatt_service_->add_characteristic(session_char_);

    // Register event callbacks through abstract API
    control_char_->set_event_callback([this](jenlib::ble::BleCharacteristicEvent event,
                                             const jenlib::ble::BlePayload& payload) {
        if (event == jenlib::ble::BleCharacteristicEvent::Written) {
            // Route into common handler
            // Create a temporary characteristic-like shim not needed; pass payload directly
            DeviceId sender_id = extract_sender_id_from_connection(*control_char_);
            // Try type-specific, else generic/queue
            if (!try_type_specific_callbacks(sender_id, payload)) {
                if (message_callback_) {
                    message_callback_(sender_id, payload);
                } else {
                    queue_received_payload(jenlib::ble::BlePayload(payload));
                }
            }
        }
    });

    receipt_char_->set_event_callback([this](jenlib::ble::BleCharacteristicEvent event,
                                             const jenlib::ble::BlePayload& payload) {
        if (event == jenlib::ble::BleCharacteristicEvent::Written) {
            DeviceId sender_id = extract_sender_id_from_connection(*receipt_char_);
            if (!try_type_specific_callbacks(sender_id, payload)) {
                if (message_callback_) {
                    message_callback_(sender_id, payload);
                } else {
                    queue_received_payload(jenlib::ble::BlePayload(payload));
                }
            }
        }
    });

    // Start advertising via service abstraction
    gatt_service_->start_advertising();
#endif
}

void ArduinoBleDriver::process_ble_events() {
#ifdef ARDUINO
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
#endif
}

void ArduinoBleDriver::handle_characteristic_write(BleCharacteristic& characteristic) {
#ifdef ARDUINO
    (void)characteristic;  //  handled via set_event_callback now
#else
    (void)characteristic;
#endif
}

void ArduinoBleDriver::send_via_advertising(const BlePayload& payload) {
#ifdef ARDUINO
    //  In a real implementation, you'd set advertising data
    //  For now, this is a placeholder
    (void)payload;
#endif
}

void ArduinoBleDriver::send_via_gatt(BleCharacteristic& characteristic, const BlePayload& payload) {
#ifdef ARDUINO
    if (payload.size > 0) {
        (void)characteristic.write_value(payload);
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

DeviceId ArduinoBleDriver::extract_sender_id_from_connection(BleCharacteristic& characteristic) {
#ifdef ARDUINO
    //  In a real implementation, you would extract the sender ID from the BLE connection
    //  For now, we'll use a placeholder that could be enhanced with:
    //  1. Connection handle lookup
    //  2. Device address mapping
    //  3. Characteristic-specific routing
    (void)characteristic;
    return DeviceId(0x00000000);  //  Placeholder
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

