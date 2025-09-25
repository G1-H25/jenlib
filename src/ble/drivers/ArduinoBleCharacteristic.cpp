
//! @file src/ble/drivers/ArduinoBleCharacteristic.cpp
//! @brief Arduino implementation of BLE characteristic using ArduinoBLE library.

#include <jenlib/ble/drivers/BleCharacteristic.h>
#include <utility>

#ifdef ARDUINO
#include <ArduinoBLE.h>
#endif

namespace jenlib::ble {

//! @brief Arduino BLE characteristic implementation using ArduinoBLE library.
class ArduinoBleCharacteristic : public BleCharacteristic {
 public:
    //! @brief Constructor.
    //! @param uuid The characteristic UUID.
    //! @param properties Bitmask of BleCharacteristicProperty values.
    //! @param max_size Maximum payload size for this characteristic.
    ArduinoBleCharacteristic(std::string_view uuid, std::uint8_t properties, std::size_t max_size)
        : uuid_(uuid)
        , properties_(properties)
        , max_size_(max_size)
#ifdef ARDUINO
        , characteristic_(nullptr)
#endif
    {
#ifdef ARDUINO
        // Convert properties to Arduino BLE format
        int arduino_properties = 0;
        if (properties & static_cast<std::uint8_t>(BleCharacteristicProperty::Read)) {
            arduino_properties |= BLERead;
        }
        if (properties & static_cast<std::uint8_t>(BleCharacteristicProperty::Write)) {
            arduino_properties |= BLEWrite;
        }
        if (properties & static_cast<std::uint8_t>(BleCharacteristicProperty::Notify)) {
            arduino_properties |= BLENotify;
        }
        if (properties & static_cast<std::uint8_t>(BleCharacteristicProperty::Indicate)) {
            arduino_properties |= BLEIndicate;
        }
        if (properties & static_cast<std::uint8_t>(BleCharacteristicProperty::WriteWithoutResponse)) {
            arduino_properties |= BLEWriteWithoutResponse;
        }

        // Create the Arduino BLE characteristic
        characteristic_ = new BLECharacteristic(uuid.data(), arduino_properties, max_size);
#endif
    }

    ~ArduinoBleCharacteristic() override {
#ifdef ARDUINO
        delete characteristic_;
#endif
    }

    bool write_value(const BlePayload& payload) override {
#ifdef ARDUINO
        if (characteristic_ && payload.size > 0) {
            return characteristic_->writeValue(payload.bytes.data(), payload.size);
        }
        return false;
#else
        (void)payload;
        return false;
#endif
    }

    bool read_value(BlePayload& out_payload) const override {
#ifdef ARDUINO
        if (!characteristic_ || !has_property(BleCharacteristicProperty::Read)) {
            return false;
        }

        out_payload.clear();
        return out_payload.append_raw(characteristic_->value(), characteristic_->valueLength());
#else
        (void)out_payload;
        return false;
#endif
    }

    void set_event_callback(BleCharacteristicCallback callback) override {
        callback_ = std::move(callback);
#ifdef ARDUINO
        if (characteristic_) {
            characteristic_->setEventHandler(BLEWritten, [this](BLEDevice central, BLECharacteristic characteristic) {
                if (callback_) {
                    BlePayload payload;
                    payload.append_raw(characteristic.value(), characteristic.valueLength());
                    callback_(BleCharacteristicEvent::Written, payload);
                }
            });
        }
#endif
    }

    std::uint8_t get_properties() const override {
        return properties_;
    }

    std::size_t get_max_payload_size() const override {
        return max_size_;
    }

#ifdef ARDUINO
    BLECharacteristic* get_arduino_characteristic() const {
        return characteristic_;
    }
#endif

 private:
    std::string_view uuid_;
    std::uint8_t properties_;
    std::size_t max_size_;
    BleCharacteristicCallback callback_;
#ifdef ARDUINO
    BLECharacteristic* characteristic_;
#endif
};

} // namespace jenlib::ble

