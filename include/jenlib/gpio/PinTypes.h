//! @file jenlib/gpio/PinTypes.h
//! @brief Type-safe pin wrappers for different use cases.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_GPIO_PINTYPES_H_
#define INCLUDE_JENLIB_GPIO_PINTYPES_H_

#include "GPIO.h"
#include <cstdint>
#include <type_traits>

namespace jenlib::gpio {

//! @brief Type-safe wrapper for pins used with external libraries.
//! @details This wrapper provides type safety while allowing conversion to raw pin numbers
//! when needed for compatibility with existing libraries like OneWire, SPI, I2C, etc.
template<typename Tag>
class TypedPin {
 public:
    //! @brief Construct from a GPIO::Pin.
    explicit TypedPin(const Pin& pin) noexcept : pin_(pin) {}

    //! @brief Construct from a raw pin index.
    explicit TypedPin(PinIndex index) noexcept : pin_(index) {}

    //! @brief Get the underlying GPIO::Pin.
    const Pin& getPin() const noexcept { return pin_; }

    //! @brief Get the raw pin index.
    PinIndex getIndex() const noexcept { return pin_.index(); }

    //! @brief Implicit conversion to raw pin number for library compatibility.
    operator PinIndex() const noexcept { return pin_.index(); }

    //! @brief Implicit conversion to GPIO::Pin for GPIO operations.
    operator const Pin&() const noexcept { return pin_; }

    //! @brief Access GPIO operations through the underlying pin.
    const Pin* operator->() const noexcept { return &pin_; }

    //! @brief Access GPIO operations through the underlying pin.
    const Pin& operator*() const noexcept { return pin_; }

 private:
    Pin pin_;
};

//! @namespace jenlib::gpio::PinTags
//! @brief Tag types for different pin uses and type safety.
//! @details
//! Provides tag types that enable compile-time type safety for
//! different pin use cases. These tags prevent accidental mixing
//! of pins intended for different purposes (OneWire, SPI, I2C, etc.).
//!
//! @par Usage Example:
//! @code
//! #include <jenlib/gpio/PinTypes.h>
//!
//! // Create type-safe pins
//! auto onewire_pin = jenlib::gpio::makeTypedPin<jenlib::gpio::PinTags::OneWire>(2);
//! auto spi_pin = jenlib::gpio::makeTypedPin<jenlib::gpio::PinTags::SPI>(13);
//!
//! // Type-safe operations
//! onewire_pin->pinMode(jenlib::gpio::PinMode::OUTPUT);
//! spi_pin->pinMode(jenlib::gpio::PinMode::OUTPUT);
//!
//! // Implicit conversion to raw pin for library compatibility
//! OneWire onewire_bus(onewire_pin);  // Converts to raw pin number
//! @endcode
namespace PinTags {
struct OneWire {};
struct SPI {};
struct I2C {};
struct UART {};
struct PWM {};
struct ADC {};
struct Digital {};
}

//! @brief Type aliases for common pin types.
using OneWirePin = TypedPin<PinTags::OneWire>;
using SPIPin = TypedPin<PinTags::SPI>;
using I2CPin = TypedPin<PinTags::I2C>;
using UARTPin = TypedPin<PinTags::UART>;
using PWMPin = TypedPin<PinTags::PWM>;
using ADCPin = TypedPin<PinTags::ADC>;
using DigitalPin = TypedPin<PinTags::Digital>;

//! @brief Factory function to create typed pins from raw indices.
template<typename Tag>
TypedPin<Tag> makeTypedPin(PinIndex index) noexcept {
    return TypedPin<Tag>(index);
}

//! @brief Factory function to create typed pins from GPIO::Pin.
template<typename Tag>
TypedPin<Tag> makeTypedPin(const Pin& pin) noexcept {
    return TypedPin<Tag>(pin);
}

}  // namespace jenlib::gpio

#endif  // INCLUDE_JENLIB_GPIO_PINTYPES_H_
