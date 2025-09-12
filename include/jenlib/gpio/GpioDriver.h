//! @file jenlib/gpio/GpioDriver.h
//! @brief GPIO driver interface.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_GPIO_GPIODRIVER_H_
#define INCLUDE_JENLIB_GPIO_GPIODRIVER_H_

#include <cstdint>

//! @namespace gpio
//! @brief GPIO namespace.

namespace gpio {

//! @enum PinMode
//! @brief Pin mode for a GPIO pin.
enum class PinMode : std::uint8_t {
INPUT,
OUTPUT,
INPUT_PULLUP,
INPUT_PULLDOWN,
};

//! @enum DigitalValue
//! @brief Logical digital value for GPIO reads/writes.
enum class DigitalValue : std::uint8_t {
LOW,
HIGH,
};

//! @typedef PinIndex
//! @brief Hardware-defined pin index; user creates mapping.
using PinIndex = std::uint8_t;

class GpioDriver {
 public:
    virtual ~GpioDriver() = default;

    //! @brief Set a pin mode (input/output/pullups).
    //! @param pin The pin index.
    //! @param mode The pin mode.
    //! @note This method is not implemented by the base class.
    virtual void set_pin_mode(PinIndex pin,
                              PinMode mode) noexcept = 0;
    //! Write a logical level to a pin.
    //! @param pin The pin index.
    //! @param value The logical level.
    //! @note This method is not implemented by the base class.
    virtual void digital_write(PinIndex pin,
                               DigitalValue value) noexcept = 0;
    //! Read a logical level from a pin.
    //! @param pin The pin index.
    //! @note This method is not implemented by the base class.
    virtual DigitalValue digital_read(PinIndex pin) noexcept = 0;

    //! Write an analog value (DAC/PWM) to a pin.
    //! @param pin The pin index.
    //! @param value The analog value.
    //! @note This method is not implemented by the base class.
    virtual void analog_write(PinIndex pin,
                              std::uint16_t value) noexcept = 0;
    //! Read an analog value (ADC) from a pin.
    //! @param pin The pin index.
    //! @note This method is not implemented by the base class.
    virtual std::uint16_t analog_read(PinIndex pin) noexcept = 0;

    //! Configure platform analog read resolution in bits.
    //! @param bits The analog read resolution in bits.
    //! @note This method is not implemented by the base class.
    virtual void set_analog_read_resolution(std::uint8_t bits) noexcept = 0;
    //! Configure platform analog write resolution in bits.
    //! @param bits The analog write resolution in bits.
    //! @note This method is not implemented by the base class.
    virtual void set_analog_write_resolution(std::uint8_t bits) noexcept = 0;
    //! Get current analog read resolution.
    //! @note This method is not implemented by the base class.
    virtual std::uint8_t get_analog_read_resolution() const noexcept = 0;
    //! Get current analog write resolution.
    //! @note This method is not implemented by the base class.
    virtual std::uint8_t get_analog_write_resolution() const noexcept = 0;
};

//! @class Pin
//! @brief Lightweight handle to a GPIO pin; forwards calls to the active driver.
class Pin {
 public:
    Pin() noexcept : driver_(nullptr), pin_(0) {}
    //! @brief Constructor.
    //! @param driver The driver.
    //! @param pin The pin index.
    explicit Pin(GpioDriver* driver, PinIndex pin) noexcept
        : driver_(driver), pin_(pin) {}

    //! @brief Set a pin mode (input/output/pullups).
    //! @param mode The pin mode.
    //! @note This method is not implemented by the base class.
    void pin_mode(PinMode mode) const noexcept {
        driver_->set_pin_mode(pin_, mode);
    }
    //! @brief Write a logical level to a pin.
    //! @param value The logical level.
    //! @note This method is not implemented by the base class.
    void digital_write(DigitalValue value) const noexcept {
        driver_->digital_write(pin_, value);
    }
    //! @brief Read a logical level from a pin.
    //! @note This method is not implemented by the base class.
    DigitalValue digital_read() const noexcept {
        return driver_->digital_read(pin_);
    }

    //! @brief Write an analog value (DAC/PWM) to a pin.
    //! @param value The analog value.
    //! @note This method is not implemented by the base class.
    void analog_write(std::uint16_t value) const noexcept {
        driver_->analog_write(pin_, value);
    }
    //! @brief Read an analog value (ADC) from a pin.
    //! @note This method is not implemented by the base class.
    std::uint16_t analog_read() const noexcept {
        return driver_->analog_read(pin_);
    }

    //! @brief Get the pin index.
    //! @note This method is not implemented by the base class.
    PinIndex index() const noexcept { return pin_; }
    //! @brief Get the driver.
    //! @note This method is not implemented by the base class.
    GpioDriver* driver() const noexcept { return driver_; }

 private:
    GpioDriver* driver_; //!< The driver.
    PinIndex pin_; //!< The pin index.
};

} // namespace gpio

#endif  // INCLUDE_JENLIB_GPIO_GPIODRIVER_H_


