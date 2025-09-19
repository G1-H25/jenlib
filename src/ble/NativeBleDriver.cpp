//! @file src/ble/NativeBleDriver.cpp
//! @brief Native (container-friendly) BLE driver using in-memory queues (UDP-like).

#include <jenlib/ble/BleDriver.h>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <utility>

namespace ble {

// Native driver constants
constexpr std::uint8_t kSenderIdMarker = 0xFF;
constexpr std::size_t kMaxQueueSize = 100u;  // Maximum messages per device inbox


//! @class NativeBleDriver
//! @brief Native BLE driver implementation.
//! @details Uses in-memory queues for broadcast and point-to-point messaging.
//! Queues are bounded to prevent memory exhaustion in resource-constrained environments.
//! When queues are full, oldest messages are dropped to maintain system stability.
class NativeBleDriver : public BleDriver {
public:
    void advertise(DeviceId device_id, BlePayload payload) override {
        //! @brief Send a best-effort broadcast from a device.
        //! @param device_id Logical sender identity.
        //! @param payload Serialized message bytes (moved).
        // Broadcast goes to broker inbox (device_id 0 reserved for broker)
        enqueue(DeviceId(0u), payload_with_sender(device_id, std::move(payload)));
    }

    //! @brief Send a directed, point-to-point message.
    //! @param device_id Destination identity.
    //! @param payload Serialized message bytes (moved).
    void send_to(DeviceId device_id, BlePayload payload) override {
        enqueue(device_id, std::move(payload));
    }

    
    //! @brief Poll next received payload for a local device.
    //! @param self_id Local identity being polled.
    //! @param out_payload Destination buffer for the payload.
    //! @return True if a payload was returned, false if none available.
    bool receive(DeviceId self_id, BlePayload &out_payload) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto &q = inbox_[self_id.value()];
        if (q.empty()) return false;
        out_payload = std::move(q.front());
        q.pop_front();
        return true;
    }

private:
    //! @brief Create a payload with a sender ID.
    //! @param sender_id Sender identity.
    //! @param payload Serialized message bytes (moved).
    //! @return Payload with sender ID.
    static BlePayload payload_with_sender(DeviceId sender_id, BlePayload payload) {
        BlePayload buf;
        // Prefix marker to indicate presence of sender id in shim header
        buf.append_u8(kSenderIdMarker);
        // Embed raw 4-byte LE device id (no checksum) for shim routing only
        const std::uint32_t v = sender_id.value();
        buf.append_u8(static_cast<std::uint8_t>(v & 0xFF));
        buf.append_u8(static_cast<std::uint8_t>((v >> 8) & 0xFF));
        buf.append_u8(static_cast<std::uint8_t>((v >> 16) & 0xFF));
        buf.append_u8(static_cast<std::uint8_t>((v >> 24) & 0xFF));
        buf.append_raw(payload.bytes.data(), payload.size);
        return buf;
    }

    //! @brief Enqueue a payload for a destination device.
    //! @param dest Destination identity.
    //! @param payload Serialized message bytes (moved into queue).
    //! @post Inbox is updated with the payload. If inbox is full, oldest payload is dropped.
    //! @note Swallows exceptions on the queue operations. I am willing to accept this as a failure mode for BLE which is inherently unreliable
    void enqueue(DeviceId dest, BlePayload payload) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        try {
            auto &queue = inbox_[dest.value()];
            
            //! Drop oldest messages if queue is at capacity
            while (queue.size() >= kMaxQueueSize) {
                queue.pop_front();
            }
            
            queue.push_back(std::move(payload));
        } catch (const std::bad_alloc&) {
            //! Memory allocation failed - swallow and move on
        } catch (...) {
            //! Swallow any other unexpected exceptions
        }
    }

    std::unordered_map<std::uint32_t, std::deque<BlePayload>> inbox_; //!< Inbox for received payloads.
    std::mutex mutex_; //!< Mutex for inbox.
};

} // namespace ble


