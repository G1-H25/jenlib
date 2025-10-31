//! @file src/onewire/drivers/EspIdfOneWireBus.cpp
//! @brief ESP-IDF OneWire bus driver implementation.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/onewire/drivers/EspIdfOneWireBus.h>

#ifdef ESP_PLATFORM
#include <driver/gpio.h>
#include <esp_timer.h>

namespace jenlib::onewire {

EspIdfOneWireBus::EspIdfOneWireBus(gpio_num_t pin)
    : pin_(pin), initialized_(false), device_count_(0) {
}

bool EspIdfOneWireBus::begin() {
    if (initialized_) {
        return true;
    }

    // Configure GPIO pin
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin_);
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        return false;
    }

    // Set pin high initially
    gpio_set_level(pin_, 1);

    initialized_ = true;
    return true;
}

bool EspIdfOneWireBus::reset() {
    if (!initialized_) {
        return false;
    }

    set_output();
    gpio_set_level(pin_, 0);
    wait_us(480);

    set_input();
    wait_us(70);

    bool presence = !gpio_get_level(pin_);
    wait_us(410);

    return presence;
}

void EspIdfOneWireBus::write_bit(bool bit) {
    write_bit_timing(bit);
}

bool EspIdfOneWireBus::read_bit() {
    return read_bit_timing();
}

void EspIdfOneWireBus::write_byte(std::uint8_t data) {
    for (int i = 0; i < 8; i++) {
        write_bit(data & 0x01);
        data >>= 1;
    }
}

std::uint8_t EspIdfOneWireBus::read_byte() {
    std::uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        data >>= 1;
        if (read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}

void EspIdfOneWireBus::write_bytes(const std::uint8_t* data, std::size_t count) {
    for (std::size_t i = 0; i < count; i++) {
        write_byte(data[i]);
    }
}

void EspIdfOneWireBus::read_bytes(std::uint8_t* data, std::size_t count) {
    for (std::size_t i = 0; i < count; i++) {
        data[i] = read_byte();
    }
}

bool EspIdfOneWireBus::search(std::uint8_t* device_address) {
    // Simplified search implementation
    // In a real implementation, this would perform the full OneWire search algorithm
    (void)device_address;
    return false;
}

std::size_t EspIdfOneWireBus::get_device_count() const {
    return device_count_;
}

void EspIdfOneWireBus::set_output() const {
    gpio_set_direction(pin_, GPIO_MODE_OUTPUT_OD);
}

void EspIdfOneWireBus::set_input() const {
    gpio_set_direction(pin_, GPIO_MODE_INPUT);
}

void EspIdfOneWireBus::write_bit_timing(bool bit) const {
    set_output();
    gpio_set_level(pin_, 0);

    if (bit) {
        wait_us(6);
        gpio_set_level(pin_, 1);
        wait_us(64);
    } else {
        wait_us(60);
        gpio_set_level(pin_, 1);
        wait_us(10);
    }
}

bool EspIdfOneWireBus::read_bit_timing() const {
    set_output();
    gpio_set_level(pin_, 0);
    wait_us(6);

    set_input();
    wait_us(9);

    bool bit = gpio_get_level(pin_);
    wait_us(55);

    return bit;
}

void EspIdfOneWireBus::wait_us(std::uint32_t us) const {
    esp_timer_wait(us);
}

}  // namespace jenlib::onewire

#else
// Empty implementation for non-ESP platforms
// The header file already provides deleted constructors
namespace jenlib::onewire {
    // No implementation needed - constructors are deleted in header
}

#endif  // ESP_PLATFORM
