//! @file include/jenlib/ble/GattProfile.h
//! @brief UUIDs and metadata for the sensor GATT profile.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_BLE_GATTPROFILE_H_
#define INCLUDE_JENLIB_BLE_GATTPROFILE_H_

#include <string_view>

namespace jenlib::ble {

//! @namespace jenlib::ble::gatt
//! @brief GATT profile definitions for BLE transport layer.
//! @details
//! Defines the 128-bit UUIDs and characteristics used for BLE
//! communication between sensors and brokers. These UUIDs are
//! stable placeholders suitable for examples and testing.
//!
//! @note These are placeholder UUIDs but stable for examples/tests.
namespace jenlib::ble::gatt {

//! @brief Service: Sensor Telemetry
constexpr std::string_view kServiceSensor = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";

//! @brief Characteristics
//! @brief Control (StartBroadcast): Write
constexpr std::string_view kChrControl = "6e400010-b5a3-f393-e0a9-e50e24dcca9e";

//! @brief Reading: Notify/Indicate
constexpr std::string_view kChrReading = "6e400011-b5a3-f393-e0a9-e50e24dcca9e";

//! @brief Receipt: Write
constexpr std::string_view kChrReceipt = "6e400012-b5a3-f393-e0a9-e50e24dcca9e";

//! @brief Session: Read (optional)
constexpr std::string_view kChrSession = "6e400013-b5a3-f393-e0a9-e50e24dcca9e";

}  // namespace gatt

}  // namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_GATTPROFILE_H_

