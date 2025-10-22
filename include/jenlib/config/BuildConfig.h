//! @file include/jenlib/config/BuildConfig.h
//! @brief Build configuration header.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)


#ifndef INCLUDE_JENLIB_CONFIG_BUILDCONFIG_H_
#define INCLUDE_JENLIB_CONFIG_BUILDCONFIG_H_

namespace jenlib::config {

//! @brief Platform detection
#ifdef ARDUINO
inline constexpr bool kArduinoPlatform = true;
#else
inline constexpr bool kArduinoPlatform = false;
#endif

//! @brief C++ standard version detection
#if __cplusplus >= 202002L
inline constexpr bool kCpp20Available = true;
#else
inline constexpr bool kCpp20Available = false;
#endif

//! @brief Feature availability flags
inline constexpr bool kThreadingAvailable = !kArduinoPlatform || defined(JENLIB_ENABLE_THREADING);
inline constexpr bool kStdMutexAvailable = !kArduinoPlatform || defined(JENLIB_ENABLE_STD_MUTEX);

//! @brief Whether the build is for a sensor only.
//! Set build flag to skip GATT paths (maybe use a preprocessor define?).
#ifdef JENLIB_BLE_SENSOR_ONLY
inline constexpr bool kSensorOnly = true;
#else
inline constexpr bool kSensorOnly = false;
#endif  // JENLIB_BLE_SENSOR_ONLY

//! @brief Whether to use native drivers (desktop/container environments)
inline constexpr bool kUseNativeDrivers = !kArduinoPlatform;

}  //  namespace jenlib::config

#endif  // INCLUDE_JENLIB_CONFIG_BUILDCONFIG_H_
