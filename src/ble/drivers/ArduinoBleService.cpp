
//! @file src/ble/drivers/ArduinoBleService.cpp
//! @brief Arduino implementation of BLE service using ArduinoBLE library.

#include <jenlib/ble/drivers/BleService.h>
#include <jenlib/ble/drivers/BleCharacteristic.h>
#include <vector>

#ifdef ARDUINO
#include <ArduinoBLE.h>
#endif

namespace jenlib::ble {

//! @brief Arduino BLE service implementation using ArduinoBLE library.
class ArduinoBleService : public BleService {
 public:
    //! @brief Constructor.
    //! @param uuid The service UUID.
    explicit ArduinoBleService(std::string_view uuid) : uuid_(uuid)
#ifdef ARDUINO
        , service_(nullptr)
#endif
    {
#ifdef ARDUINO
        service_ = new BLEService(uuid.data());
#endif
    }

    ~ArduinoBleService() override {
#ifdef ARDUINO
        delete service_;
#endif
    }

    bool add_characteristic(BleCharacteristic* characteristic) override {
        if (!characteristic) {
            return false;
        }

#ifdef ARDUINO
        // Cast to Arduino-specific implementation to get the underlying characteristic
        auto* arduino_char = dynamic_cast<class ArduinoBleCharacteristic*>(characteristic);
        if (arduino_char && service_) {
            service_->addCharacteristic(*arduino_char->get_arduino_characteristic());
        }
#endif

        // Store the characteristic
        characteristics_.push_back(std::unique_ptr<BleCharacteristic>(characteristic));
        return true;
    }

    BleCharacteristic* get_characteristic(std::string_view uuid) override {
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
#ifdef ARDUINO
        if (service_) {
            BLE.setAdvertisedService(*service_);
            BLE.addService(*service_);
            BLE.advertise();
            return true;
        }
        return false;
#else
        return false;
#endif
    }

    void stop_advertising() override {
#ifdef ARDUINO
        BLE.stopAdvertise();
#endif
    }

#ifdef ARDUINO
    BLEService* get_arduino_service() const {
        return service_;
    }
#endif

 private:
    std::string_view uuid_;
    std::vector<std::unique_ptr<BleCharacteristic>> characteristics_;
#ifdef ARDUINO
    BLEService* service_;
#endif
};

}  // namespace jenlib::ble

