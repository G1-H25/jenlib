//! @file smoke_tests/StateMachineSmokeTests.cpp
//! @brief State machine smoke tests for jenlib
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <unity.h>
#include <atomic>
#include <vector>
#include "smoke_tests/SmokeTestSuites.h"
#include "jenlib/state/SensorStateMachine.h"
#include "jenlib/events/EventDispatcher.h"
#include "jenlib/events/EventTypes.h"
#include "jenlib/time/Time.h"
#include "jenlib/time/drivers/NativeTimeDriver.h"
#include "smoke_tests/PlatformMocks.h"

//! @section Test State Tracking
static std::atomic<int> state_entry_count{0};
static std::atomic<int> state_exit_count{0};
static std::atomic<int> state_do_count{0};
static std::vector<jenlib::state::SensorState> state_transitions;
static std::atomic<bool> measurement_taken{false};

//! @section Test Callbacks

//! @brief Test callback for state machine state actions
//! @param action The state action (entry, exit, or do)
//! @param state The sensor state
void test_state_callback(jenlib::state::StateAction action, jenlib::state::SensorState state) {
    switch (action) {
        case jenlib::state::StateAction::kEntry:
            state_entry_count++;
            break;
        case jenlib::state::StateAction::kExit:
            state_exit_count++;
            break;
        case jenlib::state::StateAction::kDo:
            state_do_count++;
            break;
    }
    state_transitions.push_back(state);
}

//! @section Test Setup and Teardown

//! @brief Unity test setup function - resets test state and initializes services
void setUp(void) {
    //! Reset test state
    state_entry_count = 0;
    state_exit_count = 0;
    state_do_count = 0;
    state_transitions.clear();
    measurement_taken = false;

    //! Initialize time service with mock driver
    static smoke_tests::MockTimeDriver mock_time_driver;
    jenlib::time::Time::setDriver(&mock_time_driver);
    jenlib::time::Time::initialize();
    jenlib::time::Time::clear_all_timers();

    //! Initialize event dispatcher
    jenlib::events::EventDispatcher::initialize();
    jenlib::events::EventDispatcher::clear_all_callbacks();
}

//! @brief Unity test teardown function - cleans up after each test
void tearDown(void) {
    //! Clean up
    jenlib::events::EventDispatcher::clear_all_callbacks();
    jenlib::time::Time::clear_all_timers();
}

//! @section State Machine Tests

//! @test Validates sensor state machine initialization and initial state
void test_sensor_state_machine_initialization(void) {
    //! ARRANGE: Create state machine
    jenlib::state::SensorStateMachine state_machine;

    //! ACT: No action needed - testing initial state

    //! ASSERT: Verify initial state
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kDisconnected));
    TEST_ASSERT_FALSE(state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0, state_machine.get_current_session_id().value());
}

//! @test Validates sensor state machine connection transition
void test_sensor_state_connection_transition(void) {
    //! ARRANGE: Create state machine and set callback
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, state_machine.get_current_state());

    //! ACT: Handle connection change
    bool transitioned = state_machine.handle_connection_change(true);

    //! ASSERT: Verify transition to waiting state
    TEST_ASSERT_TRUE(transitioned);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kWaiting));
    TEST_ASSERT_EQUAL(1, state_entry_count.load());
    TEST_ASSERT_EQUAL(1, state_exit_count.load());
}

//! @test Validates sensor state machine disconnection transition
void test_sensor_state_disconnection_transition(void) {
    //! ARRANGE: Create state machine, set callback, and connect
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, state_machine.get_current_state());

    //! ACT: Handle disconnection
    bool transitioned = state_machine.handle_connection_change(false);

    //! ASSERT: Verify transition back to disconnected state
    TEST_ASSERT_TRUE(transitioned);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kDisconnected));
    TEST_ASSERT_EQUAL(2, state_entry_count.load());
    TEST_ASSERT_EQUAL(2, state_exit_count.load());
}

//! @test Validates sensor session start functionality
void test_sensor_session_start(void) {
    //! ARRANGE: Create state machine, set callback, and connect
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_connection_change(true);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, state_machine.get_current_state());

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Handle start broadcast message
    bool started = state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ASSERT: Verify session started and state transitioned to running
    TEST_ASSERT_TRUE(started);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kRunning));
    TEST_ASSERT_TRUE(state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0x1234, state_machine.get_current_session_id().value());
}

//! @test Validates sensor session end functionality
void test_sensor_session_end(void) {
    //! ARRANGE: Create state machine, set callback, connect, and start session
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };
    state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_session_active());

    //! ACT: Handle session end
    bool ended = state_machine.handle_session_end();

    //! ASSERT: Verify session ended and state transitioned back to waiting
    TEST_ASSERT_TRUE(ended);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kWaiting, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kWaiting));
    TEST_ASSERT_FALSE(state_machine.is_session_active());
    TEST_ASSERT_EQUAL(0, state_machine.get_current_session_id().value());
}

//! @test Validates sensor measurement handling functionality
void test_sensor_measurement_handling(void) {
    //! ARRANGE: Create state machine, set callback, connect, and start session
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x87654321),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, state_machine.get_current_state());

    //! ACT: Handle measurement timer
    bool measurement_handled = state_machine.handle_measurement_timer();

    //! ASSERT: Verify measurement was handled
    TEST_ASSERT_TRUE(measurement_handled);
    //! Note: The actual measurement taking is implemented by the application
    //! The state machine just manages the timing and state
}

//! @test Validates sensor receipt handling functionality
void test_sensor_receipt_handling(void) {
    //! ARRANGE: Create state machine, set callback, connect, and start session
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x87654321),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, state_machine.get_current_state());

    //! ARRANGE: Prepare receipt message
    jenlib::ble::ReceiptMsg receipt_msg{
        .session_id = jenlib::ble::SessionId(0x1234),
        .up_to_offset_ms = 1000
    };

    //! ACT: Handle receipt message
    bool receipt_handled = state_machine.handle_receipt(jenlib::ble::DeviceId(0x87654321), receipt_msg);

    //! ASSERT: Verify receipt was handled
    TEST_ASSERT_TRUE(receipt_handled);
}

//! @test Validates sensor error handling functionality
void test_sensor_error_handling(void) {
    //! ARRANGE: Create state machine and set callback
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);

    //! ACT: Handle error
    state_machine.handle_error("Test error");

    //! ASSERT: Verify transition to error state
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kError, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kError));
}

//! @test Validates sensor error recovery functionality
void test_sensor_error_recovery(void) {
    //! ARRANGE: Create state machine, set callback, and trigger error
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_error("Test error");
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kError, state_machine.get_current_state());

    //! ACT: Handle recovery
    state_machine.handle_recovery();

    //! ASSERT: Verify transition back to disconnected state
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, state_machine.get_current_state());
    TEST_ASSERT_TRUE(state_machine.is_in_state(jenlib::state::SensorState::kDisconnected));
}

//! @test Validates sensor invalid transition handling
void test_sensor_invalid_transitions(void) {
    //! ARRANGE: Create state machine and set callback
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, state_machine.get_current_state());

    //! ARRANGE: Prepare start broadcast message
    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    //! ACT: Try to start broadcast while disconnected
    bool started = state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);

    //! ASSERT: Verify start broadcast failed when disconnected
    TEST_ASSERT_FALSE(started);  //  Should fail when disconnected
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kDisconnected, state_machine.get_current_state());
}

//! @test Validates sensor invalid receipt handling
void test_sensor_invalid_receipt_handling(void) {
    //! ARRANGE: Create state machine, set callback, connect, and start session
    jenlib::state::SensorStateMachine state_machine;
    state_machine.set_state_action_callback(test_state_callback);
    state_machine.handle_connection_change(true);

    jenlib::ble::StartBroadcastMsg start_msg{
        .device_id = jenlib::ble::DeviceId(0x12345678),
        .session_id = jenlib::ble::SessionId(0x1234)
    };

    state_machine.handle_start_broadcast(jenlib::ble::DeviceId(0x87654321), start_msg);
    TEST_ASSERT_EQUAL(jenlib::state::SensorState::kRunning, state_machine.get_current_state());

    //! ARRANGE: Prepare receipt message with wrong session ID
    jenlib::ble::ReceiptMsg wrong_receipt{
        .session_id = jenlib::ble::SessionId(0x5678),  //  Wrong session ID
        .up_to_offset_ms = 1000
    };

    //! ACT: Try to handle receipt with wrong session ID
    bool receipt_handled = state_machine.handle_receipt(jenlib::ble::DeviceId(0x87654321), wrong_receipt);

    //! ASSERT: Verify receipt handling failed with wrong session ID
    TEST_ASSERT_FALSE(receipt_handled);  //  Should fail with wrong session ID
}

//! @section Test Runner

//! @brief Main function to run all state machine smoke tests
int main(void) {
    UNITY_BEGIN();

    // State Machine Initialization Tests
    RUN_TEST(test_sensor_state_machine_initialization);

    // State Machine Transition Tests
    RUN_TEST(test_sensor_state_connection_transition);
    RUN_TEST(test_sensor_state_disconnection_transition);

    // State Machine Session Management Tests
    RUN_TEST(test_sensor_session_start);
    RUN_TEST(test_sensor_session_end);

    // State Machine Measurement and Receipt Tests
    RUN_TEST(test_sensor_measurement_handling);
    RUN_TEST(test_sensor_receipt_handling);

    // State Machine Error Handling Tests
    RUN_TEST(test_sensor_error_handling);
    RUN_TEST(test_sensor_error_recovery);

    // State Machine Invalid Transition Tests
    RUN_TEST(test_sensor_invalid_transitions);
    RUN_TEST(test_sensor_invalid_receipt_handling);

    return UNITY_END();
}

