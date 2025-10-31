
//! @file src/ble/drivers/NativeBleDriver.cpp
//! @brief Native (container-friendly) BLE driver using in-memory queues (UDP-like).
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef ARDUINO

#include <jenlib/ble/BleDriver.h>
#include <jenlib/ble/Messages.h>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <utility>

namespace jenlib::ble {

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
    //! @brief Constructor.
    //! @param local_device_id Local device identifier for this instance.
    explicit NativeBleDriver(DeviceId local_device_id) : local_device_id_(local_device_id), initialized_(false) {}

    //! @brief Initialize the BLE driver and establish connections (Arduino-friendly).
    //! @return true if initialization succeeded, false otherwise.
    bool begin() override {
        initialized_ = true;
        if (connection_callback_) {
            connection_callback_(true);
        }
        return true;
    }

    //! @brief Cleanup BLE driver resources and close connections (Arduino-friendly).
    void end() override {
        std::lock_guard<std::mutex> lock(mutex_);
        inbox_.clear();
        initialized_ = false;
        if (connection_callback_) {
            connection_callback_(false);
        }
    }

    // initialize/cleanup removed in favor of begin/end

    //! @brief Check if the driver is connected and ready for communication.
    //! @return true if connected and ready, false otherwise.
    bool is_connected() const override {
        return initialized_;
    }

    //! @brief Get the local device identifier for this driver instance.
    //! @return The device ID that identifies this driver instance.
    DeviceId get_local_device_id() const override {
        return local_device_id_;
    }

    void advertise(DeviceId device_id, BlePayload payload) override {
        if (!initialized_) {
            return;
        }
        // Broadcast goes to broker inbox (device_id 0 reserved for broker)
        enqueue(DeviceId(0u), payload_with_sender(device_id, std::move(payload)));
    }

    void send_to(DeviceId device_id, BlePayload payload) override {
        if (!initialized_) {
            return;
        }
        enqueue(device_id, std::move(payload));
    }

    bool receive(DeviceId self_id, BlePayload &out_payload) override {
        if (!initialized_) {
            return false;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        auto &q = inbox_[self_id.value()];
        if (q.empty()) {
            return false;
        }
        out_payload = std::move(q.front());
        q.pop_front();
        return true;
    }

    void poll() override {
        // Native driver doesn't need event processing - messages are queued directly
        // This method is provided for Arduino compatibility
    }

    void set_message_callback(BleMessageCallback callback) override {
        message_callback_ = std::move(callback);
    }

    void clear_message_callback() override {
        message_callback_ = nullptr;
    }

    void set_start_broadcast_callback(StartBroadcastCallback callback) override {
        start_broadcast_callback_ = std::move(callback);
    }

    void set_reading_callback(ReadingCallback callback) override {
        reading_callback_ = std::move(callback);
    }

    void set_receipt_callback(ReceiptCallback callback) override {
        receipt_callback_ = std::move(callback);
    }

    void clear_type_specific_callbacks() override {
        start_broadcast_callback_ = nullptr;
        reading_callback_ = nullptr;
        receipt_callback_ = nullptr;
    }

    void set_connection_callback(ConnectionCallback callback) override {
        connection_callback_ = std::move(callback);
    }

    void clear_connection_callback() override {
        connection_callback_ = nullptr;
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
    //! @note Swallows exceptions on the queue operations.
    //!       I am willing to accept this as a failure mode for BLE which is
    //!       inherently unreliable.
    void enqueue(DeviceId dest, BlePayload payload) {
        // Extract sender ID from payload if it has the sender marker
        DeviceId sender_id = extract_sender_id(payload);

        // Try type-specific callbacks first
        if (try_type_specific_callbacks(sender_id, payload)) {
            return;  // Handled by type-specific callback
        }

        // Fallback to generic callback
        if (message_callback_) {
            message_callback_(sender_id, payload);
            return;
        }

        // Fallback to queuing for polling-based access
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

    //! @brief Extract sender ID from payload if it contains the sender marker.
    //! @param payload The payload to extract sender ID from.
    //! @return The sender ID, or DeviceId(0) if not found.
    DeviceId extract_sender_id(const BlePayload& payload) {
        if (payload.size >= 5 && payload.bytes[0] == kSenderIdMarker) {
            // Extract 4-byte LE device ID from payload
            std::uint32_t sender_value = static_cast<std::uint32_t>(payload.bytes[1]) |
                                       (static_cast<std::uint32_t>(payload.bytes[2]) << 8) |
                                       (static_cast<std::uint32_t>(payload.bytes[3]) << 16) |
                                       (static_cast<std::uint32_t>(payload.bytes[4]) << 24);
            return DeviceId(sender_value);
        }
        return DeviceId(0);  // Unknown sender
    }

    //! @brief Try to handle payload with type-specific callbacks.
    //! @param sender_id The sender device ID.
    //! @param payload The received payload.
    //! @return true if handled by type-specific callback, false otherwise.
    bool try_type_specific_callbacks(DeviceId sender_id, const BlePayload& payload) {
        // Try StartBroadcastMsg
        if (start_broadcast_callback_) {
            StartBroadcastMsg start_msg;
            if (StartBroadcastMsg::deserialize(payload, start_msg)) {
                start_broadcast_callback_(sender_id, start_msg);
                return true;
            }
        }

        // Try ReadingMsg
        if (reading_callback_) {
            ReadingMsg reading;
            if (ReadingMsg::deserialize(payload, reading)) {
                reading_callback_(sender_id, reading);
                return true;
            }
        }

        // Try ReceiptMsg
        if (receipt_callback_) {
            ReceiptMsg receipt;
            if (ReceiptMsg::deserialize(payload, receipt)) {
                receipt_callback_(sender_id, receipt);
                return true;
            }
        }

        return false;  // No type-specific callback handled this message
    }

    DeviceId local_device_id_;  //!< Local device identifier.
    bool initialized_;  //!< Initialization state.
    BleMessageCallback message_callback_;  //!< Callback for received messages.
    StartBroadcastCallback start_broadcast_callback_;  //!< Callback for StartBroadcast messages.
    ReadingCallback reading_callback_;  //!< Callback for Reading messages.
    ReceiptCallback receipt_callback_;  //!< Callback for Receipt messages.
    ConnectionCallback connection_callback_;  //!< Callback for connection state changes.
    std::unordered_map<std::uint32_t, std::deque<BlePayload>> inbox_;  //!< Inbox for received payloads.
    std::mutex mutex_;  //!< Mutex for inbox.
};

}  // namespace jenlib::ble

#endif  // ARDUINO
