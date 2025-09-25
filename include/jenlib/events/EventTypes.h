//! @file include/jenlib/events/EventTypes.h
//! @brief Event system types and structures for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_EVENTS_EVENTTYPES_H_
#define INCLUDE_JENLIB_EVENTS_EVENTTYPES_H_

#include <cstdint>
#include <functional>

namespace jenlib::events {

//! @brief Event types supported by the event system
enum class EventType : std::uint8_t {
    kTimeTick = 0x01,             //!<  Periodic timer events
    kBleMessage = 0x02,           //!<  BLE message received
    kGpioChange = 0x03,           //!<  GPIO state change
    kMeasurementReady = 0x04,     //!<  Sensor reading available
    kConnectionStateChange = 0x05,  //!<  BLE connection state change
    kCustom = 0x80                //!<  Custom event types start here
};

//! @brief Generic event structure for the event system
struct Event {
    EventType type;               //!<  Type of event
    std::uint32_t timestamp;      //!<  Event timestamp (platform-specific)
    std::uint32_t data;           //!<  Event data (or pointer for complex data)

    //! @brief Default constructor
    Event() : type(EventType::kCustom), timestamp(0), data(0) {}

    //! @brief Constructor with parameters
    Event(EventType event_type, std::uint32_t event_timestamp, std::uint32_t event_data = 0)
        : type(event_type), timestamp(event_timestamp), data(event_data) {}
};

//! @brief Event callback function type
using EventCallback = std::function<void(const Event&)>;

//! @brief Event ID type for tracking registered callbacks
using EventId = std::uint32_t;

//! @brief Invalid event ID constant
constexpr EventId kInvalidEventId = 0;

}  //  namespace jenlib::events

#endif  // INCLUDE_JENLIB_EVENTS_EVENTTYPES_H_

