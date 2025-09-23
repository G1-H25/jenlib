//! @file smoke_tests/PlatformMocks.h
//! @brief Platform mocks for smoke testing jenlib on Native platform
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef SMOKE_TESTS_PLATFORMMOCKS_H_
#define SMOKE_TESTS_PLATFORMMOCKS_H_

#include <jenlib/time/TimeDriver.h>
#include <jenlib/ble/BleDriver.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/ble/Ids.h>
#include <jenlib/events/EventTypes.h>
#include <chrono>
#include <queue>
#include <map>
#include <mutex>
#include <thread>
#include <atomic>

namespace smoke_tests {

//! @brief Mock time driver for smoke testing
//! @details Simulates time advancement for deterministic testing
class MockTimeDriver : public jenlib::time::TimeDriver {
public:
    MockTimeDriver() : start_time_(std::chrono::steady_clock::now()), current_time_ms_(0) {}

    std::uint32_t now() override {
        std::lock_guard<std::mutex> lock(time_mutex_);
        return current_time_ms_;
    }

    void delay(std::uint32_t delay_ms) override {
        std::lock_guard<std::mutex> lock(time_mutex_);
        current_time_ms_ += delay_ms;
    }

    //! @brief Advance time for testing
    void advance_time(std::uint32_t ms) {
        std::lock_guard<std::mutex> lock(time_mutex_);
        current_time_ms_ += ms;
    }

    //! @brief Set time for testing
    void set_time(std::uint32_t ms) {
        std::lock_guard<std::mutex> lock(time_mutex_);
        current_time_ms_ = ms;
    }

    //! @brief Reset time to zero
    void reset() {
        std::lock_guard<std::mutex> lock(time_mutex_);
        current_time_ms_ = 0;
    }

private:
    std::chrono::steady_clock::time_point start_time_;
    std::uint32_t current_time_ms_;
    std::mutex time_mutex_;
};

//! @brief Mock BLE driver for smoke testing
//! @details Simulates BLE communication using in-memory message queues
class MockBleDriver : public jenlib::ble::BleDriver {
public:
    MockBleDriver() : initialized_(false), connected_(false), local_device_id_(0) {}

    bool begin() override {
        initialized_ = true;
        connected_ = true;

        // Trigger connection callback if set
        if (connection_callback_) {
            connection_callback_(true);
        }

        return true;
    }

    void end() override {
        initialized_ = false;
        connected_ = false;
        clear_all_messages();
    }

    bool is_connected() const override {
        return initialized_ && connected_;
    }

    jenlib::ble::DeviceId get_local_device_id() const override {
        return local_device_id_;
    }

    void set_local_device_id(jenlib::ble::DeviceId device_id) {
        local_device_id_ = device_id;
    }

    void advertise(jenlib::ble::DeviceId device_id, jenlib::ble::BlePayload payload) override {
        if (!initialized_) return;

        std::lock_guard<std::mutex> lock(message_mutex_);
        // Simulate broadcast - add to all device inboxes
        for (auto& [target_id, inbox] : device_inboxes_) {
            if (target_id != device_id) { // Don't send to self
                inbox.emplace(device_id, std::move(payload));
            }
        }
    }

    void send_to(jenlib::ble::DeviceId device_id, jenlib::ble::BlePayload payload) override {
        if (!initialized_) return;

        std::lock_guard<std::mutex> lock(message_mutex_);
        if (device_inboxes_.find(device_id) != device_inboxes_.end()) {
            device_inboxes_[device_id].emplace(local_device_id_, std::move(payload));
        }
    }

    bool receive(jenlib::ble::DeviceId self_id, jenlib::ble::BlePayload &out_payload) override {
        if (!initialized_) return false;

        std::lock_guard<std::mutex> lock(message_mutex_);
        if (device_inboxes_.find(self_id) != device_inboxes_.end() &&
            !device_inboxes_[self_id].empty()) {
            auto message = std::move(device_inboxes_[self_id].front());
            device_inboxes_[self_id].pop();
            out_payload = std::move(message.payload);
            return true;
        }
        return false;
    }

    void poll() override {
        std::lock_guard<std::mutex> lock(message_mutex_);
        if (device_inboxes_.find(local_device_id_) != device_inboxes_.end()) {
            auto& inbox = device_inboxes_[local_device_id_];
            while (!inbox.empty()) {
                auto message = std::move(inbox.front());
                inbox.pop();

                // Try to deserialize and call appropriate typed callback
                bool handled = false;

                // Try StartBroadcastMsg
                if (start_broadcast_callback_) {
                    jenlib::ble::StartBroadcastMsg start_msg;
                    if (jenlib::ble::StartBroadcastMsg::deserialize(message.payload, start_msg)) {
                        start_broadcast_callback_(message.sender_id, start_msg);
                        handled = true;
                    }
                }

                // Try ReadingMsg
                if (!handled && reading_callback_) {
                    jenlib::ble::ReadingMsg reading_msg;
                    if (jenlib::ble::ReadingMsg::deserialize(message.payload, reading_msg)) {
                        reading_callback_(message.sender_id, reading_msg);
                        handled = true;
                    }
                }

                // Try ReceiptMsg
                if (!handled && receipt_callback_) {
                    jenlib::ble::ReceiptMsg receipt_msg;
                    if (jenlib::ble::ReceiptMsg::deserialize(message.payload, receipt_msg)) {
                        receipt_callback_(message.sender_id, receipt_msg);
                        handled = true;
                    }
                }

                // Fall back to generic message callback if no typed callback handled it
                if (!handled && message_callback_) {
                    message_callback_(message.sender_id, message.payload);
                }
            }
        }
    }

    // Callback management
    void set_message_callback(jenlib::ble::BleMessageCallback callback) override {
        message_callback_ = std::move(callback);
    }

    void clear_message_callback() override {
        message_callback_ = nullptr;
    }

    void set_start_broadcast_callback(jenlib::ble::StartBroadcastCallback callback) override {
        start_broadcast_callback_ = std::move(callback);
    }

    void set_reading_callback(jenlib::ble::ReadingCallback callback) override {
        reading_callback_ = std::move(callback);
    }

    void set_receipt_callback(jenlib::ble::ReceiptCallback callback) override {
        receipt_callback_ = std::move(callback);
    }

    void clear_type_specific_callbacks() override {
        start_broadcast_callback_ = nullptr;
        reading_callback_ = nullptr;
        receipt_callback_ = nullptr;
    }

    void set_connection_callback(jenlib::ble::ConnectionCallback callback) override {
        connection_callback_ = std::move(callback);
    }

    void clear_connection_callback() override {
        connection_callback_ = nullptr;
    }

    // Test helper methods
    void register_device(jenlib::ble::DeviceId device_id) {
        std::lock_guard<std::mutex> lock(message_mutex_);
        device_inboxes_[device_id] = std::queue<Message>();
    }

    void unregister_device(jenlib::ble::DeviceId device_id) {
        std::lock_guard<std::mutex> lock(message_mutex_);
        device_inboxes_.erase(device_id);
    }

    void simulate_connection_loss() {
        connected_ = false;
        if (connection_callback_) {
            connection_callback_(false);
        }
    }

    void simulate_connection_restore() {
        connected_ = true;
        if (connection_callback_) {
            connection_callback_(true);
        }
    }

    void clear_all_messages() {
        std::lock_guard<std::mutex> lock(message_mutex_);
        for (auto& [device_id, inbox] : device_inboxes_) {
            while (!inbox.empty()) {
                inbox.pop();
            }
        }
    }

    std::size_t get_message_count(jenlib::ble::DeviceId device_id) const {
        std::lock_guard<std::mutex> lock(message_mutex_);
        if (device_inboxes_.find(device_id) != device_inboxes_.end()) {
            return device_inboxes_.at(device_id).size();
        }
        return 0;
    }

private:
    struct Message {
        jenlib::ble::DeviceId sender_id;
        jenlib::ble::BlePayload payload;

        // Constructor
        Message(jenlib::ble::DeviceId id, jenlib::ble::BlePayload&& p)
            : sender_id(id), payload(std::move(p)) {}

        // Move constructor and assignment
        Message(Message&& other) noexcept
            : sender_id(other.sender_id), payload(std::move(other.payload)) {}

        Message& operator=(Message&& other) noexcept {
            if (this != &other) {
                sender_id = other.sender_id;
                payload = std::move(other.payload);
            }
            return *this;
        }

        // Disable copy
        Message(const Message&) = delete;
        Message& operator=(const Message&) = delete;
    };

    bool initialized_;
    bool connected_;
    jenlib::ble::DeviceId local_device_id_;

    std::map<jenlib::ble::DeviceId, std::queue<Message>> device_inboxes_;
    mutable std::mutex message_mutex_;

    // Callbacks
    jenlib::ble::BleMessageCallback message_callback_;
    jenlib::ble::StartBroadcastCallback start_broadcast_callback_;
    jenlib::ble::ReadingCallback reading_callback_;
    jenlib::ble::ReceiptCallback receipt_callback_;
    jenlib::ble::ConnectionCallback connection_callback_;
};

//! @brief Mock sensor reading functions for smoke testing
class MockSensorReadings {
public:
    static float read_temperature_sensor() {
        static float base_temp = 22.5f;
        static float variation = 0.0f;
        variation += 0.1f;
        if (variation > 2.0f) variation = -2.0f;
        return base_temp + variation;
    }

    static float read_humidity_sensor() {
        static float base_humidity = 45.0f;
        static float variation = 0.0f;
        variation += 0.2f;
        if (variation > 5.0f) variation = -5.0f;
        return base_humidity + variation;
    }

    static void reset_sensor_state() {
        // Reset static variables for deterministic testing
        // Note: This is a limitation of the current mock implementation
        // In a real test, you might want to use dependency injection
    }
};

//! @brief Mock broker behavior for smoke testing
class MockBroker {
public:
    MockBroker(jenlib::ble::DeviceId broker_id, MockBleDriver* ble_driver)
        : broker_id_(broker_id), ble_driver_(ble_driver), session_active_(false) {
        ble_driver_->register_device(broker_id_);
        ble_driver_->set_local_device_id(broker_id_);
    }

    void start_session(jenlib::ble::DeviceId sensor_id, jenlib::ble::SessionId session_id) {
        if (session_active_) {
            return; // Already have an active session
        }

        jenlib::ble::StartBroadcastMsg msg{
            .device_id = sensor_id,
            .session_id = session_id
        };

        jenlib::ble::BlePayload payload;
        if (jenlib::ble::StartBroadcastMsg::serialize(msg, payload)) {
            ble_driver_->send_to(sensor_id, std::move(payload));
            session_active_ = true;
            current_session_id_ = session_id;
            current_sensor_id_ = sensor_id;
        }
    }

    void stop_session() {
        session_active_ = false;
        current_session_id_ = jenlib::ble::SessionId(0);
        current_sensor_id_ = jenlib::ble::DeviceId(0);
    }

    void process_messages() {
        jenlib::ble::BlePayload payload;
        while (ble_driver_->receive(broker_id_, payload)) {
            // Process received messages (e.g., reading messages)
            // For smoke testing, we just acknowledge receipt
            if (session_active_) {
                jenlib::ble::ReceiptMsg receipt{
                    .session_id = current_session_id_,
                    .up_to_offset_ms = 1000 // Acknowledge up to 1 second
                };

                jenlib::ble::BlePayload receipt_payload;
                if (jenlib::ble::ReceiptMsg::serialize(receipt, receipt_payload)) {
                    ble_driver_->send_to(current_sensor_id_, std::move(receipt_payload));
                }
            }
        }
    }

    bool is_session_active() const { return session_active_; }
    jenlib::ble::SessionId get_current_session_id() const { return current_session_id_; }
    jenlib::ble::DeviceId get_current_sensor_id() const { return current_sensor_id_; }

private:
    jenlib::ble::DeviceId broker_id_;
    MockBleDriver* ble_driver_;
    bool session_active_;
    jenlib::ble::SessionId current_session_id_;
    jenlib::ble::DeviceId current_sensor_id_;
};

} // namespace smoke_tests

#endif // SMOKE_TESTS_PLATFORMMOCKS_H_

