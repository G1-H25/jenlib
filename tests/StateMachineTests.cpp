//! @file tests/StateMachineTests.cpp
//! @brief Tests for state machine functionality
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include "unity.h"
#include <jenlib/state/StateMachine.h>
#include <jenlib/state/SensorStateMachine.h>
#include <jenlib/state/BrokerStateMachine.h>
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/events/EventTypes.h>

using namespace jenlib::state;
using namespace jenlib::ble;
using namespace jenlib::events;

//! @test test_state_machine_initialization
//! @brief Verifies state machine initializes to correct initial state
void test_state_machine_initialization(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    BrokerStateMachine broker_sm;

    // Act
    // (No action needed - testing initialization)

    // Assert
    TEST_ASSERT_EQUAL(SensorState::kDisconnected, sensor_sm.get_current_state());
    TEST_ASSERT_EQUAL(BrokerState::kNoSession, broker_sm.get_current_state());
}

//! @test test_sensor_disconnected_to_waiting_transition
//! @brief Verifies sensor transitions from disconnected to waiting on connection
void test_sensor_disconnected_to_waiting_transition(void) {
    // Arrange
    SensorStateMachine sensor_sm;

    // Act
    sensor_sm.handle_event(Event(EventType::kConnectionStateChange, 0, 1));

    // Assert
    TEST_ASSERT_EQUAL(SensorState::kWaiting, sensor_sm.get_current_state());
}

//! @test test_sensor_waiting_to_running_transition
//! @brief Verifies sensor transitions from waiting to running on start broadcast
void test_sensor_waiting_to_running_transition(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    StartBroadcastMsg start_msg{DeviceId(0x1234), SessionId(0x5678)};
    sensor_sm.handle_event(Event(EventType::kConnectionStateChange, 0, 1)); // Connect first

    // Act
    sensor_sm.handle_start_broadcast(DeviceId(0x1234), start_msg);

    // Assert
    TEST_ASSERT_EQUAL(SensorState::kRunning, sensor_sm.get_current_state());
}

//! @test test_sensor_running_to_waiting_transition
//! @brief Verifies sensor transitions from running to waiting on session end
void test_sensor_running_to_waiting_transition(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    StartBroadcastMsg start_msg{DeviceId(0x1234), SessionId(0x5678)};
    sensor_sm.handle_event(Event(EventType::kConnectionStateChange, 0, 1)); // Connect
    sensor_sm.handle_start_broadcast(DeviceId(0x1234), start_msg); // Start session

    // Act
    sensor_sm.handle_session_end();

    // Assert
    TEST_ASSERT_EQUAL(SensorState::kWaiting, sensor_sm.get_current_state());
}

//! @test test_broker_no_session_to_session_started_transition
//! @brief Verifies broker transitions from no session to session started on start command
void test_broker_no_session_to_session_started_transition(void) {
    // Arrange
    BrokerStateMachine broker_sm;

    // Act
    broker_sm.handle_start_command(DeviceId(0x1234), SessionId(0x5678));

    // Assert
    TEST_ASSERT_EQUAL(BrokerState::kSessionStarted, broker_sm.get_current_state());
}

//! @test test_broker_session_started_to_no_session_transition
//! @brief Verifies broker transitions from session started to no session on session end
void test_broker_session_started_to_no_session_transition(void) {
    // Arrange
    BrokerStateMachine broker_sm;
    broker_sm.handle_start_command(DeviceId(0x1234), SessionId(0x5678)); // Start session first

    // Act
    broker_sm.handle_session_end();

    // Assert
    TEST_ASSERT_EQUAL(BrokerState::kNoSession, broker_sm.get_current_state());
}

//! @test test_invalid_start_broadcast_while_disconnected
//! @brief Verifies start broadcast is rejected when sensor is disconnected
void test_invalid_start_broadcast_while_disconnected(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    StartBroadcastMsg start_msg{DeviceId(0x1234), SessionId(0x5678)};

    // Act
    bool result = sensor_sm.handle_start_broadcast(DeviceId(0x1234), start_msg);

    // Assert
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(SensorState::kDisconnected, sensor_sm.get_current_state());
}

//! @test test_state_entry_exit_actions
//! @brief Verifies entry and exit actions are called during state transitions
void test_state_entry_exit_actions(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    bool entry_called = false;
    bool exit_called = false;

    sensor_sm.set_state_action_callback([&](StateAction action, SensorState state) {
        if (action == StateAction::kEntry && state == SensorState::kWaiting) {
            entry_called = true;
        }
        if (action == StateAction::kExit && state == SensorState::kDisconnected) {
            exit_called = true;
        }
    });

    // Act
    sensor_sm.handle_event(Event(EventType::kConnectionStateChange, 0, 1));

    // Assert
    TEST_ASSERT_TRUE(entry_called);
    TEST_ASSERT_TRUE(exit_called);
}

//! @test test_start_broadcast_rejected_when_disconnected
//! @brief Verifies start broadcast is rejected when sensor is in disconnected state
void test_start_broadcast_rejected_when_disconnected(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    StartBroadcastMsg start_msg{DeviceId(0x1234), SessionId(0x5678)};

    // Act
    bool result = sensor_sm.handle_start_broadcast(DeviceId(0x1234), start_msg);

    // Assert
    TEST_ASSERT_FALSE(result);
}

//! @test test_start_broadcast_accepted_when_waiting
//! @brief Verifies start broadcast is accepted when sensor is in waiting state
void test_start_broadcast_accepted_when_waiting(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    StartBroadcastMsg start_msg{DeviceId(0x1234), SessionId(0x5678)};
    sensor_sm.handle_event(Event(EventType::kConnectionStateChange, 0, 1)); // Connect first

    // Act
    bool result = sensor_sm.handle_start_broadcast(DeviceId(0x1234), start_msg);

    // Assert
    TEST_ASSERT_TRUE(result);
}

//! @test test_start_broadcast_device_id_validation
//! @brief Verifies that device ID validation is handled at the application level
//! @note The state machine doesn't validate device IDs - that's done in the callback
void test_start_broadcast_device_id_validation(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    StartBroadcastMsg start_msg{DeviceId(0x9999), SessionId(0x5678)}; // Different device ID
    sensor_sm.handle_event(Event(EventType::kConnectionStateChange, 0, 1)); // Connect first

    // Act
    // The state machine will accept this (it doesn't validate device IDs)
    bool result = sensor_sm.handle_start_broadcast(DeviceId(0x1234), start_msg);

    // Assert
    // State machine accepts it because it only validates state, not device ID
    TEST_ASSERT_TRUE(result);
    // Note: In the actual application, device ID validation happens in the callback
    // before calling the state machine, so invalid device IDs never reach here
}

//! @test test_state_machine_error_transition
//! @brief Verifies state machine transitions to error state on error
void test_state_machine_error_transition(void) {
    // Arrange
    SensorStateMachine sensor_sm;

    // Act
    sensor_sm.handle_error("Test error");

    // Assert
    TEST_ASSERT_EQUAL(SensorState::kError, sensor_sm.get_current_state());
}

//! @test test_state_machine_error_recovery
//! @brief Verifies state machine recovers from error state
void test_state_machine_error_recovery(void) {
    // Arrange
    SensorStateMachine sensor_sm;
    sensor_sm.handle_error("Test error"); // Put in error state first

    // Act
    sensor_sm.handle_recovery();

    // Assert
    TEST_ASSERT_EQUAL(SensorState::kDisconnected, sensor_sm.get_current_state());
}

