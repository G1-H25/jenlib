//! @file src/ble/drivers/NativeBleCharacteristic.cpp
//! @brief Native implementation of BLE characteristic for testing/simulation.

#include <jenlib/ble/drivers/BleCharacteristic.h>
#include <array>
#include <cstring>

namespace jenlib::ble {

//! @brief Native BLE characteristic implementation for testing/simulation.
class NativeBleCharacteristic : public jenlib::ble::BleCharacteristic {
 public:
    //! @brief Constructor.
    //! @param uuid The characteristic UUID.
    //! @param properties Bitmask of BleCharacteristicProperty values.
    //! @param max_size Maximum payload size for this characteristic.
    NativeBleCharacteristic(std::string_view uuid, std::uint8_t properties, std::size_t max_size)
        : uuid_(uuid)
        , properties_(properties)
        , max_size_(max_size)
        , current_value_{}
        , current_size_(0) {
    }

    bool write_value(const jenlib::ble::BlePayload& payload) override {
        if (payload.size > max_size_) {
            return false;
        }

        // Copy the payload data
        std::memcpy(current_value_.data(), payload.bytes.data(), payload.size);
        current_size_ = payload.size;

        // Trigger callback if set
        if (callback_ && has_property(jenlib::ble::BleCharacteristicProperty::Write)) {
            jenlib::ble::BlePayload callback_payload;
            callback_payload.append_raw(payload.bytes.data(), payload.size);
            callback_(jenlib::ble::BleCharacteristicEvent::Written, callback_payload);
        }

        return true;
    }

    bool read_value(jenlib::ble::BlePayload& out_payload) const override {
        if (!has_property(jenlib::ble::BleCharacteristicProperty::Read) || current_size_ == 0) {
            return false;
        }

        out_payload.clear();
        return out_payload.append_raw(current_value_.data(), current_size_);
    }

    void set_event_callback(jenlib::ble::BleCharacteristicCallback callback) override {
        callback_ = std::move(callback);
    }

    std::uint8_t get_properties() const override {
        return properties_;
    }

    std::size_t get_max_payload_size() const override {
        return max_size_;
    }

    std::string_view get_uuid() const {
        return uuid_;
    }

 private:
    std::string_view uuid_;
    std::uint8_t properties_;
    std::size_t max_size_;
    std::array<std::uint8_t, jenlib::ble::kMaxPayload> current_value_;
    std::size_t current_size_;
    jenlib::ble::BleCharacteristicCallback callback_;
};

} // namespace jenlib::ble

