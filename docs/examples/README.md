# JenLib Examples

This directory contains comprehensive examples demonstrating how to use JenLib's various components and systems.

## Directory Structure

### Complete Examples (`complete/`)

Complete, end-to-end examples that demonstrate full workflows:

- **[BLE Sensor Example](complete/ble_sensor_example.md)** - Complete sensor node implementation showing how to read sensor values and transmit them over BLE using all JenLib components

### Component Examples (`components/`)

Focused examples for individual JenLib components:

- **[BLE Examples](components/ble_example.md)** - BLE communication, sensor/broker setup, and message broadcasting
- **[GPIO Examples](components/gpio_example.md)** - Pin configuration, digital/analog I/O, and sensor reading
- **[Event System Examples](components/event_example.md)** - Event handling, dispatching, and main loop integration
- **[State Machine Examples](components/state_example.md)** - State management and validation
- **[Measurement Examples](components/measurement_example.md)** - Unit conversions and message creation
- **[Time Service Examples](components/time_example.md)** - Timer scheduling and management

## Getting Started

### For New Users
Start with the **[BLE Sensor Example](complete/ble_sensor_example.md)** to understand the complete workflow and how all components work together.

### For Component-Specific Learning
Browse the **[Component Examples](components/)** to learn about specific JenLib features in isolation.

## Key Design Principles

All examples demonstrate these core JenLib principles:

1. **Separation of Concerns** - Each component has a specific responsibility
2. **Platform Abstraction** - Use appropriate drivers for your platform (Arduino, Native, etc.)
3. **Event-Driven Architecture** - Components communicate through events
4. **State Validation** - State machines ensure proper operation flow
5. **Hardware Abstraction** - GPIO system provides platform-independent pin access

## Customization

Examples are designed to be starting points. Key customization areas:

- **Sensor Types** - Adapt sensor reading code for different sensors (TMP36, DS18B20, etc.)
- **Pin Assignments** - Modify pin numbers for your hardware setup
- **Measurement Intervals** - Adjust timing for your application needs
- **Event Handlers** - Add custom logic for your specific requirements
- **Platform Drivers** - Choose appropriate drivers for your target platform

## Related Documentation

- [Main Documentation](../html/index.html) - Complete API reference
- [BLE Protocol](../ble_protocol.md) - Detailed BLE communication protocol
- [Project README](../../README.md) - Project overview and setup
