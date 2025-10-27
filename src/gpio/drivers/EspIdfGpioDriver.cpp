//! @file src/gpio/drivers/EspIdfGpioDriver.cpp
//! @brief ESP-IDF GPIO driver implementation.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/gpio/drivers/EspIdfGpioDriver.h>

#ifdef ESP_PLATFORM
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/ledc.h>
#include <esp_adc_cal.h>

namespace jenlib::gpio {

void EspIdfGpioDriver::set_pin_mode(PinIndex pin, PinMode mode) noexcept {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = convert_pin_mode(mode);
    io_conf.pull_up_en = (mode == PinMode::INPUT_PULLUP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = (mode == PinMode::INPUT_PULLDOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    gpio_config(&io_conf);
}

void EspIdfGpioDriver::digital_write(PinIndex pin, DigitalValue value) noexcept {
    gpio_set_level(static_cast<gpio_num_t>(pin), 
                   (value == DigitalValue::HIGH) ? 1 : 0);
}

DigitalValue EspIdfGpioDriver::digital_read(PinIndex pin) noexcept {
    int level = gpio_get_level(static_cast<gpio_num_t>(pin));
    return (level == 1) ? DigitalValue::HIGH : DigitalValue::LOW;
}

void EspIdfGpioDriver::analog_write(PinIndex pin, std::uint16_t value) noexcept {
    // Configure LEDC timer for PWM
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer.duty_resolution = static_cast<ledc_timer_bit_t>(analog_write_bits_);
    ledc_timer.freq_hz = 5000;
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channel
    ledc_channel_config_t ledc_channel = {};
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num = pin;
    ledc_channel.duty = value;
    ledc_channel.hpoint = 0;
    ledc_channel_config(&ledc_channel);
}

std::uint16_t EspIdfGpioDriver::analog_read(PinIndex pin) noexcept {
    // Configure ADC
    adc1_config_width(static_cast<adc_bits_width_t>(analog_read_bits_));
    adc1_config_channel_atten(static_cast<adc1_channel_t>(pin), ADC_ATTEN_DB_11);
    
    // Read ADC value
    int adc_reading = adc1_get_raw(static_cast<adc1_channel_t>(pin));
    
    // Convert to 16-bit value based on resolution
    std::uint16_t max_value = (1 << analog_read_bits_) - 1;
    return static_cast<std::uint16_t>((adc_reading * 65535) / max_value);
}

void EspIdfGpioDriver::set_analog_read_resolution(std::uint8_t bits) noexcept {
    analog_read_bits_ = bits;
}

void EspIdfGpioDriver::set_analog_write_resolution(std::uint8_t bits) noexcept {
    analog_write_bits_ = bits;
}

std::uint8_t EspIdfGpioDriver::get_analog_read_resolution() const noexcept {
    return analog_read_bits_;
}

std::uint8_t EspIdfGpioDriver::get_analog_write_resolution() const noexcept {
    return analog_write_bits_;
}

gpio_mode_t EspIdfGpioDriver::convert_pin_mode(PinMode mode) const noexcept {
    switch (mode) {
        case PinMode::INPUT:
        case PinMode::INPUT_PULLUP:
        case PinMode::INPUT_PULLDOWN:
            return GPIO_MODE_INPUT;
        case PinMode::OUTPUT:
            return GPIO_MODE_OUTPUT;
        default:
            return GPIO_MODE_INPUT;
    }
}

gpio_pull_mode_t EspIdfGpioDriver::convert_pull_mode(PinMode mode) const noexcept {
    switch (mode) {
        case PinMode::INPUT_PULLUP:
            return GPIO_PULLUP_ONLY;
        case PinMode::INPUT_PULLDOWN:
            return GPIO_PULLDOWN_ONLY;
        default:
            return GPIO_FLOATING;
    }
}

}  // namespace jenlib::gpio

#else
// Empty implementation for non-ESP platforms
// The header file already provides deleted constructors
namespace jenlib::gpio {
    // No implementation needed - constructors are deleted in header
}

#endif  // ESP_PLATFORM
