# Arduino BLE Sensor Example - Callback-Based Implementation

## Overview
This example demonstrates how to use the jenlib callback-based event system for an Arduino BLE sensor. The implementation eliminates manual timing loops and state polling in favor of event-driven callbacks.

## Complete Arduino main.cpp Example

```cpp
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

// Device configuration
constexpr DeviceId kDeviceId = DeviceId(0x12345678);
constexpr std::uint32_t kBroadcastInterval = 5000; // 5 seconds
constexpr std::uint32_t kConnectionTimeout = 30000; // 30 seconds
constexpr std::uint32_t kRetryInterval = 2000; // 2 seconds

// Global state management
enum class ActivityState {
    kDisconnected,
    kConnecting,
    kConnected,
    kPassiveWaiting,
    kBroadcasting,
    kNumStates
};

ActivityState g_state = ActivityState::kDisconnected;
SessionId g_current_session_id = SessionId(0);
TimerId g_broadcast_timer = kInvalidTimerId;
TimerId g_connection_timeout = kInvalidTimerId;
TimerId g_retry_timer = kInvalidTimerId;
std::uint32_t g_session_start_time = 0;

// Forward declarations for callback functions
void on_connection_changed(bool connected);
void on_start_broadcast(DeviceId sender_id, const StartBroadcastMsg& msg);
void on_receipt(DeviceId sender_id, const ReceiptMsg& msg);
void on_connection_timeout();
void on_retry_connection();
void on_broadcast_timer();
void on_measurement_ready(const Measurement& measurement);

// Connection state callback
void on_connection_changed(bool connected) {
    Serial.print("Connection state changed: ");
    Serial.println(connected ? "CONNECTED" : "DISCONNECTED");
    
    if (connected) {
        g_state = ActivityState::kConnected;
        
        // Cancel any pending retry timers
        if (g_retry_timer != kInvalidTimerId) {
            jenlib::time::cancel_callback(g_retry_timer);
            g_retry_timer = kInvalidTimerId;
        }
        
        // Cancel connection timeout
        if (g_connection_timeout != kInvalidTimerId) {
            jenlib::time::cancel_callback(g_connection_timeout);
            g_connection_timeout = kInvalidTimerId;
        }
        
        // Subscribe to GATT service for our device
        jenlib::BLE::subscribe_to_service(kDeviceId);
        
        // Transition to passive waiting state
        g_state = ActivityState::kPassiveWaiting;
        Serial.println("State: PASSIVE_WAITING - Ready to receive start broadcast");
        
    } else {
        g_state = ActivityState::kDisconnected;
        
        // Cancel broadcast timer if active
        if (g_broadcast_timer != kInvalidTimerId) {
            jenlib::time::cancel_callback(g_broadcast_timer);
            g_broadcast_timer = kInvalidTimerId;
        }
        
        // Cancel connection timeout
        if (g_connection_timeout != kInvalidTimerId) {
            jenlib::time::cancel_callback(g_connection_timeout);
            g_connection_timeout = kInvalidTimerId;
        }
        
        // Schedule retry connection
        g_retry_timer = jenlib::time::schedule_callback(
            kRetryInterval,
            on_retry_connection,
            false // one-shot
        );
        
        Serial.println("State: DISCONNECTED - Will retry connection");
    }
}

// Start broadcast message callback
void on_start_broadcast(DeviceId sender_id, const StartBroadcastMsg& msg) {
    Serial.print("Received start broadcast from: 0x");
    Serial.println(static_cast<uint32_t>(sender_id), HEX);
    
    if (msg.device_id == kDeviceId) {
        g_state = ActivityState::kBroadcasting;
        g_current_session_id = msg.session_id;
        g_session_start_time = jenlib::time::now();
        
        Serial.print("State: BROADCASTING - Session ID: 0x");
        Serial.println(static_cast<uint32_t>(g_current_session_id), HEX);
        
        // Schedule periodic broadcasts
        g_broadcast_timer = jenlib::time::schedule_callback(
            kBroadcastInterval,
            on_broadcast_timer,
            true // repeat
        );
        
        // Send initial reading immediately
        on_broadcast_timer();
    }
}

// Receipt message callback
void on_receipt(DeviceId sender_id, const ReceiptMsg& msg) {
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

// Connection timeout callback
void on_connection_timeout() {
    Serial.println("Connection timeout - transitioning to disconnected");
    g_connection_timeout = kInvalidTimerId;
    g_state = ActivityState::kDisconnected;
    
    // The connection callback will handle retry logic
    jenlib::BLE::disconnect();
}

// Retry connection callback
void on_retry_connection() {
    Serial.println("Retrying connection...");
    g_retry_timer = kInvalidTimerId;
    g_state = ActivityState::kConnecting;
    
    // Attempt to reconnect
    if (jenlib::BLE::connect()) {
        // Set connection timeout
        g_connection_timeout = jenlib::time::schedule_callback(
            kConnectionTimeout,
            on_connection_timeout,
            false // one-shot
        );
    } else {
        // Schedule another retry
        g_retry_timer = jenlib::time::schedule_callback(
            kRetryInterval,
            on_retry_connection,
            false // one-shot
        );
    }
}

// Broadcast timer callback
void on_broadcast_timer() {
    if (g_state != ActivityState::kBroadcasting) {
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
    jenlib::BLE::broadcast_reading(kDeviceId, reading_msg);
    
    Serial.print("Broadcast reading - Temp: ");
    Serial.print(measurement.temperature_c_centi / 100.0f);
    Serial.print("°C, Humidity: ");
    Serial.print(measurement.humidity_bp / 100.0f);
    Serial.print("%, Offset: ");
    Serial.print(offset_ms);
    Serial.println("ms");
}

// Measurement ready callback (if using interrupt-driven sensor reading)
void on_measurement_ready(const Measurement& measurement) {
    // This could be called from a sensor interrupt or timer
    // For now, we'll use the broadcast timer for simplicity
    Serial.print("Measurement ready - Temp: ");
    Serial.print(measurement.temperature_c_centi / 100.0f);
    Serial.print("°C, Humidity: ");
    Serial.println(measurement.humidity_bp / 100.0f);
}

// Setup function
void setup() {
    Serial.begin(115200);
    Serial.println("BLE Sensor Starting...");
    
    // Initialize drivers
    jenlib::GPIO::setDriver(new gpio::ArduinoGpioDriver());
    jenlib::BLE::setDriver(new ble::ArduinoBleDriver());
    
    // Initialize measurement system
    jenlib::measurement::initialize();
    
    // Set up BLE callbacks
    jenlib::BLE::set_connection_callback(on_connection_changed);
    jenlib::BLE::set_start_broadcast_callback(on_start_broadcast);
    jenlib::BLE::set_receipt_callback(on_receipt);
    
    // Initialize BLE
    if (!jenlib::BLE::initialize(kDeviceId)) {
        Serial.println("BLE initialization failed!");
        return;
    }
    
    Serial.println("BLE initialized successfully");
    
    // Start connection process
    g_state = ActivityState::kConnecting;
    if (jenlib::BLE::connect()) {
        // Set connection timeout
        g_connection_timeout = jenlib::time::schedule_callback(
            kConnectionTimeout,
            on_connection_timeout,
            false // one-shot
        );
        Serial.println("Connection attempt started");
    } else {
        // Schedule retry
        g_retry_timer = jenlib::time::schedule_callback(
            kRetryInterval,
            on_retry_connection,
            false // one-shot
        );
        Serial.println("Connection failed, will retry");
    }
}

// Main loop - now much simpler with event-driven architecture
void loop() {
    // Process all event systems
    jenlib::BLE::process_events();
    jenlib::time::process_timers();
    jenlib::events::EventDispatcher::process_events();
    
    // Minimal delay to prevent busy waiting
    jenlib::time::delay(10);
}

// Optional: Add interrupt handlers for sensor readings
void on_sensor_interrupt() {
    // This could trigger immediate measurement reading
    // and dispatch an event to the measurement system
    jenlib::events::EventDispatcher::dispatch_event({
        .type = jenlib::events::EventType::kMeasurementReady,
        .timestamp = jenlib::time::now(),
        .data = 0
    });
}

// Optional: Add watchdog timer for system health
void on_watchdog_timeout() {
    Serial.println("Watchdog timeout - system may be unresponsive");
    
    // Reset system or take corrective action
    // This could be implemented as a periodic timer
    // that checks system health
}
```

## Key Features Demonstrated

### 1. **Event-Driven Architecture**
- No manual state polling in the main loop
- All state transitions triggered by callbacks
- Clean separation of concerns

### 2. **Timer-Based Operations**
- Connection timeout handling
- Retry logic with exponential backoff
- Periodic broadcast scheduling
- No manual timing loops

### 3. **Callback Registration**
- Connection state monitoring
- Message type-specific callbacks
- Error handling and recovery

### 4. **State Management**
- Clear state transitions
- State validation
- Error recovery mechanisms

### 5. **Resource Management**
- Timer cleanup on state changes
- Memory-efficient callback storage
- Proper resource deallocation

## Benefits Over Polling Approach

1. **Responsiveness**: Immediate response to events
2. **Efficiency**: No busy waiting or unnecessary polling
3. **Maintainability**: Clear event flow and state transitions
4. **Testability**: Easy to mock callbacks and events
5. **Scalability**: Can handle multiple concurrent operations
6. **Power Efficiency**: Reduced CPU usage during idle periods

## Usage Notes

- The main loop is now minimal and non-blocking
- All timing is handled by the timer service
- State transitions are explicit and traceable
- Error handling is built into the callback system
- The system can easily be extended with additional event types
