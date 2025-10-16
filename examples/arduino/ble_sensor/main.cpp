//! @file examples/arduino/ble_sensor/main.cpp
//! @brief BLE sensor example for Arduino, using state machine and event driven architecture.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)


#include <jenlib/ble/Roles.h>
#include <jenlib/ble/Ids.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/time/Time.h>
#include <jenlib/state/SensorStateMachine.h>
#include <jenlib/measurement/Measurement.h>
#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>
#include <jenlib/ble/drivers/ArduinoBleDriver.h>
#include <jenlib/time/drivers/ArduinoTimeDriver.h>

//! @section Global state
jenlib::events::EventDispatcher event_dispatcher;  //!< Event dispatcher
constexpr jenlib::ble::DeviceId kDeviceId = jenlib::ble::DeviceId(0x12345678);  //!< Some DeviceID
static jenlib::ble::Sensor sensor(kDeviceId);  //!< We are a Sensor

//! @section Arduino driver construction
static jenlib::gpio::ArduinoGpioDriver gpio_driver;
static jenlib::ble::ArduinoBleDriver ble_driver;
static jenlib::time::ArduinoTimeDriver time_driver;

//! @section State machine
//! @brief Sensor state machine manages the lifecycle from disconnected -> waiting -> running
//! @details The state machine ensures proper state transitions and validates operations
//! based on the current state (e.g., can only start measurements when in 'waiting' state)
jenlib::state::SensorStateMachine sensor_state_machine;

//! @section Forward declaration of functions
//! BLE event callback functions
void callback_connection(bool connected);
void callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg);
void callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg);
void callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload);

//! Event handlers
void handle_measurement_timer();
void handle_ble_message_event(const jenlib::events::Event& event);
void handle_connection_state_event(const jenlib::events::Event& event);
void handle_time_tick_event(const jenlib::events::Event& event);

//! Helper functions
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg);
void stop_measurement_session();
void take_and_broadcast_reading();
float read_temperature_sensor();  // Mock sensor reading
float read_humidity_sensor();     // Mock sensor reading

//! @section Arduino functions
void setup() {
    // Initialize time service first
    jenlib::time::Time::initialize();

    // Initialize BLE communication
    sensor.begin();
    sensor.configure_callbacks(jenlib::ble::BleCallbacks{
        .on_connection = callback_connection,
        .on_start = callback_start,
        .on_receipt = callback_receipt,
        .on_generic = callback_generic,
    });

    // Configure state machine callbacks for debugging/monitoring
    sensor_state_machine.set_state_action_callback(
        [](jenlib::state::StateAction action,
           jenlib::state::SensorState state) {
            // Optional: Add logging or debugging here
            // For example, you could print state transitions to Serial
        });

    // Register event handlers
    event_dispatcher.register_callback(
        jenlib::events::EventType::kBleMessage,
        handle_ble_message_event);
    event_dispatcher.register_callback(
        jenlib::events::EventType::kConnectionStateChange,
        handle_connection_state_event);
    event_dispatcher.register_callback(
        jenlib::events::EventType::kTimeTick,
        handle_time_tick_event);
}

void loop() {
    // Process all event systems
    event_dispatcher.process_events();
    sensor.process_events();
    jenlib::time::Time::process_timers();

    // Process state machine events
    // The state machine handles its own event processing internally
}

//! @section Implementations of forward declared functions
void callback_connection(bool connected) {
    // Update state machine first - this validates the transition
    sensor_state_machine.handle_connection_change(connected);

    // Then dispatch event for other systems that might need to know
    jenlib::events::Event event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        connected ? 1 : 0);
    event_dispatcher.dispatch_event(event);
}

//! @brief What happens when a sensor recieves a message from a broker to start a measurement session
//! @param sender_id The ID of the broker that sent the message
//! @param msg The message from the broker
void callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg) {
    // First check if this message is intended for this sensor
    if (msg.device_id != kDeviceId) {
        // This message is not for us - ignore it
        return;
    }

    // Update state machine - this validates we're in the right state (waiting)
    bool success = sensor_state_machine.handle_start_broadcast(sender_id, msg);
    if (success) {
        // Only start session if state machine allows it
        start_measurement_session(msg);
    }

    // Dispatch BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::StartBroadcast));
    event_dispatcher.dispatch_event(event);
}

void callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg) {
    // Update state machine
    sensor_state_machine.handle_receipt(sender_id, msg);

    // Dispatch BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::Receipt));
    event_dispatcher.dispatch_event(event);

    // Handle receipt acknowledgment (could purge buffered readings here)
    // The state machine ensures we're in the right state and session
}

void callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload) {
    // Dispatch generic BLE message event
    jenlib::events::Event event(
        jenlib::events::EventType::kBleMessage,
        jenlib::time::Time::now(),
        static_cast<std::uint32_t>(jenlib::ble::MessageType::Reading));
    event_dispatcher.dispatch_event(event, nullptr);
}

//! @section Implementations of event handler functions
void handle_measurement_timer() {
    // Let the state machine handle the measurement logic
    sensor_state_machine.handle_measurement_timer();
    take_and_broadcast_reading();
}

void handle_time_tick_event(const jenlib::events::Event& event) {
    // Forward time tick events to the state machine
    sensor_state_machine.handle_event(event);
}

void handle_ble_message_event(const jenlib::events::Event& event) {
    // This could be used for logging or additional processing
    // The actual message handling is done in the BLE callbacks
}

void handle_connection_state_event(const jenlib::events::Event& event) {
    bool connected = (event.data != 0);
    if (!connected && sensor_state_machine.is_session_active()) {
        // Connection lost, stop measurement session
        sensor_state_machine.handle_session_end();
        stop_measurement_session();
    }
}

//! @section Implementations of helper functions
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg) {
    // Stop any existing session
    stop_measurement_session();

    // Configure state machine with session parameters
    sensor_state_machine.set_measurement_interval_ms(1000);  // 1 second interval

    // Schedule first measurement immediately
    take_and_broadcast_reading();

    // Schedule recurring measurements using state machine's timer
    jenlib::time::schedule_repeating_timer(
        sensor_state_machine.get_measurement_interval_ms(),
        handle_measurement_timer);
}

void stop_measurement_session() {
    // The state machine handles session state management
    // This function can be used for cleanup if needed
}

void take_and_broadcast_reading() {
    if (!sensor_state_machine.is_session_active()) {
        return;
    }

    // Read sensors
    float temperature_c = read_temperature_sensor();
    float humidity_pct = read_humidity_sensor();

    // Create reading message using state machine's session info
    jenlib::ble::ReadingMsg reading_msg{
        .sender_id = kDeviceId,
        .session_id = sensor_state_machine.get_current_session_id(),
        .offset_ms = jenlib::time::Time::now(),  // Simplified for this example
        .temperature_c_centi = measurement::temperature_to_centi(temperature_c),
        .humidity_bp = measurement::humidity_to_basis_points(humidity_pct)
    };

    // Broadcast the reading
    sensor.broadcast_reading(reading_msg);
}

//! @section Implementations of mock sensor reading functions
float read_temperature_sensor() {
    // Mock temperature reading - replace with actual sensor code
    // For demo purposes, return a simulated temperature
    static float base_temp = 22.5f;
    static float variation = 0.0f;
    variation += 0.1f;
    if (variation > 2.0f) variation = -2.0f;
    return base_temp + variation;
}

float read_humidity_sensor() {
    // Mock humidity reading - replace with actual sensor code
    // For demo purposes, return a simulated humidity
    static float base_humidity = 45.0f;
    static float variation = 0.0f;
    variation += 0.2f;
    if (variation > 5.0f) variation = -5.0f;
    return base_humidity + variation;
}
