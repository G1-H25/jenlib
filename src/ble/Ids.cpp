//! @file src/ble/Ids.cpp
//! @brief Implementations for strong ID types with CRC8.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/ble/Ids.h>
#include <utility>

namespace {
// CRC-8-ATM constants
constexpr std::uint8_t kCrc8Poly = 0x07;
constexpr std::uint8_t kCrc8Init = 0x00;
constexpr std::uint8_t kCrc8HighBit = 0x80;

static std::uint8_t crc8(const std::uint8_t *data, size_t len) {
    // CRC-8-ATM (poly 0x07), init 0x00, no reflect, no xorout
    std::uint8_t crc = kCrc8Init;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if (crc & kCrc8HighBit) crc = static_cast<std::uint8_t>((crc << 1) ^ kCrc8Poly);
            else crc <<= 1;
        }
    }
    return crc;
}
}  // namespace

namespace jenlib::ble {

bool DeviceId::serialize(const DeviceId &id, BlePayload &out) {
    const std::uint32_t v = id.value_;
    const std::uint8_t bytes[4] = {
        static_cast<std::uint8_t>(v & 0xFF),
        static_cast<std::uint8_t>((v >> 8) & 0xFF),
        static_cast<std::uint8_t>((v >> 16) & 0xFF),
        static_cast<std::uint8_t>((v >> 24) & 0xFF)
    };
    if (!out.append_raw(bytes, 4)) return false;
    return out.append_u8(crc8(bytes, 4));
}

bool DeviceId::deserialize(const BlePayload &buf, size_t &offset, DeviceId &out) {
    size_t i = offset;
    std::uint32_t v = 0;
    if (!read_u32le(buf, i, v)) return false;
    std::uint8_t crc = 0;
    if (!read_u8(buf, i, crc)) return false;
    const std::uint8_t bytes[4] = {
        static_cast<std::uint8_t>(v & 0xFF),
        static_cast<std::uint8_t>((v >> 8) & 0xFF),
        static_cast<std::uint8_t>((v >> 16) & 0xFF),
        static_cast<std::uint8_t>((v >> 24) & 0xFF)
    };
    if (crc != crc8(bytes, 4)) return false;
    out = DeviceId(v);
    offset = i;
    return true;
}

bool DeviceId::deserialize(BlePayload::const_iterator &it, BlePayload::const_iterator end, DeviceId &out) {
    // Read 4 bytes LE into v
    std::uint32_t v = 0;
    if (!read_u32le(it, end, v)) return false;
    std::uint8_t crc = 0;
    if (!read_u8(it, end, crc)) return false;
    const std::uint8_t bytes[4] = {
        static_cast<std::uint8_t>(v & 0xFF),
        static_cast<std::uint8_t>((v >> 8) & 0xFF),
        static_cast<std::uint8_t>((v >> 16) & 0xFF),
        static_cast<std::uint8_t>((v >> 24) & 0xFF)
    };
    if (crc != crc8(bytes, 4)) return false;
    out = DeviceId(v);
    return true;
}

std::uint8_t compute_crc8(const std::uint8_t *data, size_t len) {
    return crc8(data, len);
}

}  // namespace jenlib::ble
