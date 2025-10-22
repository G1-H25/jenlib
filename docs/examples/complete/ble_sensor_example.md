# JenLib Example Usage Guide

This guide demonstrates how to use JenLib's pin, event, and callback systems to read sensor values and transmit them over BLE, based on the Arduino BLE sensor example.

## Overview

The example shows a complete sensor node that:
1. Reads temperature and humidity values from sensors
2. Uses a state machine to manage measurement sessions
3. Transmits readings over BLE when requested by a broker
4. Handles connection state changes and message acknowledgments

## Hardware

The example uses a TMP36 sensor. Readings are calculated from an input analog voltage.
The calculation result will depend on the reference voltage.

## Key Components

### 1. Pin System (GPIO)

The GPIO system provides hardware abstraction for reading sensor values:

```cpp
#include <jenlib/gpio/GPIO.h>
#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>

// Initialize GPIO driver
static jenlib::gpio::ArduinoGpioDriver gpio_driver;

// Configure GPIO system
void setup_gpio() {
    GPIO::setDriver(&gpio_driver);
    GPIO::setAnalogReadResolution(10);  // 10-bit ADC resolution
}

// Read TMP36 temperature sensor on pin A0
float read_temperature_sensor() {
    // Create pin handle for analog pin A0
    GPIO::Pin temp_pin(A0);
    temp_pin.pinMode(GPIO::PinMode::INPUT);

    // Read analog value (0-1023 for 10-bit ADC)
    std::uint16_t analog_value = temp_pin.analogRead();

    // Convert to voltage (assuming 5V reference)
    float voltage = (analog_value * 5.0f) / 1023.0f;

    // TMP36 conversion: (voltage - 0.5) * 100 = temperature in Celsius
    // TMP36 outputs 10mV per degree C with 0.5V offset at 0°C
    float temperature_c = (voltage - 0.5f) * 100.0f;

    return temperature_c;
}
```

**Key Points:**
- Use platform-specific drivers (ArduinoGpioDriver for Arduino)
- Configure ADC resolution for accurate analog readings
- TMP36 sensor provides linear voltage output (10mV/°C with 0.5V offset)
- The library provides GPIO abstraction, sensor-specific conversion is in application code
- Pin A0 is used for analog input (ADC) on most Arduino boards

### 2. Event System

The event system enables decoupled communication between components:

```cpp
#include <jenlib/events/EventDispatcher.h>

// Global event dispatcher
jenlib::events::EventDispatcher event_dispatcher;

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

// Process events in main loop
void loop() {
    event_dispatcher.process_events();
    // ... other processing
}
```

**Event Types:**
- `kBleMessage`: BLE communication events
- `kConnectionStateChange`: Connection status changes
- `kTimeTick`: Timer-based events

### 3. Callback System (BLE)

The BLE callback system handles incoming messages and connection changes:

```cpp
#include <jenlib/ble/drivers/ArduinoBleDriver.h>
#include <jenlib/ble/Roles.h>

// Define device identity
constexpr jenlib::ble::DeviceId kDeviceId = jenlib::ble::DeviceId(0x12345678);
static jenlib::ble::Sensor sensor(kDeviceId);

// Configure BLE callbacks
sensor.configure_callbacks(jenlib::ble::BleCallbacks{
    .on_connection = callback_connection,
    .on_start = callback_start,
    .on_receipt = callback_receipt,
    .on_generic = callback_generic,
});
```

**Callback Functions:**

#### Connection Callback
```cpp
void callback_connection(bool connected) {
    // Update state machine first
    sensor_state_machine.handle_connection_change(connected);

    // Dispatch event for other systems
    jenlib::events::Event event(
        jenlib::events::EventType::kConnectionStateChange,
        jenlib::time::Time::now(),
        connected ? 1 : 0);
    event_dispatcher.dispatch_event(event);
}
```

#### Start Measurement Callback
```cpp
void callback_start(jenlib::ble::DeviceId sender_id,
                   const jenlib::ble::StartBroadcastMsg &msg) {
    // Check if message is for this device
    if (msg.device_id != kDeviceId) {
        return; // Not for us
    }

    // Validate state transition
    bool success = sensor_state_machine.handle_start_broadcast(sender_id, msg);
    if (success) {
        start_measurement_session(msg);
    }
}
```

## Complete Workflow

### 1. Initialization (setup())

```cpp
void setup() {
    // Initialize GPIO system
    setup_gpio();

    // Initialize time service
    jenlib::time::Time::initialize();

    // Initialize BLE
    sensor.begin();
    sensor.configure_callbacks(/* callbacks */);

    // Register event handlers
    event_dispatcher.register_callback(/* handlers */);
}
```

### 2. Main Loop (loop())

```cpp
void loop() {
    // Process all event systems
    event_dispatcher.process_events();
    sensor.process_events();
    jenlib::time::Time::process_timers();
}
```

### 3. Measurement Session

When a broker requests measurements:

```cpp
void start_measurement_session(const jenlib::ble::StartBroadcastMsg& msg) {
    // Configure measurement interval
    sensor_state_machine.set_measurement_interval_ms(1000);

    // Take immediate reading
    take_and_broadcast_reading();

    // Schedule recurring measurements
    jenlib::time::schedule_repeating_timer(
        sensor_state_machine.get_measurement_interval_ms(),
        handle_measurement_timer);
}
```

### 4. Reading and Broadcasting

```cpp
void take_and_broadcast_reading() {
    if (!sensor_state_machine.is_session_active()) {
        return;
    }

    // Read TMP36 temperature sensor
    float temperature_c = read_temperature_sensor();

    // Create reading message (humidity set to 0 for TMP36-only setup)
    jenlib::ble::ReadingMsg reading_msg{
        .sender_id = kDeviceId,
        .session_id = sensor_state_machine.get_current_session_id(),
        .offset_ms = jenlib::time::Time::now(),
        .temperature_c_centi = jenlib::measurement::temperature_to_centi(temperature_c),
        .humidity_bp = 0  // No humidity sensor in this example
    };

    // Broadcast the reading
    sensor.broadcast_reading(reading_msg);
}
```

## State Machine Integration

The state machine ensures proper operation flow:

```cpp
#include <jenlib/state/SensorStateMachine.h>

jenlib::state::SensorStateMachine sensor_state_machine;

// State machine validates all operations
sensor_state_machine.handle_connection_change(connected);
sensor_state_machine.handle_start_broadcast(sender_id, msg);
sensor_state_machine.handle_measurement_timer();
```

**States:**
- `Disconnected`: No BLE connection
- `Waiting`: Connected, waiting for measurement request
- `Running`: Actively taking and sending measurements

## Key Design Principles

1. **Separation of Concerns**: Pin reading, event handling, and BLE communication are separate systems
2. **State Validation**: The state machine ensures operations only occur in valid states
3. **Event-Driven**: Components communicate through events rather than direct calls
4. **Platform Abstraction**: Use appropriate drivers for your platform (Arduino, Native, etc.)

## Customization Points

- **Sensor Reading**: Implement your own `read_temperature_sensor()` function for different sensors (TMP36, DS18B20, etc.)
- **Pin Configuration**: Change pin assignments by modifying the pin number in `GPIO::Pin temp_pin(A0)`
- **ADC Resolution**: Adjust `GPIO::setAnalogReadResolution()` for different precision needs
- **Voltage Reference**: Modify voltage calculation if using different reference voltages
- **Measurement Interval**: Adjust `set_measurement_interval_ms()` for your needs
- **Event Handlers**: Add custom logic in event handler functions
- **State Actions**: Configure state machine callbacks for debugging/monitoring