//! @file src/ble/drivers/NativeBleService.cpp
//! @brief Native implementation of BLE service for testing/simulation.

#include <jenlib/ble/drivers/BleService.h>
#include <jenlib/ble/drivers/BleCharacteristic.h>
#include <unordered_map>
#include <vector>
#include <memory>

namespace jenlib::ble {

//! @brief Native BLE service implementation for testing/simulation.
class NativeBleService : public jenlib::ble::BleService {
 public:
    //! @brief Constructor.
    //! @param uuid The service UUID.
    explicit NativeBleService(std::string_view uuid) : uuid_(uuid), advertising_(false) {
    }

    bool add_characteristic(jenlib::ble::BleCharacteristic* characteristic) override {
        if (!characteristic) {
            return false;
        }

        // Store the characteristic
        characteristics_.push_back(std::unique_ptr<jenlib::ble::BleCharacteristic>(characteristic));
        return true;
    }

    jenlib::ble::BleCharacteristic* get_characteristic(std::string_view uuid) override {
        for (auto& characteristic : characteristics_) {
            // In a real implementation, we'd need to store UUIDs with characteristics
            // For now, we'll return the first characteristic as a placeholder
            if (characteristic) {
                return characteristic.get();
            }
        }
        return nullptr;
    }

    std::string_view get_uuid() const override {
        return uuid_;
    }

    bool start_advertising() override {
        advertising_ = true;
        return true;
    }

    void stop_advertising() override {
        advertising_ = false;
    }

    bool is_advertising() const {
        return advertising_;
    }

 private:
    std::string_view uuid_;
    std::vector<std::unique_ptr<jenlib::ble::BleCharacteristic>> characteristics_;
    bool advertising_;
};

} // namespace jenlib::ble
