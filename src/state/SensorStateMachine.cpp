//! @file src/state/SensorStateMachine.cpp
//! @brief Sensor state machine implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/state/SensorStateMachine.h>
#include <jenlib/events/EventTypes.h>
#include <jenlib/time/Time.h>

namespace jenlib::state {

SensorStateMachine::SensorStateMachine()
    : StateMachine<SensorState>(SensorState::kDisconnected)
    , current_session_id_(0)
    , broker_id_(0)
    , measurement_interval_ms_(1000)
    , session_start_time_ms_(0)
    , session_active_(false) {
}

bool SensorStateMachine::handle_event(const jenlib::events::Event& event) {
    switch (event.type) {
        case jenlib::events::EventType::kConnectionStateChange:
            return handle_connection_change(event.data != 0);

        case jenlib::events::EventType::kTimeTick:
            if (is_in_state(SensorState::kRunning)) {
                return handle_measurement_timer();
            }
            break;

        case jenlib::events::EventType::kBleMessage:
            // BLE messages are handled by specific methods
            break;

        default:
            break;
    }

    return false;
}

bool SensorStateMachine::handle_connection_change(bool connected) {
    if (connected && is_in_state(SensorState::kDisconnected)) {
        return transition_to(SensorState::kWaiting);
    } else if (!connected && !is_in_state(SensorState::kDisconnected)) {
        return transition_to(SensorState::kDisconnected);
    }

    return false;
}

bool SensorStateMachine::handle_start_broadcast(
    jenlib::ble::DeviceId sender_id,
    const jenlib::ble::StartBroadcastMsg& msg) {
    if (!is_in_state(SensorState::kWaiting)) {
        return false;  // Can only start broadcast when waiting
    }

    start_measurement_session(msg);
    return transition_to(SensorState::kRunning);
}

bool SensorStateMachine::handle_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg& msg) {
    if (!is_in_state(SensorState::kRunning) || msg.session_id != current_session_id_) {
        return false;  // Can only receive receipts when running and session matches
    }

    // Process receipt - could purge buffered readings here
    return true;
}

bool SensorStateMachine::handle_session_end() {
    if (!is_in_state(SensorState::kRunning)) {
        return false;  // Can only end session when running
    }

    stop_measurement_session();
    return transition_to(SensorState::kWaiting);
}

bool SensorStateMachine::handle_measurement_timer() {
    if (!is_in_state(SensorState::kRunning)) {
        return false;  // Can only take measurements when running
    }

    take_measurement();
    return true;
}

void SensorStateMachine::handle_error(std::string_view error_message) {
    StateMachine<SensorState>::handle_error(error_message);
    transition_to(SensorState::kError);
}

void SensorStateMachine::handle_recovery() {
    StateMachine<SensorState>::handle_recovery();
    transition_to(SensorState::kDisconnected);
}

bool SensorStateMachine::is_valid_transition(SensorState from_state, SensorState to_state) const {
    // Define valid state transitions
    switch (from_state) {
        case SensorState::kDisconnected:
            return to_state == SensorState::kWaiting || to_state == SensorState::kError;

        case SensorState::kWaiting:
            return to_state == SensorState::kRunning ||
                   to_state == SensorState::kDisconnected ||
                   to_state == SensorState::kError;

        case SensorState::kRunning:
            return to_state == SensorState::kWaiting ||
                   to_state == SensorState::kDisconnected ||
                   to_state == SensorState::kError;

        case SensorState::kError:
            return to_state == SensorState::kDisconnected;

        default:
            return false;
    }
}

void SensorStateMachine::on_state_entry(SensorState state) {
    switch (state) {
        case SensorState::kWaiting:
            // Register for start broadcast messages
            break;

        case SensorState::kRunning:
            // Start measurement timer
            session_active_ = true;
            break;

        case SensorState::kDisconnected:
            // Clean up any active sessions
            if (session_active_) {
                stop_measurement_session();
            }
            break;

        case SensorState::kError:
            // Stop all activities
            if (session_active_) {
                stop_measurement_session();
            }
            break;
    }
}

void SensorStateMachine::on_state_exit(SensorState state) {
    switch (state) {
        case SensorState::kRunning:
            // Stop measurement timer
            session_active_ = false;
            break;

        default:
            break;
    }
}

void SensorStateMachine::on_state_do(SensorState state) {
    switch (state) {
        case SensorState::kRunning:
            // Periodic actions while running
            break;

        default:
            break;
    }
}

void SensorStateMachine::start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg) {
    current_session_id_ = msg.session_id;
    broker_id_ = msg.device_id;
    session_start_time_ms_ = jenlib::time::Time::now();
    session_active_ = true;
}

void SensorStateMachine::stop_measurement_session() {
    session_active_ = false;
    current_session_id_ = jenlib::ble::SessionId(0);
    broker_id_ = jenlib::ble::DeviceId(0);
}

void SensorStateMachine::take_measurement() {
    // This would be implemented by the application
    // The state machine just manages the timing and state
}

}  // namespace jenlib::state

