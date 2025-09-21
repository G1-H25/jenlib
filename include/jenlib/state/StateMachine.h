//! @file include/jenlib/state/StateMachine.h
//! @brief Event-driven state machine for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#ifndef INCLUDE_JENLIB_STATE_STATEMACHINE_H_
#define INCLUDE_JENLIB_STATE_STATEMACHINE_H_

#include <functional>
#include <unordered_map>
#include <vector>
#include "jenlib/events/EventTypes.h"

namespace jenlib::state {

//! @brief State ID type for identifying states
using StateId = std::uint32_t;

//! @brief Invalid state ID constant
constexpr StateId kInvalidStateId = 0;

//! @brief State transition callback function type
using StateTransitionCallback = std::function<void(StateId from_state, StateId to_state)>;

//! @brief Event handler function type
using EventHandler = std::function<bool(const jenlib::events::Event& event)>;

//! @brief State entry/exit callback function type
using StateCallback = std::function<void()>;

//! @brief State information structure
struct StateInfo {
    StateId id;
    std::string name;
    StateCallback on_enter;
    StateCallback on_exit;
    std::unordered_map<jenlib::events::EventType, EventHandler> event_handlers;
    
    StateInfo(StateId state_id, const std::string& state_name)
        : id(state_id), name(state_name) {}
};

//! @brief Event-driven state machine
class StateMachine {
public:
    //! @brief Constructor
    StateMachine();
    
    //! @brief Destructor
    ~StateMachine();
    
    //! @brief Add a state to the state machine
    //! @param state_id Unique identifier for the state
    //! @param state_name Human-readable name for the state
    //! @return true if state was added successfully, false if state already exists
    bool add_state(StateId state_id, const std::string& state_name);
    
    //! @brief Remove a state from the state machine
    //! @param state_id The state ID to remove
    //! @return true if state was removed successfully, false if state not found
    bool remove_state(StateId state_id);
    
    //! @brief Set the current state
    //! @param state_id The state ID to transition to
    //! @return true if transition was successful, false if state not found
    bool set_state(StateId state_id);
    
    //! @brief Get the current state ID
    //! @return Current state ID, or kInvalidStateId if no state is set
    StateId get_current_state() const;
    
    //! @brief Get the current state name
    //! @return Current state name, or empty string if no state is set
    std::string get_current_state_name() const;
    
    //! @brief Set callback for state entry
    //! @param state_id The state ID
    //! @param callback Function to call when entering the state
    //! @return true if callback was set successfully, false if state not found
    bool set_state_entry_callback(StateId state_id, StateCallback callback);
    
    //! @brief Set callback for state exit
    //! @param state_id The state ID
    //! @param callback Function to call when exiting the state
    //! @return true if callback was set successfully, false if state not found
    bool set_state_exit_callback(StateId state_id, StateCallback callback);
    
    //! @brief Set event handler for a specific state and event type
    //! @param state_id The state ID
    //! @param event_type The event type to handle
    //! @param handler Function to call when the event occurs in this state
    //! @return true if handler was set successfully, false if state not found
    bool set_event_handler(StateId state_id, jenlib::events::EventType event_type, EventHandler handler);
    
    //! @brief Handle an event in the current state
    //! @param event The event to handle
    //! @return true if event was handled, false if no handler was found
    bool handle_event(const jenlib::events::Event& event);
    
    //! @brief Set callback for state transitions
    //! @param callback Function to call when transitioning between states
    void set_transition_callback(StateTransitionCallback callback);
    
    //! @brief Get the number of states in the state machine
    //! @return Number of states
    std::size_t get_state_count() const;
    
    //! @brief Check if a state exists
    //! @param state_id The state ID to check
    //! @return true if state exists, false otherwise
    bool has_state(StateId state_id) const;
    
    //! @brief Clear all states and reset the state machine
    void clear();

private:
    //! @brief Find state info by ID
    //! @param state_id The state ID to find
    //! @return Pointer to state info, or nullptr if not found
    StateInfo* find_state(StateId state_id);
    
    //! @brief Find state info by ID (const version)
    //! @param state_id The state ID to find
    //! @return Pointer to state info, or nullptr if not found
    const StateInfo* find_state(StateId state_id) const;
    
    //! @brief Transition to a new state
    //! @param new_state_id The state ID to transition to
    //! @return true if transition was successful, false otherwise
    bool transition_to(StateId new_state_id);
    
    //! @brief Current state ID
    StateId current_state_id_;
    
    //! @brief State storage
    std::unordered_map<StateId, std::unique_ptr<StateInfo>> states_;
    
    //! @brief State transition callback
    StateTransitionCallback transition_callback_;
};

} // namespace jenlib::state

#endif // INCLUDE_JENLIB_STATE_STATEMACHINE_H_
