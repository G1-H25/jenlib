//! @file include/jenlib/state/SensorStateMachine.h
//! @brief Sensor state machine for BLE sensor applications
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_STATE_SENSORSTATEMACHINE_H_
#define INCLUDE_JENLIB_STATE_SENSORSTATEMACHINE_H_

#include <jenlib/state/StateMachine.h>
#include <jenlib/ble/Ids.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/events/EventTypes.h>

//! @namespace jenlib::state
//! @brief State machine implementations for sensor and broker roles.
//! @details
//! Provides state machines that manage the lifecycle of sensor
//! and broker operations. State machines ensure proper state
//! transitions and validate operations based on current state.
//!
//! @par Sensor State Flow:
//! @dot
//! digraph sensor_states {
//!     Disconnected -> Waiting [label="BLE Connected"];
//!     Waiting -> Running [label="StartBroadcast"];
//!     Running -> Waiting [label="Session End"];
//!     Waiting -> Disconnected [label="BLE Disconnected"];
//!     Running -> Disconnected [label="BLE Disconnected"];
//!     Disconnected -> Error [label="Connection Error"];
//!     Waiting -> Error [label="Invalid Message"];
//!     Running -> Error [label="Session Error"];
//! }
//! @enddot
//!
//! @par Usage Example:
//! @code
//! #include <jenlib/state/SensorStateMachine.h>
//!
//! jenlib::state::SensorStateMachine sensor_state_machine;
//!
//! // Handle BLE connection changes
//! void callback_connection(bool connected) {
//!     sensor_state_machine.handle_connection_change(connected);
//! }
//!
//! // Handle start broadcast messages
//! void callback_start(jenlib::ble::DeviceId sender_id,
//!                    const jenlib::ble::StartBroadcastMsg &msg) {
//!     bool success = sensor_state_machine.handle_start_broadcast(sender_id, msg);
//!     if (success) {
//!         start_measurement_session(msg);
//!     }
//! }
//!
//! // Check if session is active before taking readings
//! void take_reading() {
//!     if (sensor_state_machine.is_session_active()) {
//!         // Take and broadcast reading
//!     }
//! }
//! @endcode
//!
//! @par Integration with Other Systems:
//! - Uses @ref jenlib::events for state change notifications
//! - Coordinates with @ref jenlib::ble for message handling
//! - Manages timing through @ref jenlib::time services
//!
//! @see @ref state_example "State Machine Example" for complete usage patterns
//! @see jenlib::state::StateMachine for base state machine functionality
namespace jenlib::state {

//! @brief Sensor state enumeration
enum class SensorState : std::uint8_t {
    kDisconnected = 0x01,  //!< Not connected to broker
    kWaiting = 0x02,       //!< Connected, waiting for start command
    kRunning = 0x03,       //!< Actively broadcasting measurements
    kError = 0x04          //!< Error state
};

//! @brief Sensor state machine
//! @details
//! Manages the lifecycle of a BLE sensor from connection through measurement broadcasting.
//! Handles state transitions based on BLE events and timer events.
class SensorStateMachine : public StateMachine<SensorState> {
 public:
    //! @brief Constructor
    SensorStateMachine();

    //! @brief Handle generic event
    bool handle_event(const jenlib::events::Event& event) override;

    //! @brief Handle BLE connection state change
    //! @param connected true if connected, false if disconnected
    //! @return true if state changed, false otherwise
    bool handle_connection_change(bool connected);

    //! @brief Handle start broadcast message
    //! @param sender_id ID of the sender (broker)
    //! @param msg Start broadcast message
    //! @return true if message was processed, false otherwise
    //! @note Device ID validation should be done at the application level before calling this method
    bool handle_start_broadcast(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg& msg);

    //! @brief Handle receipt message
    //! @param sender_id ID of the sender (broker)
    //! @param msg Receipt message
    //! @return true if message was processed, false otherwise
    bool handle_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg& msg);

    //! @brief Handle session end
    //! @return true if session was ended, false otherwise
    bool handle_session_end();

    //! @brief Handle measurement timer event
    //! @return true if measurement was taken, false otherwise
    bool handle_measurement_timer();

    //! @brief Handle error condition
    void handle_error(std::string_view error_message) override;

    //! @brief Handle recovery from error
    void handle_recovery() override;

    //! @brief Get current session ID
    jenlib::ble::SessionId get_current_session_id() const { return current_session_id_; }

    //! @brief Check if session is active
    bool is_session_active() const { return is_in_state(SensorState::kRunning); }

    //! @brief Get measurement interval
    std::uint32_t get_measurement_interval_ms() const { return measurement_interval_ms_; }

    //! @brief Set measurement interval
    void set_measurement_interval_ms(std::uint32_t interval_ms) { measurement_interval_ms_ = interval_ms; }

 protected:
    //! @brief Check if transition is valid
    bool is_valid_transition(SensorState from_state, SensorState to_state) const override;

    //! @brief Get initial state
    SensorState get_initial_state() const override { return SensorState::kDisconnected; }

 private:
    //! @brief Handle state entry actions
    void on_state_entry(SensorState state);

    //! @brief Handle state exit actions
    void on_state_exit(SensorState state);

    //! @brief Handle state do actions
    void on_state_do(SensorState state);

    //! @brief Start measurement session
    void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg);

    //! @brief Stop measurement session
    void stop_measurement_session();

    //! @brief Take and broadcast measurement
    void take_measurement();

    // State data
    jenlib::ble::SessionId current_session_id_;
    jenlib::ble::DeviceId broker_id_;
    std::uint32_t measurement_interval_ms_;
    std::uint32_t session_start_time_ms_;
    bool session_active_;
};

}  // namespace jenlib::state

#endif  // INCLUDE_JENLIB_STATE_SENSORSTATEMACHINE_H_

