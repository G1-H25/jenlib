//! @file src/state/StateMachine.cpp
//! @brief Event-driven state machine implementation
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include "jenlib/state/StateMachine.h"
#include <algorithm>

namespace jenlib::state {

StateMachine::StateMachine() 
    : current_state_id_(kInvalidStateId) {
}

StateMachine::~StateMachine() {
    clear();
}

bool StateMachine::add_state(StateId state_id, const std::string& state_name) {
    if (state_id == kInvalidStateId || has_state(state_id)) {
        return false;
    }
    
    auto state_info = std::make_unique<StateInfo>(state_id, state_name);
    states_[state_id] = std::move(state_info);
    
    return true;
}

bool StateMachine::remove_state(StateId state_id) {
    if (state_id == kInvalidStateId) {
        return false;
    }
    
    auto it = states_.find(state_id);
    if (it == states_.end()) {
        return false;
    }
    
    // If this is the current state, clear current state
    if (current_state_id_ == state_id) {
        current_state_id_ = kInvalidStateId;
    }
    
    states_.erase(it);
    return true;
}

bool StateMachine::set_state(StateId state_id) {
    if (state_id == kInvalidStateId || !has_state(state_id)) {
        return false;
    }
    
    return transition_to(state_id);
}

StateId StateMachine::get_current_state() const {
    return current_state_id_;
}

std::string StateMachine::get_current_state_name() const {
    const StateInfo* state_info = find_state(current_state_id_);
    return state_info ? state_info->name : "";
}

bool StateMachine::set_state_entry_callback(StateId state_id, StateCallback callback) {
    StateInfo* state_info = find_state(state_id);
    if (!state_info) {
        return false;
    }
    
    state_info->on_enter = std::move(callback);
    return true;
}

bool StateMachine::set_state_exit_callback(StateId state_id, StateCallback callback) {
    StateInfo* state_info = find_state(state_id);
    if (!state_info) {
        return false;
    }
    
    state_info->on_exit = std::move(callback);
    return true;
}

bool StateMachine::set_event_handler(StateId state_id, jenlib::events::EventType event_type, EventHandler handler) {
    StateInfo* state_info = find_state(state_id);
    if (!state_info) {
        return false;
    }
    
    state_info->event_handlers[event_type] = std::move(handler);
    return true;
}

bool StateMachine::handle_event(const jenlib::events::Event& event) {
    if (current_state_id_ == kInvalidStateId) {
        return false;
    }
    
    StateInfo* state_info = find_state(current_state_id_);
    if (!state_info) {
        return false;
    }
    
    auto it = state_info->event_handlers.find(event.type);
    if (it == state_info->event_handlers.end()) {
        return false;
    }
    
    // Call the event handler
    if (it->second) {
        try {
            return it->second(event);
        } catch (...) {
            // Event handler exception - return false to indicate event was not handled
            return false;
        }
    }
    
    return false;
}

void StateMachine::set_transition_callback(StateTransitionCallback callback) {
    transition_callback_ = std::move(callback);
}

std::size_t StateMachine::get_state_count() const {
    return states_.size();
}

bool StateMachine::has_state(StateId state_id) const {
    return states_.find(state_id) != states_.end();
}

void StateMachine::clear() {
    // Call exit callback for current state if it exists
    if (current_state_id_ != kInvalidStateId) {
        StateInfo* state_info = find_state(current_state_id_);
        if (state_info && state_info->on_exit) {
            try {
                state_info->on_exit();
            } catch (...) {
                // Exit callback exception - continue with cleanup
            }
        }
    }
    
    current_state_id_ = kInvalidStateId;
    states_.clear();
    transition_callback_ = nullptr;
}

StateInfo* StateMachine::find_state(StateId state_id) {
    auto it = states_.find(state_id);
    return (it != states_.end()) ? it->second.get() : nullptr;
}

const StateInfo* StateMachine::find_state(StateId state_id) const {
    auto it = states_.find(state_id);
    return (it != states_.end()) ? it->second.get() : nullptr;
}

bool StateMachine::transition_to(StateId new_state_id) {
    StateId old_state_id = current_state_id_;
    
    // Call exit callback for current state if it exists
    if (old_state_id != kInvalidStateId) {
        StateInfo* old_state_info = find_state(old_state_id);
        if (old_state_info && old_state_info->on_exit) {
            try {
                old_state_info->on_exit();
            } catch (...) {
                // Exit callback exception - continue with transition
            }
        }
    }
    
    // Update current state
    current_state_id_ = new_state_id;
    
    // Call entry callback for new state if it exists
    StateInfo* new_state_info = find_state(new_state_id);
    if (new_state_info && new_state_info->on_enter) {
        try {
            new_state_info->on_enter();
        } catch (...) {
            // Entry callback exception - continue with transition
        }
    }
    
    // Call transition callback if it exists
    if (transition_callback_) {
        try {
            transition_callback_(old_state_id, new_state_id);
        } catch (...) {
            // Transition callback exception - continue
        }
    }
    
    return true;
}

} // namespace jenlib::state
