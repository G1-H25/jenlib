//! @file examples/arduino/ble_sensor_callback/main.cpp
//! @brief Arduino BLE sensor using callback-based event system
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (simbachu@gmail.com)

#include <Arduino.h>
#include <jenlib/time/Time.h>
#include <jenlib/measurement/Measurement.h>
#include <jenlib/gpio/GPIO.h>
#include <jenlib/ble/Ble.h>
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/state/StateMachine.h>

// Device configuration
constexpr jenlib::ble::DeviceId kDeviceId = jenlib::ble::DeviceId(0x12345678);
constexpr std::uint32_t kBroadcastInterval = 5000; // 5 seconds
constexpr std::uint32_t kConnectionTimeout = 30000; // 30 seconds
constexpr std::uint32_t kRetryInterval = 2000; // 2 seconds

// State machine states
enum class SensorState : jenlib::state::StateId {
    kDisconnected = 1,
    kConnecting = 2,
    kConnected = 3,
    kPassiveWaiting = 4,
    kBroadcasting = 5
};

// Global state management
jenlib::state::StateMachine g_state_machine;
jenlib::ble::SessionId g_current_session_id = jenlib::ble::SessionId(0);
jenlib::time::TimerId g_broadcast_timer = jenlib::time::kInvalidTimerId;
jenlib::time::TimerId g_connection_timeout = jenlib::time::kInvalidTimerId;
jenlib::time::TimerId g_retry_timer = jenlib::time::kInvalidTimerId;
std::uint32_t g_session_start_time = 0;

// Forward declarations for callback functions
void on_connection_changed(bool connected);
void on_start_broadcast(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg& msg);
void on_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg& msg);
void on_connection_timeout();
void on_retry_connection();
void on_broadcast_timer();
void on_measurement_ready(const Measurement& measurement);

// State machine event handlers
bool handle_connection_event(const jenlib::events::Event& event);
bool handle_ble_message_event(const jenlib::events::Event& event);
bool handle_timer_event(const jenlib::events::Event& event);

// State entry/exit callbacks
void on_enter_disconnected();
void on_enter_connecting();
void on_enter_connected();
void on_enter_passive_waiting();
void on_enter_broadcasting();

void on_exit_disconnected();
void on_exit_connecting();
void on_exit_connected();
void on_exit_passive_waiting();
void on_exit_broadcasting();

// Connection state callback
void on_connection_changed(bool connected) {
    Serial.print("Connection state changed: ");
    Serial.println(connected ? "CONNECTED" : "DISCONNECTED");
    
    // Dispatch connection state change event
    jenlib::events::EventDispatcher::dispatch_event({
        .type = jenlib::events::EventType::kConnectionStateChange,
        .timestamp = jenlib::time::now(),
        .data = connected ? 1 : 0
    });
}

// Start broadcast message callback
void on_start_broadcast(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg& msg) {
    Serial.print("Received start broadcast from: 0x");
    Serial.println(static_cast<uint32_t>(sender_id), HEX);
    
    if (msg.device_id == kDeviceId) {
        g_current_session_id = msg.session_id;
        g_session_start_time = jenlib::time::now();
        
        Serial.print("Session ID: 0x");
        Serial.println(static_cast<uint32_t>(g_current_session_id), HEX);
        
        // Dispatch BLE message event
        jenlib::events::EventDispatcher::dispatch_event({
            .type = jenlib::events::EventType::kBleMessage,
            .timestamp = jenlib::time::now(),
            .data = static_cast<std::uint32_t>(jenlib::events::EventType::kBleMessage)
        });
    }
}

// Receipt message callback
void on_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg& msg) {
    Serial.print("Received receipt from: 0x");
    Serial.print(static_cast<uint32_t>(sender_id), HEX);
    Serial.print(" - Session: 0x");
    Serial.print(static_cast<uint32_t>(msg.session_id), HEX);
    Serial.print(" - Up to offset: ");
    Serial.println(msg.up_to_offset_ms);
    
    if (msg.session_id == g_current_session_id) {
        // Purge buffered readings up to the acknowledged offset
        jenlib::measurement::purge_buffered_readings(msg.session_id, msg.up_to_offset_ms);
    }
}

// Timer callbacks
void on_connection_timeout() {
    Serial.println("Connection timeout - transitioning to disconnected");
    g_connection_timeout = jenlib::time::kInvalidTimerId;
    
    // Dispatch timer event
    jenlib::events::EventDispatcher::dispatch_event({
        .type = jenlib::events::EventType::kTimeTick,
        .timestamp = jenlib::time::now(),
        .data = static_cast<std::uint32_t>(jenlib::events::EventType::kTimeTick)
    });
}

void on_retry_connection() {
    Serial.println("Retrying connection...");
    g_retry_timer = jenlib::time::kInvalidTimerId;
    
    // Dispatch timer event
    jenlib::events::EventDispatcher::dispatch_event({
        .type = jenlib::events::EventType::kTimeTick,
        .timestamp = jenlib::time::now(),
        .data = static_cast<std::uint32_t>(jenlib::events::EventType::kTimeTick)
    });
}

void on_broadcast_timer() {
    if (g_state_machine.get_current_state() != static_cast<jenlib::state::StateId>(SensorState::kBroadcasting)) {
        return;
    }
    
    // Read sensor measurement
    Measurement measurement = jenlib::measurement::read_sensor();
    
    // Calculate time offset since session start
    std::uint32_t current_time = jenlib::time::now();
    std::uint32_t offset_ms = current_time - g_session_start_time;
    
    // Create reading message
    ReadingMsg reading_msg = {
        .sender_id = kDeviceId,
        .session_id = g_current_session_id,
        .offset_ms = offset_ms,
        .temperature_c_centi = measurement.temperature_c_centi,
        .humidity_bp = measurement.humidity_bp
    };
    
    // Broadcast the reading
    jenlib::ble::BLE::broadcast_reading(kDeviceId, reading_msg);
    
    Serial.print("Broadcast reading - Temp: ");
    Serial.print(measurement.temperature_c_centi / 100.0f);
    Serial.print("°C, Humidity: ");
    Serial.print(measurement.humidity_bp / 100.0f);
    Serial.print("%, Offset: ");
    Serial.print(offset_ms);
    Serial.println("ms");
}

// State machine event handlers
bool handle_connection_event(const jenlib::events::Event& event) {
    bool connected = (event.data != 0);
    
    if (connected) {
        g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kConnected));
    } else {
        g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kDisconnected));
    }
    
    return true;
}

bool handle_ble_message_event(const jenlib::events::Event& event) {
    jenlib::state::StateId current_state = g_state_machine.get_current_state();
    
    if (current_state == static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting)) {
        g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kBroadcasting));
        return true;
    }
    
    return false;
}

bool handle_timer_event(const jenlib::events::Event& event) {
    jenlib::state::StateId current_state = g_state_machine.get_current_state();
    
    if (current_state == static_cast<jenlib::state::StateId>(SensorState::kConnecting)) {
        // Connection timeout or retry
        g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kDisconnected));
        return true;
    }
    
    return false;
}

// State entry callbacks
void on_enter_disconnected() {
    Serial.println("State: DISCONNECTED");
    
    // Cancel any active timers
    if (g_broadcast_timer != jenlib::time::kInvalidTimerId) {
        jenlib::time::cancel_callback(g_broadcast_timer);
        g_broadcast_timer = jenlib::time::kInvalidTimerId;
    }
    
    if (g_connection_timeout != jenlib::time::kInvalidTimerId) {
        jenlib::time::cancel_callback(g_connection_timeout);
        g_connection_timeout = jenlib::time::kInvalidTimerId;
    }
    
    // Schedule retry connection
    g_retry_timer = jenlib::time::schedule_callback(
        kRetryInterval,
        on_retry_connection,
        false // one-shot
    );
}

void on_enter_connecting() {
    Serial.println("State: CONNECTING");
    
    // Set connection timeout
    g_connection_timeout = jenlib::time::schedule_callback(
        kConnectionTimeout,
        on_connection_timeout,
        false // one-shot
    );
}

void on_enter_connected() {
    Serial.println("State: CONNECTED");
    
    // Cancel retry timer
    if (g_retry_timer != jenlib::time::kInvalidTimerId) {
        jenlib::time::cancel_callback(g_retry_timer);
        g_retry_timer = jenlib::time::kInvalidTimerId;
    }
    
    // Cancel connection timeout
    if (g_connection_timeout != jenlib::time::kInvalidTimerId) {
        jenlib::time::cancel_callback(g_connection_timeout);
        g_connection_timeout = jenlib::time::kInvalidTimerId;
    }
    
    // Subscribe to GATT service for our device (not implemented in facade yet)
    
    // Transition to passive waiting
    g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting));
}

void on_enter_passive_waiting() {
    Serial.println("State: PASSIVE_WAITING - Ready to receive start broadcast");
}

void on_enter_broadcasting() {
    Serial.println("State: BROADCASTING");
    
    // Schedule periodic broadcasts
    g_broadcast_timer = jenlib::time::schedule_callback(
        kBroadcastInterval,
        on_broadcast_timer,
        true // repeat
    );
    
    // Send initial reading immediately
    on_broadcast_timer();
}

// State exit callbacks
void on_exit_disconnected() {
    // Cleanup when leaving disconnected state
}

void on_exit_connecting() {
    // Cleanup when leaving connecting state
}

void on_exit_connected() {
    // Cleanup when leaving connected state
}

void on_exit_passive_waiting() {
    // Cleanup when leaving passive waiting state
}

void on_exit_broadcasting() {
    // Cancel broadcast timer
    if (g_broadcast_timer != jenlib::time::kInvalidTimerId) {
        jenlib::time::cancel_callback(g_broadcast_timer);
        g_broadcast_timer = jenlib::time::kInvalidTimerId;
    }
}

// Setup function
void setup() {
    Serial.begin(115200);
    Serial.println("BLE Sensor Starting...");
    
    // Initialize drivers
    jenlib::GPIO::setDriver(new gpio::ArduinoGpioDriver());
    jenlib::ble::BLE::set_driver(new jenlib::ble::ArduinoBleDriver("Sensor", jenlib::ble::DeviceId(static_cast<std::uint32_t>(kDeviceId))));
    
    // Initialize measurement system
    jenlib::measurement::initialize();
    
    // Set up BLE callbacks
    jenlib::ble::BLE::set_connection_callback(on_connection_changed);
    jenlib::ble::BLE::set_start_broadcast_callback(on_start_broadcast);
    jenlib::ble::BLE::set_receipt_callback(on_receipt);
    
    // Initialize BLE
    if (!jenlib::ble::BLE::begin()) {
        Serial.println("BLE initialization failed!");
        return;
    }
    
    Serial.println("BLE initialized successfully");
    
    // Set up state machine
    g_state_machine.add_state(static_cast<jenlib::state::StateId>(SensorState::kDisconnected), "Disconnected");
    g_state_machine.add_state(static_cast<jenlib::state::StateId>(SensorState::kConnecting), "Connecting");
    g_state_machine.add_state(static_cast<jenlib::state::StateId>(SensorState::kConnected), "Connected");
    g_state_machine.add_state(static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting), "PassiveWaiting");
    g_state_machine.add_state(static_cast<jenlib::state::StateId>(SensorState::kBroadcasting), "Broadcasting");
    
    // Set up state entry/exit callbacks
    g_state_machine.set_state_entry_callback(static_cast<jenlib::state::StateId>(SensorState::kDisconnected), on_enter_disconnected);
    g_state_machine.set_state_entry_callback(static_cast<jenlib::state::StateId>(SensorState::kConnecting), on_enter_connecting);
    g_state_machine.set_state_entry_callback(static_cast<jenlib::state::StateId>(SensorState::kConnected), on_enter_connected);
    g_state_machine.set_state_entry_callback(static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting), on_enter_passive_waiting);
    g_state_machine.set_state_entry_callback(static_cast<jenlib::state::StateId>(SensorState::kBroadcasting), on_enter_broadcasting);
    
    g_state_machine.set_state_exit_callback(static_cast<jenlib::state::StateId>(SensorState::kDisconnected), on_exit_disconnected);
    g_state_machine.set_state_exit_callback(static_cast<jenlib::state::StateId>(SensorState::kConnecting), on_exit_connecting);
    g_state_machine.set_state_exit_callback(static_cast<jenlib::state::StateId>(SensorState::kConnected), on_exit_connected);
    g_state_machine.set_state_exit_callback(static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting), on_exit_passive_waiting);
    g_state_machine.set_state_exit_callback(static_cast<jenlib::state::StateId>(SensorState::kBroadcasting), on_exit_broadcasting);
    
    // Set up event handlers for each state
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kDisconnected), jenlib::events::EventType::kConnectionStateChange, handle_connection_event);
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kConnecting), jenlib::events::EventType::kConnectionStateChange, handle_connection_event);
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kConnecting), jenlib::events::EventType::kTimeTick, handle_timer_event);
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kConnected), jenlib::events::EventType::kConnectionStateChange, handle_connection_event);
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting), jenlib::events::EventType::kConnectionStateChange, handle_connection_event);
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kPassiveWaiting), jenlib::events::EventType::kBleMessage, handle_ble_message_event);
    g_state_machine.set_event_handler(static_cast<jenlib::state::StateId>(SensorState::kBroadcasting), jenlib::events::EventType::kConnectionStateChange, handle_connection_event);
    
    // Start in connecting state
    g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kConnecting));
    
    // Connection management handled internally; check status
    if (jenlib::ble::BLE::is_connected()) {
        Serial.println("Connection attempt started");
    } else {
        Serial.println("Connection failed, will retry");
        g_state_machine.set_state(static_cast<jenlib::state::StateId>(SensorState::kDisconnected));
    }
}

// Main loop - now much simpler with event-driven architecture
void loop() {
    // Process all event systems
    jenlib::ble::BLE::process_events();
    jenlib::time::process_timers();
    jenlib::events::EventDispatcher::process_events();
    
    // Handle events in the state machine
    // Note: In a real implementation, you might want to process events
    // from a queue rather than handling them immediately
    
    // Minimal delay to prevent busy waiting
    jenlib::time::delay(10);
}
