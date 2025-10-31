//! @file jenlib/gpio/drivers/NativeGpioDriver.h
//! @brief Native GPIO driver implementation.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_GPIO_DRIVERS_NATIVEGPIODRIVER_H_
#define INCLUDE_JENLIB_GPIO_DRIVERS_NATIVEGPIODRIVER_H_

#include <jenlib/gpio/GpioDriver.h>
#include <unordered_map>  // OK for desktop; NativeGpioDriver doesn't run on Arduino

//! @namespace jenlib::gpio
//! @brief GPIO namespace.
namespace jenlib::gpio {

//! @class NativeGpioDriver
//! @brief Native GPIO driver.
class NativeGpioDriver : public GpioDriver {
 public:
        //! @brief Set the reference voltage.
        //! @param volts The reference voltage.
        //! @note This method is not implemented by the base class.
        void set_reference_voltage(float volts) noexcept {
            reference_voltage_volts_ = volts;
        }
        //! @brief Get the reference voltage.
        //! @return The reference voltage.
        //! @note This method is not implemented by the base class.
        float get_reference_voltage() const noexcept {
            return reference_voltage_volts_;
        }
        //! @brief Set the digital threshold ratio.
        //! @param ratio The digital threshold ratio.
        //! @note This method is not implemented by the base class.
        void set_digital_threshold_ratio(float ratio) noexcept {
            digital_threshold_ratio_ = ratio;
        }
        float get_digital_threshold_ratio() const noexcept {
            return digital_threshold_ratio_;
        }
        void set_pin_voltage(PinIndex pin, float volts) noexcept {
            pin_voltage_volts_[pin] = volts;
        }

        void set_pin_mode(PinIndex pin, PinMode mode) noexcept override {
            pin_modes_[pin] = mode;
        }

        void digital_write(PinIndex pin, DigitalValue value) noexcept override {
            digital_values_[pin] = value;
        }

        DigitalValue digital_read(PinIndex pin) noexcept override {
            auto vit = pin_voltage_volts_.find(pin);
            if (vit != pin_voltage_volts_.end()) {
                const float threshold = reference_voltage_volts_ * digital_threshold_ratio_;
                return vit->second >= threshold ? DigitalValue::HIGH : DigitalValue::LOW;
            }
            auto it = digital_values_.find(pin);
            return it != digital_values_.end() ? it->second : DigitalValue::LOW;
        }

        void analog_write(PinIndex pin, std::uint16_t value) noexcept override {
            analog_values_[pin] = value;
        }

        std::uint16_t analog_read(PinIndex pin) noexcept override {
            auto vit = pin_voltage_volts_.find(pin);
            if (vit != pin_voltage_volts_.end()) {
                const std::uint16_t max_code =
                    static_cast<std::uint16_t>((1u << analog_read_bits_) - 1u);
                float volts = vit->second;
                if (volts < 0.0f) volts = 0.0f;
                if (volts > reference_voltage_volts_) volts = reference_voltage_volts_;
                const float ratio = reference_voltage_volts_ > 0.0f
                                       ? (volts / reference_voltage_volts_)
                                       : 0.0;
                const unsigned int code = static_cast<unsigned int>(
                    ratio * static_cast<float>(max_code) + 0.5f);
                return static_cast<std::uint16_t>(code);
            }
            auto it = analog_values_.find(pin);
            return it != analog_values_.end() ? it->second : 0;
        }

        void set_analog_read_resolution(std::uint8_t bits) noexcept override {
            analog_read_bits_ = bits;
        }

        void set_analog_write_resolution(std::uint8_t bits) noexcept override {
            analog_write_bits_ = bits;
        }

        std::uint8_t get_analog_read_resolution() const noexcept override {
            return analog_read_bits_;
        }

        std::uint8_t get_analog_write_resolution() const noexcept override {
            return analog_write_bits_;
        }

 private:
        std::unordered_map<PinIndex, PinMode> pin_modes_{};
        std::unordered_map<PinIndex, DigitalValue> digital_values_{};
        std::unordered_map<PinIndex, std::uint16_t> analog_values_{};
        std::unordered_map<PinIndex, float> pin_voltage_volts_{};
        std::uint8_t analog_read_bits_{10};
        std::uint8_t analog_write_bits_{8};
        float reference_voltage_volts_{3.3f};
        float digital_threshold_ratio_{0.5f};
};

}  // namespace jenlib::gpio

#endif  // INCLUDE_JENLIB_GPIO_DRIVERS_NATIVEGPIODRIVER_H_
