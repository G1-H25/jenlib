//! @file include/jenlib/onewire/drivers/EspIdfOneWireBus.h
//! @brief ESP-IDF OneWire bus driver interface.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_ONEWIRE_DRIVERS_ESPIDFONEWIREBUS_H_
#define INCLUDE_JENLIB_ONEWIRE_DRIVERS_ESPIDFONEWIREBUS_H_

#include <jenlib/onewire/OneWireBus.h>

#ifdef ESP_PLATFORM
#include <driver/gpio.h>

namespace jenlib::onewire {

//! @brief ESP-IDF OneWire bus driver implementation.
class EspIdfOneWireBus {
 public:
    //! @brief Constructor.
    explicit EspIdfOneWireBus(gpio_num_t pin);

    //! @brief Destructor.
    ~EspIdfOneWireBus() = default;

    //! @brief Initialize the OneWire bus.
    bool begin();

    //! @brief Reset the OneWire bus.
    bool reset();

    //! @brief Write a bit to the OneWire bus.
    void write_bit(bool bit);

    //! @brief Read a bit from the OneWire bus.
    bool read_bit();

    //! @brief Write a byte to the OneWire bus.
    void write_byte(std::uint8_t data);

    //! @brief Read a byte from the OneWire bus.
    std::uint8_t read_byte();

    //! @brief Write multiple bytes to the OneWire bus.
    void write_bytes(const std::uint8_t* data, std::size_t count);

    //! @brief Read multiple bytes from the OneWire bus.
    void read_bytes(std::uint8_t* data, std::size_t count);

    //! @brief Search for devices on the OneWire bus.
    bool search(std::uint8_t* device_address);

    //! @brief Get the number of devices found.
    std::size_t get_device_count() const;

 private:
    gpio_num_t pin_;                    //!< GPIO pin number
    bool initialized_;                  //!< Initialization state
    std::size_t device_count_;         //!< Number of devices found

    //! @brief Set GPIO pin as output.
    void set_output() const;

    //! @brief Set GPIO pin as input.
    void set_input() const;

    //! @brief Write a bit with proper timing.
    void write_bit_timing(bool bit) const;

    //! @brief Read a bit with proper timing.
    bool read_bit_timing() const;

    //! @brief Wait for specified microseconds.
    void wait_us(std::uint32_t us) const;
};

}  // namespace jenlib::onewire

#else
// Fallback implementation for non-ESP platforms
namespace jenlib::onewire {

//! @brief Empty stub implementation for non-ESP platforms.
class EspIdfOneWireBus {
 public:
    EspIdfOneWireBus() = delete;
};

}  // namespace jenlib::onewire

#endif  // ESP_PLATFORM

#endif  // INCLUDE_JENLIB_ONEWIRE_DRIVERS_ESPIDFONEWIREBUS_H_
