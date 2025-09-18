//! @file src/ble/Messages.cpp
//! @brief Implementations for BLE message serialization.

#include <jenlib/ble/Messages.h>


namespace ble {

bool StartBroadcastMsg::serialize(const StartBroadcastMsg &msg, BlePayload &out) {
    out.clear();
    if (!out.append_u8(static_cast<std::uint8_t>(MessageType::StartBroadcast))) return false;
    if (!DeviceId::serialize(msg.device_id, out)) return false;
    return out.append_u32le(msg.session_id.value());
}

bool StartBroadcastMsg::deserialize(const BlePayload &buf, StartBroadcastMsg &out) {
    size_t i = 0;
    std::uint8_t type = 0;
    if (!read_u8(buf, i, type)) return false;
    if (type != static_cast<std::uint8_t>(MessageType::StartBroadcast)) return false;
    if (!DeviceId::deserialize(buf, i, out.device_id)) return false;
    std::uint32_t sess = 0;
    if (!read_u32le(buf, i, sess)) return false;
    out.session_id = SessionId(sess);
    return i == buf.size;
}

bool ReadingMsg::serialize(const ReadingMsg &msg, BlePayload &out) {
    out.clear();
    if (!out.append_u8(static_cast<std::uint8_t>(MessageType::Reading))) return false;
    if (!DeviceId::serialize(msg.sender_id, out)) return false;
    if (!out.append_u32le(msg.session_id.value())) return false;
    if (!out.append_u32le(msg.offset_ms)) return false;
    if (!out.append_i16le(msg.temperature_c_centi)) return false;
    return out.append_u16le(msg.humidity_bp);
}

bool ReadingMsg::deserialize(const BlePayload &buf, ReadingMsg &out) {
    size_t i = 0;
    std::uint8_t type = 0;
    if (!read_u8(buf, i, type)) return false;
    if (type != static_cast<std::uint8_t>(MessageType::Reading)) return false;
    if (!DeviceId::deserialize(buf, i, out.sender_id)) return false;
    std::uint32_t sess = 0;
    if (!read_u32le(buf, i, sess)) return false;
    out.session_id = SessionId(sess);
    if (!read_u32le(buf, i, out.offset_ms)) return false;
    if (!read_i16le(buf, i, out.temperature_c_centi)) return false;
    if (!read_u16le(buf, i, out.humidity_bp)) return false;
    return i == buf.size;
}

bool ReceiptMsg::serialize(const ReceiptMsg &msg, BlePayload &out) {
    out.clear();
    if (!out.append_u8(static_cast<std::uint8_t>(MessageType::Receipt))) return false;
    if (!out.append_u32le(msg.session_id.value())) return false;
    return out.append_u32le(msg.up_to_offset_ms);
}

bool ReceiptMsg::deserialize(const BlePayload &buf, ReceiptMsg &out) {
    size_t i = 0;
    std::uint8_t type = 0;
    if (!read_u8(buf, i, type)) return false;
    if (type != static_cast<std::uint8_t>(MessageType::Receipt)) return false;
    std::uint32_t sess2 = 0;
    if (!read_u32le(buf, i, sess2)) return false;
    out.session_id = SessionId(sess2);
    if (!read_u32le(buf, i, out.up_to_offset_ms)) return false;
    return i == buf.size;
}

} // namespace ble


