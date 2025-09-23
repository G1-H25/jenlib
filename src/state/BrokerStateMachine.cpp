//! @file src/state/BrokerStateMachine.cpp
//! @brief Broker state machine implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <jenlib/state/BrokerStateMachine.h>
#include <jenlib/events/EventTypes.h>
#include <jenlib/time/Time.h>

namespace jenlib::state {

BrokerStateMachine::BrokerStateMachine()
    : StateMachine<BrokerState>(BrokerState::kNoSession)
    , current_session_id_(0)
    , target_sensor_id_(0)
    , session_start_time_ms_(0)
    , reading_count_(0)
    , last_receipt_offset_ms_(0)
    , session_active_(false) {
}

bool BrokerStateMachine::handle_event(const jenlib::events::Event& event) {
    switch (event.type) {
        case jenlib::events::EventType::kBleMessage:
            // BLE messages are handled by specific methods
            break;

        case jenlib::events::EventType::kTimeTick:
            if (is_in_state(BrokerState::kSessionStarted)) {
                // Could implement session timeout here
            }
            break;

        default:
            break;
    }

    return false;
}

bool BrokerStateMachine::handle_start_command(jenlib::ble::DeviceId sensor_id, jenlib::ble::SessionId session_id) {
    if (!is_in_state(BrokerState::kNoSession)) {
        return false; // Can only start new session when no session is active
    }

    start_session(sensor_id, session_id);
    return transition_to(BrokerState::kSessionStarted);
}

bool BrokerStateMachine::handle_reading(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReadingMsg& msg) {
    if (!is_in_state(BrokerState::kSessionStarted) ||
        sender_id != target_sensor_id_ ||
        msg.session_id != current_session_id_) {
        return false; // Can only receive readings when session is active and IDs match
    }

    process_reading(msg);
    return true;
}

bool BrokerStateMachine::handle_session_end() {
    if (!is_in_state(BrokerState::kSessionStarted)) {
        return false; // Can only end session when session is active
    }

    end_session();
    return transition_to(BrokerState::kNoSession);
}

bool BrokerStateMachine::handle_backend_timeout() {
    if (!is_in_state(BrokerState::kSessionStarted)) {
        return false; // Can only timeout when session is active
    }

    // Handle timeout - could end session or retry
    end_session();
    return transition_to(BrokerState::kNoSession);
}

void BrokerStateMachine::handle_error(std::string_view error_message) {
    StateMachine<BrokerState>::handle_error(error_message);
    transition_to(BrokerState::kError);
}

void BrokerStateMachine::handle_recovery() {
    StateMachine<BrokerState>::handle_recovery();
    transition_to(BrokerState::kNoSession);
}

bool BrokerStateMachine::is_valid_transition(BrokerState from_state, BrokerState to_state) const {
    // Define valid state transitions
    switch (from_state) {
        case BrokerState::kNoSession:
            return to_state == BrokerState::kSessionStarted || to_state == BrokerState::kError;

        case BrokerState::kSessionStarted:
            return to_state == BrokerState::kNoSession || to_state == BrokerState::kError;

        case BrokerState::kError:
            return to_state == BrokerState::kNoSession;

        default:
            return false;
    }
}

void BrokerStateMachine::on_state_entry(BrokerState state) {
    switch (state) {
        case BrokerState::kSessionStarted:
            // Register for reading messages from target sensor
            session_active_ = true;
            break;

        case BrokerState::kNoSession:
            // Clean up session data
            if (session_active_) {
                end_session();
            }
            break;

        case BrokerState::kError:
            // Stop all activities
            if (session_active_) {
                end_session();
            }
            break;
    }
}

void BrokerStateMachine::on_state_exit(BrokerState state) {
    switch (state) {
        case BrokerState::kSessionStarted:
            // Unregister from reading messages
            session_active_ = false;
            break;

        default:
            break;
    }
}

void BrokerStateMachine::on_state_do(BrokerState state) {
    switch (state) {
        case BrokerState::kSessionStarted:
            // Periodic actions while session is active
            // Could implement periodic receipt sending here
            break;

        default:
            break;
    }
}

void BrokerStateMachine::start_session(jenlib::ble::DeviceId sensor_id, jenlib::ble::SessionId session_id) {
    current_session_id_ = session_id;
    target_sensor_id_ = sensor_id;
    session_start_time_ms_ = jenlib::time::Time::now();
    reading_count_ = 0;
    last_receipt_offset_ms_ = 0;
    session_active_ = true;
}

void BrokerStateMachine::end_session() {
    session_active_ = false;
    current_session_id_ = jenlib::ble::SessionId(0);
    target_sensor_id_ = jenlib::ble::DeviceId(0);
    reading_count_ = 0;
    last_receipt_offset_ms_ = 0;
}

void BrokerStateMachine::send_receipt(jenlib::ble::DeviceId sensor_id, std::uint32_t up_to_offset_ms) {
    // This would be implemented by the application
    // The state machine just manages the state and timing
    last_receipt_offset_ms_ = up_to_offset_ms;
}

void BrokerStateMachine::process_reading(const jenlib::ble::ReadingMsg& msg) {
    reading_count_++;

    // Could implement receipt sending logic here
    // For now, just track the reading
}

} // namespace jenlib::state

