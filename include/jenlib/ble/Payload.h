//! @file include/jenlib/ble/Payload.h
//! @brief Fixed-size BLE payload buffer for Arduino compatibility.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_BLE_PAYLOAD_H_
#define INCLUDE_JENLIB_BLE_PAYLOAD_H_

#include <array>
#include <cstdint>

namespace ble {

//! @brief Maximum payload size used by BLE messages in this library.
//! @details Chosen to fit typical ATT MTU values while leaving headroom.
constexpr std::size_t kMaxPayload = 64u;

//! @brief Fixed-size buffer with helpers for LE encoding/decoding.
//!
//! Replaces std::vector to retain Arduino compatibility and avoid dynamic
//! allocation. Append methods return false if the buffer would overflow.
struct BlePayload {
    std::array<std::uint8_t, kMaxPayload> bytes{};
    std::uint8_t size{0};

    //! @brief Reset the buffer to empty.
    void clear() { size = 0; }

    //! @brief Append one byte.
    bool append_u8(std::uint8_t v) {
        if (size + 1 > bytes.size()) return false;
        bytes[size++] = v;
        return true;
    }
    //! @brief Append a 16-bit little-endian value.
    bool append_u16le(std::uint16_t v) {
        return append_u8(static_cast<std::uint8_t>(v & 0xFF)) &&
               append_u8(static_cast<std::uint8_t>((v >> 8) & 0xFF));
    }
    //! @brief Append a 32-bit little-endian value.
    bool append_u32le(std::uint32_t v) {
        return append_u8(static_cast<std::uint8_t>(v & 0xFF)) &&
               append_u8(static_cast<std::uint8_t>((v >> 8) & 0xFF)) &&
               append_u8(static_cast<std::uint8_t>((v >> 16) & 0xFF)) &&
               append_u8(static_cast<std::uint8_t>((v >> 24) & 0xFF));
    }
    //! @brief Append a signed 16-bit value in little-endian.
    bool append_i16le(std::int16_t v) {
        return append_u16le(static_cast<std::uint16_t>(v));
    }
    //! @brief Append raw bytes.
    bool append_raw(const std::uint8_t *data, std::size_t len) {
        if (size + len > bytes.size()) return false;
        for (std::size_t i = 0; i < len; ++i) bytes[size++] = data[i];
        return true;
    }
};

//! @brief Read a byte from the payload.
inline bool read_u8(const BlePayload &p, std::size_t &i, std::uint8_t &out) {
    if (i + 1 > p.size) return false;
    out = p.bytes[i++];
    return true;
}
//! @brief Read a 16-bit little-endian value from the payload.
inline bool read_u16le(const BlePayload &p, std::size_t &i,
                       std::uint16_t &out) {
    if (i + 2 > p.size) return false;
    out = static_cast<std::uint16_t>(p.bytes[i]) | (static_cast<std::uint16_t>(p.bytes[i+1]) << 8);
    i += 2;
    return true;
}
//! @brief Read a 32-bit little-endian value from the payload.
inline bool read_u32le(const BlePayload &p, std::size_t &i,
                       std::uint32_t &out) {
    if (i + 4 > p.size) return false;
    out = static_cast<std::uint32_t>(p.bytes[i]) |
          (static_cast<std::uint32_t>(p.bytes[i+1]) << 8) |
          (static_cast<std::uint32_t>(p.bytes[i+2]) << 16) |
          (static_cast<std::uint32_t>(p.bytes[i+3]) << 24);
    i += 4;
    return true;
}
//! @brief Read a signed 16-bit value (LE) from the payload.
inline bool read_i16le(const BlePayload &p, std::size_t &i,
                       std::int16_t &out) {
    std::uint16_t tmp = 0;
    if (!read_u16le(p, i, tmp)) return false;
    out = static_cast<std::int16_t>(tmp);
    return true;
}

} // namespace ble


#endif  // INCLUDE_JENLIB_BLE_PAYLOAD_H_
