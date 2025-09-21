#include <jenlib/ble/Roles.h>
#include <jenlib/ble/Ids.h>
#include <jenlib/ble/Messages.h>
#include <jenlib/events/EventDispatcher.h>
#include <jenlib/time/Time.h>
#include <jenlib/measurement/Measurement.h>
#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>
#include <jenlib/ble/drivers/ArduinoBleDriver.h>
#include <jenlib/time/drivers/ArduinoTimeDriver.h>

//! @section Global state
jenlib::events::EventDispatcher event_dispatcher; //!< Event dispatcher
constexpr jenlib::ble::DeviceId kDeviceId = jenlib::ble::DeviceId(0x12345678); //!< Some DeviceID
static jenlib::ble::Sensor sensor(kDeviceId); //!< We are a Sensor

//! @section Arduino driver construction
static jenlib::gpio::ArduinoGpioDriver gpio_driver;
static jenlib::ble::ArduinoBleDriver ble_driver; 
static jenlib::time::ArduinoTimeDriver time_driver;

//! @section Session state
struct SessionState {
    bool active = false;
    jenlib::ble::SessionId session_id{0};
    std::uint32_t start_time_ms = 0;
    std::uint32_t measurement_interval_ms = 1000; // Default 1 second
    jenlib::time::TimerId measurement_timer_id = jenlib::time::kInvalidTimerId;
} session_state;

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

//! Helper functions
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg);
void stop_measurement_session();
void take_and_broadcast_reading();
float read_temperature_sensor(); // Mock sensor reading
float read_humidity_sensor();    // Mock sensor reading

void setup() {
    // Initialize BLE communication
    sensor.begin();
    sensor.configure_callbacks(jenlib::ble::BleCallbacks{
        .on_connection = callback_connection,
        .on_start = callback_start,
        .on_receipt = callback_receipt,
        .on_generic = callback_generic,
    });

    // Register event handlers
    event_dispatcher.register_callback(jenlib::events::EventType::kBleMessage, handle_ble_message_event);
    event_dispatcher.register_callback(jenlib::events::EventType::kConnectionStateChange, handle_connection_state_event);
    
    // Initialize time service
    jenlib::time::Time::initialize();
}

void loop() {
    // Process all event systems
    event_dispatcher.process_events();
    sensor.process_events();
    jenlib::time::Time::process_timers();
}

// BLE Callback implementations
void callback_connection(bool connected) {
    // Dispatch connection state change event
    jenlib::events::Event event(jenlib::events::EventType::kConnectionStateChange, 
                               jenlib::time::Time::now(), 
                               connected ? 1 : 0);
    event_dispatcher.dispatch_event(event);
}

void callback_start(jenlib::ble::DeviceId sender_id, const jenlib::ble::StartBroadcastMsg &msg) {
    // Dispatch BLE message event
    jenlib::events::Event event(jenlib::events::EventType::kBleMessage, 
                               jenlib::time::Time::now(), 
                               static_cast<std::uint32_t>(jenlib::ble::MessageType::StartBroadcast));
    event_dispatcher.dispatch_event(event);
    
    // Start the measurement session
    start_measurement_session(msg);
}

void callback_receipt(jenlib::ble::DeviceId sender_id, const jenlib::ble::ReceiptMsg &msg) {
    // Dispatch BLE message event
    jenlib::events::Event event(jenlib::events::EventType::kBleMessage, 
                               jenlib::time::Time::now(), 
                               static_cast<std::uint32_t>(jenlib::ble::MessageType::Receipt));
    event_dispatcher.dispatch_event(event);
    
    // Handle receipt acknowledgment (could purge buffered readings here)
    if (session_state.active && session_state.session_id == msg.session_id) {
        // In a real implementation, you might purge readings up to msg.up_to_offset_ms
        // For now, just acknowledge receipt
    }
}

void callback_generic(jenlib::ble::DeviceId sender_id, const jenlib::ble::BlePayload &payload) {
    // Dispatch generic BLE message event
    jenlib::events::Event event(jenlib::events::EventType::kBleMessage, 
                               jenlib::time::Time::now(), 
                               static_cast<std::uint32_t>(jenlib::ble::MessageType::Reading));
    event_dispatcher.dispatch_event(event);
}

// Event handler implementations
void handle_measurement_timer() {
    if (session_state.active) {
        take_and_broadcast_reading();
    }
}

void handle_ble_message_event(const jenlib::events::Event& event) {
    // This could be used for logging or additional processing
    // The actual message handling is done in the BLE callbacks
}

void handle_connection_state_event(const jenlib::events::Event& event) {
    bool connected = (event.data != 0);
    if (!connected && session_state.active) {
        // Connection lost, stop measurement session
        stop_measurement_session();
    }
}

// Helper function implementations
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg) {
    // Stop any existing session
    stop_measurement_session();
    
    // Start new session
    session_state.active = true;
    session_state.session_id = msg.session_id;
    session_state.start_time_ms = jenlib::time::Time::now();
    
    // Schedule first measurement immediately
    take_and_broadcast_reading();
    
    // Schedule recurring measurements
    session_state.measurement_timer_id = jenlib::time::schedule_repeating_timer(
        session_state.measurement_interval_ms, 
        handle_measurement_timer
    );
}

void stop_measurement_session() {
    if (session_state.active) {
        session_state.active = false;
        
        // Cancel measurement timer
        if (session_state.measurement_timer_id != jenlib::time::kInvalidTimerId) {
            jenlib::time::Time::cancel_callback(session_state.measurement_timer_id);
            session_state.measurement_timer_id = jenlib::time::kInvalidTimerId;
        }
    }
}

void take_and_broadcast_reading() {
    if (!session_state.active) {
        return;
    }
    
    // Read sensors
    float temperature_c = read_temperature_sensor();
    float humidity_pct = read_humidity_sensor();
    
    // Calculate time offset since session start
    std::uint32_t current_time = jenlib::time::Time::now();
    std::uint32_t offset_ms = current_time - session_state.start_time_ms;
    
    // Create reading message
    jenlib::ble::ReadingMsg reading_msg{
        .sender_id = kDeviceId,
        .session_id = session_state.session_id,
        .offset_ms = offset_ms,
        .temperature_c_centi = measurement::temperature_to_centi(temperature_c),
        .humidity_bp = measurement::humidity_to_basis_points(humidity_pct)
    };
    
    // Broadcast the reading
    sensor.broadcast_reading(reading_msg);
}

// Mock sensor reading functions (replace with actual sensor code)
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