//! @file include/jenlib/state/BrokerStateMachine.h
//! @brief Broker state machine for BLE broker applications
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_STATE_BROKERSTATEMACHINE_H_
#define INCLUDE_JENLIB_STATE_BROKERSTATEMACHINE_H_

#include <jenlib/state/StateMachine.h>
#include <jenlib/ble/Ids.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/events/EventTypes.h>

namespace jenlib::state {

//! @brief Broker state enumeration
enum class BrokerState : std::uint8_t {
    kNoSession = 0x01,      //!< No active session
    kSessionStarted = 0x02,  //!< Session started, collecting readings
    kError = 0x03           //!< Error state
};

//! @brief Broker state machine
//! @details
//! Manages the lifecycle of a BLE broker from session initiation through data collection.
//! Handles state transitions based on backend commands and sensor responses.
class BrokerStateMachine : public StateMachine<BrokerState> {
 public:
    //! @brief Constructor
    BrokerStateMachine();

    //! @brief Handle generic event
    bool handle_event(const jenlib::events::Event& event) override;

    //! @brief Handle start command from backend
    //! @param sensor_id ID of the target sensor
    //! @param session_id ID for the new session
    //! @return true if command was processed, false otherwise
    bool handle_start_command(jenlib::ble::DeviceId sensor_id, jenlib::ble::SessionId session_id);

    //! @brief Handle reading message from sensor
    //! @param sender_id ID of the sensor
    //! @param msg Reading message
    //! @return true if message was processed, false otherwise
    bool handle_reading(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReadingMsg& msg);

    //! @brief Handle session end
    //! @return true if session was ended, false otherwise
    bool handle_session_end();

    //! @brief Handle backend timeout
    //! @return true if timeout was handled, false otherwise
    bool handle_backend_timeout();

    //! @brief Handle error condition
    void handle_error(std::string_view error_message) override;

    //! @brief Handle recovery from error
    void handle_recovery() override;

    //! @brief Get current session ID
    jenlib::ble::SessionId get_current_session_id() const { return current_session_id_; }

    //! @brief Get target sensor ID
    jenlib::ble::DeviceId get_target_sensor_id() const { return target_sensor_id_; }

    //! @brief Check if session is active
    bool is_session_active() const { return is_in_state(BrokerState::kSessionStarted); }

    //! @brief Get number of readings received
    std::uint32_t get_reading_count() const { return reading_count_; }

    //! @brief Get session start time
    std::uint32_t get_session_start_time_ms() const { return session_start_time_ms_; }

 protected:
    //! @brief Check if transition is valid
    bool is_valid_transition(BrokerState from_state, BrokerState to_state) const override;

    //! @brief Get initial state
    BrokerState get_initial_state() const override { return BrokerState::kNoSession; }

 private:
    //! @brief Handle state entry actions
    void on_state_entry(BrokerState state);

    //! @brief Handle state exit actions
    void on_state_exit(BrokerState state);

    //! @brief Handle state do actions
    void on_state_do(BrokerState state);

    //! @brief Start new session
    void start_session(jenlib::ble::DeviceId sensor_id, jenlib::ble::SessionId session_id);

    //! @brief End current session
    void end_session();

    //! @brief Send receipt to sensor
    void send_receipt(jenlib::ble::DeviceId sensor_id, std::uint32_t up_to_offset_ms);

    //! @brief Process received reading
    void process_reading(const jenlib::ble::ReadingMsg& msg);

    // State data
    jenlib::ble::SessionId current_session_id_;
    jenlib::ble::DeviceId target_sensor_id_;
    std::uint32_t session_start_time_ms_;
    std::uint32_t reading_count_;
    std::uint32_t last_receipt_offset_ms_;
    bool session_active_;
};

}  // namespace jenlib::state

#endif  // INCLUDE_JENLIB_STATE_BROKERSTATEMACHINE_H_

