//! @file include/jenlib/onewire/OneWireBus.h
//! @brief Type-safe OneWire bus interface
//! Copyright 2025 Jennifer Gott
#ifndef INCLUDE_JENLIB_ONEWIRE_ONEWIREBUS_H_
#define INCLUDE_JENLIB_ONEWIRE_ONEWIREBUS_H_

#include <cstddef>
#include <cstdint>
#include <array>
#include <iterator>
#include <type_traits>
#include "../gpio/GPIO.h"
#include "../gpio/PinTypes.h"

//! @namespace OneWire
//! @brief Public wrapper API for OneWire bus operations.
namespace OneWire {

//! \brief OneWire bus interface abstraction.
//! \details Provides a minimal, platform-agnostic API for 1-Wire transactions.
//! This is a concrete class that can be easily understood by students.
//! All multi-byte data uses little-endian byte order as sent on the wire (LSB first)
//! consistent with 1-Wire protocol.
class OneWireBus {
 public:
        using byte = std::uint8_t;
        //! 64-bit ROM code, LSB first as transmitted on the wire.
        using rom_code_t = std::array<byte, 8>;

        //! @brief Standard ROM-level commands.
        enum class Command : byte {
            ReadRom     = 0x33,  //  Single-drop only
            MatchRom    = 0x55,
            SearchRom   = 0xF0,
            AlarmSearch = 0xEC,
            SkipRom     = 0xCC
        };

        //! @brief Constructor with type-safe pin.
        //! @param pin The typed pin to use for the OneWire bus.
        explicit OneWireBus(GPIO::OneWirePin pin);
        //! @brief Constructor with generic GPIO pin.
        //! @param pin The GPIO pin to use for the OneWire bus.
        explicit OneWireBus(GPIO::Pin pin);
        //! @brief Construct from a raw platform pin number.
        //! @param raw_pin The raw pin number (for backward compatibility).
        explicit OneWireBus(std::uint8_t raw_pin);
        //! @brief Destructor.
        ~OneWireBus() = default;

        //! @brief Initialize the bus and configure the GPIO.
        void begin();
        //! @brief Release any resources associated with the bus.
        void end();

        //! @brief Issue reset pulse and detect presence.
        //! @return true if at least one device pulls presence, false otherwise.
        bool reset();

        //! @brief Write a single byte (LSB first on the wire).
        void write_byte(byte data);

        //! @brief Write a range of bytes using iterators (no raw arrays required).
        //! @tparam InputIt An input iterator whose value_type is byte-compatible.
        //! @param first Iterator to the first byte to write.
        //! @param last Iterator past the last byte to write.
        template <typename InputIt>
        std::size_t write_bytes(InputIt first, InputIt last) {
            // Accept input iterators; write element-wise to avoid contiguity assumptions.
            for (auto it = first; it != last; ++it) {
                write_byte(static_cast<byte>(*it));
            }
            return static_cast<std::size_t>(std::distance(first, last));
        }

        //! @brief Read a single byte (LSB first on the wire).
        byte read_byte();

        //! @brief Read into a range using iterators (no raw arrays required).
        //! @tparam OutputIt An output iterator assignable from byte.
        //! @param first Iterator to the first destination element to write.
        //! @param last Iterator past the last destination element to write.
        //! @return Number of bytes read.
        template <typename OutputIt>
        std::size_t read_bytes(OutputIt first, OutputIt last) {
            for (auto it = first; it != last; ++it) {
                *it = read_byte();
            }
            return static_cast<std::size_t>(std::distance(first, last));
        }

        //! @brief Send SKIP ROM (address all devices).
        void skip_rom();

        //! @brief Send MATCH ROM with the provided device address.
        //! @param rom The 64-bit ROM code (LSB first).
        void match_rom(const rom_code_t& rom);

        //! @brief Read the ROM code (only valid on single-drop bus).
        //! @param out_rom Output parameter receiving the 64-bit ROM code.
        //! @return true if read likely valid (caller should verify CRC).
        bool read_rom(rom_code_t& out_rom);

        //! @brief Compute Maxim/Dallas CRC-8 over a byte range.
        //! @tparam InputIt An input iterator of bytes.
        //! @param first Iterator to the first byte to compute CRC-8 over.
        //! @param last Iterator past the last byte to compute CRC-8 over.
        //! @return CRC-8 value.
        template <typename InputIt>
        static std::uint8_t crc8(InputIt first, InputIt last);

 private:
        //! @brief The pin used for this OneWire bus.
        std::uint8_t pin_;

        //! @brief Whether the bus has been initialized.
        bool initialized_;

        //! @brief Internal method to configure the pin for OneWire use.
        void configure_pin();

        //! @brief Internal method to perform a reset pulse.
        //! @return true if a device is present, false otherwise.
        bool perform_reset();

        //! @brief Internal method to write a single bit.
        //! @param bit The bit to write (0 or 1).
        void write_bit(bool bit);

        //! @brief Internal method to read a single bit.
        //! @return The bit read from the bus.
        bool read_bit();
};

// Inline CRC-8 (Dallas/Maxim, poly 0x31 reflected => 0x8C, init 0x00)
template <typename InputIt>
inline std::uint8_t OneWireBus::crc8(InputIt first, InputIt last) {
    std::uint8_t crc = 0x00;
    for (auto it = first; it != last; ++it) {
        std::uint8_t in = static_cast<std::uint8_t>(*it);
        for (int i = 0; i < 8; ++i) {
            std::uint8_t mix = (crc ^ in) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;  //  reversed 0x31
            in >>= 1;
        }
    }
    return crc;
}

}  // namespace OneWire

#endif  // INCLUDE_JENLIB_ONEWIRE_ONEWIREBUS_H_
