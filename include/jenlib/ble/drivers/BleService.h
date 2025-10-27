//! @file include/jenlib/ble/drivers/BleService.h
//! @brief Agnostic BLE service interface for cross-platform compatibility.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_BLE_DRIVERS_BLESERVICE_H_
#define INCLUDE_JENLIB_BLE_DRIVERS_BLESERVICE_H_

#include <jenlib/ble/drivers/BleCharacteristic.h>
#include <memory>
#include <string_view>

namespace jenlib::ble {

//! @brief Abstract BLE service interface.
//! @details
//! Platform-agnostic interface for BLE services.
class BleService {
 public:
    virtual ~BleService() = default;

    //! @brief Add a characteristic to this service (preallocated, no ownership transfer).
    //! @param characteristic The characteristic to add (must outlive service).
    //! @return true if the characteristic was added successfully, false otherwise.
    virtual bool add_characteristic(BleCharacteristic* characteristic) = 0;

    //! @brief Get a characteristic by UUID.
    //! @param uuid The UUID of the characteristic to find.
    //! @return Pointer to the characteristic, or nullptr if not found.
    virtual BleCharacteristic* get_characteristic(std::string_view uuid) = 0;

    //! @brief Get the service UUID.
    //! @return The service UUID as a string.
    virtual std::string_view get_uuid() const = 0;

    //! @brief Start advertising this service.
    //! @return true if advertising started successfully, false otherwise.
    virtual bool start_advertising() = 0;

    //! @brief Stop advertising this service.
    virtual void stop_advertising() = 0;
};

}  // namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_DRIVERS_BLESERVICE_H_
