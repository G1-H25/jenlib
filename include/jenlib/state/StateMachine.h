//! @file include/jenlib/state/StateMachine.h
//! @brief Base state machine interface and utilities
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_STATE_STATEMACHINE_H_
#define INCLUDE_JENLIB_STATE_STATEMACHINE_H_

#include <cstdint>
#include <functional>
#include <string_view>
#include <jenlib/events/EventTypes.h>

namespace jenlib::state {

//! @brief State action types for entry/exit callbacks
enum class StateAction : std::uint8_t {
    kEntry = 0x01,  //!< State entry action
    kExit = 0x02,   //!< State exit action
    kDo = 0x03      //!< State do action (while in state)
};

//! @brief Base state machine interface
//! @details
//! Provides common functionality for all state machines in the system.
//! Handles event processing, state transitions, and error recovery.
template<typename StateType>
class StateMachine {
public:
    using State = StateType;
    using StateCallback = std::function<void(StateAction action, State state)>;
    using ErrorCallback = std::function<void(std::string_view error)>;

    //! @brief Constructor
    StateMachine(State initial_state) : current_state_(initial_state), previous_state_(initial_state) {}

    //! @brief Virtual destructor
    virtual ~StateMachine() = default;

    //! @brief Get current state
    State get_current_state() const { return current_state_; }

    //! @brief Get previous state
    State get_previous_state() const { return previous_state_; }

    //! @brief Check if in specific state
    bool is_in_state(State state) const { return current_state_ == state; }

    //! @brief Set state action callback
    void set_state_action_callback(StateCallback callback) {
        state_callback_ = std::move(callback);
    }

    //! @brief Set error callback
    void set_error_callback(ErrorCallback callback) {
        error_callback_ = std::move(callback);
    }

    //! @brief Handle generic event
    //! @param event The event to process
    //! @return true if event was handled, false otherwise
    virtual bool handle_event(const jenlib::events::Event& event) = 0;

    //! @brief Handle error condition
    //! @param error_message Description of the error
    virtual void handle_error(std::string_view error_message) {
        if (error_callback_) {
            error_callback_(error_message);
        }
        // Default error handling - transition to error state if available
        // Subclasses should override this
    }

    //! @brief Handle recovery from error
    virtual void handle_recovery() {
        // Default recovery - return to initial state
        // Subclasses should override this
    }

protected:
    //! @brief Transition to new state
    //! @param new_state The state to transition to
    //! @return true if transition was successful, false otherwise
    bool transition_to(State new_state) {
        if (new_state == current_state_) {
            return true; // Already in target state
        }

        // Call exit action for current state
        if (state_callback_) {
            state_callback_(StateAction::kExit, current_state_);
        }

        // Update states
        previous_state_ = current_state_;
        current_state_ = new_state;

        // Call entry action for new state
        if (state_callback_) {
            state_callback_(StateAction::kEntry, current_state_);
        }

        return true;
    }

    //! @brief Check if transition is valid
    //! @param from_state Source state
    //! @param to_state Target state
    //! @return true if transition is valid, false otherwise
    virtual bool is_valid_transition(State from_state, State to_state) const = 0;

    //! @brief Get initial state
    virtual State get_initial_state() const = 0;

private:
    State current_state_;
    State previous_state_;
    StateCallback state_callback_;
    ErrorCallback error_callback_;
};

} // namespace jenlib::state

#endif // INCLUDE_JENLIB_STATE_STATEMACHINE_H_