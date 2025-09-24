//! @file include/jenlib/config/BuildConfig.h
//! @brief Build configuration header.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)


#ifndef INCLUDE_JENLIB_CONFIG_BUILDCONFIG_H_
#define INCLUDE_JENLIB_CONFIG_BUILDCONFIG_H_

namespace jenlib::config {


//! @brief Whether the build is for a sensor only.
//! Set build flag to skip GATT paths (maybe use a preprocessor define?).
#ifdef JENLIB_BLE_SENSOR_ONLY
inline constexpr bool kSensorOnly = true;
#else
inline constexpr bool kSensorOnly = false;
#endif // JENLIB_BLE_SENSOR_ONLY

} // namespace jenlib::config

#endif // INCLUDE_JENLIB_CONFIG_BUILDCONFIG_H_
