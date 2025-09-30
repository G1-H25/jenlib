# Event System Implementation Guide

## Overview
This document outlines the implementation strategy for a callback-based event system to replace polling loops in the jenlib BLE sensor example. The system will provide unified event handling across Arduino, ESP-IDF, and native platforms.

## Core Architecture Components

### 1. Event System Foundation

#### Event Types and Structure
- [ ] Define `jenlib::events::EventType` enum with values:
  - `kTimeTick` - Periodic timer events
  - `kBleMessage` - BLE message received
  - `kGpioChange` - GPIO state change
  - `kMeasurementReady` - Sensor reading available
  - `kConnectionStateChange` - BLE connection state change
- [ ] Create `jenlib::events::Event` struct with:
  - `EventType type`
  - `std::uint32_t timestamp`
  - `std::uint32_t data` (or `void*` for complex data)
  - Platform-specific data union for efficiency
- [ ] Implement `jenlib::events::EventCallback` as `std::function<void(const Event&)>`

#### Event Dispatcher
- [ ] Create `jenlib::events::EventDispatcher` class with static methods:
  - `register_callback(EventType type, EventCallback callback)`
  - `unregister_callback(EventType type)`
  - `dispatch_event(const Event& event)`
  - `process_events()` - Main processing loop
- [ ] Implement callback registry using `std::unordered_map<EventType, EventCallback>`
- [ ] Add thread safety for multi-threaded platforms (ESP-IDF, native)
- [ ] Implement event queue with configurable size (default 32 events)

### 2. Time Service Implementation

#### Timer Management
- [ ] Create `jenlib::time::Timer` class with static methods:
  - `schedule_callback(interval_ms, callback, repeat)` → `TimerId`
  - `cancel_callback(TimerId timer_id)`
  - `process_timers()` - Called in main loop
- [ ] Define `TimerId` as `std::uint32_t` with `kInvalidTimerId = 0`
- [ ] Implement `TimerCallback` as `std::function<void()>`
- [ ] Create timer storage structure:
  ```cpp
  struct TimerEntry {
      TimerId id;
      std::uint32_t interval_ms;
      std::uint32_t next_fire_time;
      TimerCallback callback;
      bool repeat;
      bool active;
  };
  ```

#### Platform-Specific Time Abstraction
- [ ] **Arduino Implementation**:
  - Use `millis()` for timing
  - Implement non-blocking timer processing
  - Handle timer overflow (49.7 days)
- [ ] **ESP-IDF Implementation**:
  - Use `esp_timer` API for high-precision timers
  - Implement FreeRTOS task-safe timer management
  - Use `xTaskGetTickCount()` for timing
- [ ] **Native Implementation**:
  - Use `std::chrono::steady_clock` for timing
  - Implement thread-safe timer management
  - Use `std::thread` for background timer processing

#### Convenience Functions
- [ ] Implement `jenlib::time::schedule_broadcast_interval(interval_ms, callback)`
- [ ] Implement `jenlib::time::schedule_one_shot(delay_ms, callback)`
- [ ] Add `jenlib::time::elapsed_since_session_start()` for session timing
- [ ] Create `jenlib::time::now()` wrapper for platform-specific time functions

### 3. Enhanced BLE Callback System

#### Connection State Management
- [ ] Add `jenlib::ble::ConnectionCallback` type: `std::function<void(bool connected)>`
- [ ] Extend `BleDriver` interface with:
  - `set_connection_callback(ConnectionCallback callback)`
  - `clear_connection_callback()`
- [ ] Implement connection state tracking in drivers
- [ ] Add automatic callback invocation on connection state changes

#### Message Processing Enhancement
- [ ] Extend existing callback system in `BleDriver`:
  - Ensure `set_start_broadcast_callback()` is implemented
  - Ensure `set_reading_callback()` is implemented  
  - Ensure `set_receipt_callback()` is implemented
- [ ] Add `process_events()` method to `BleDriver` interface
- [ ] Implement automatic message deserialization and callback invocation
- [ ] Add message type validation before callback invocation

#### BLE Facade Updates
- [ ] Extend `jenlib::ble::BLE` class with:
  - `set_connection_callback(ConnectionCallback callback)`
  - `process_events()` - Main BLE event processing
- [ ] Implement automatic callback registration with driver
- [ ] Add error handling for callback registration failures

### 4. State Machine Refactoring

#### Event-Driven State Machine
- [ ] Create `SensorStateMachine` class to replace manual state handling
- [ ] Implement state transition logic based on event types
- [ ] Add state validation and error handling
- [ ] Create state-specific callback registration

#### State-Specific Callbacks
- [ ] **kDisconnected State**:
  - Register for connection state change events
  - Implement connection retry logic
- [ ] **kConnected State**:
  - Register for BLE message events
  - Handle service subscription
- [ ] **kPassiveWaiting State**:
  - Register for start broadcast messages
  - Implement timeout handling
- [ ] **kBroadcasting State**:
  - Register for receipt messages
  - Handle broadcast timer events
  - Implement session management

### 5. Main Loop Refactoring

#### Event Processing Loop
- [ ] Replace manual state checking with event processing:
  ```cpp
  void loop() {
      jenlib::BLE::process_events();
      jenlib::time::process_timers();
      jenlib::events::EventDispatcher::process_events();
      jenlib::time::delay(10); // Minimal delay
  }
  ```
- [ ] Remove manual timing loops from state cases
- [ ] Implement non-blocking event processing
- [ ] Add event processing performance monitoring

#### Callback Registration in Setup
- [ ] Move all callback registration to `setup()` function
- [ ] Implement callback validation and error handling
- [ ] Add callback registration logging for debugging

### 6. Platform-Specific Implementations

#### Arduino Platform
- [ ] **Time Service**:
  - Implement `ArduinoTimeService` using `millis()`
  - Handle timer overflow with 64-bit arithmetic
  - Use non-blocking timer processing
- [ ] **Event System**:
  - Implement simple event queue with static allocation
  - Use interrupt-safe event processing
  - Minimize memory usage for constrained environments
- [ ] **BLE Integration**:
  - Implement `ArduinoBleDriver::process_events()`
  - Add connection state monitoring
  - Handle BLE stack events in callback system

#### ESP-IDF Platform
- [ ] **Time Service**:
  - Implement `EspIdfTimeService` using `esp_timer`
  - Use FreeRTOS timers for high-precision timing
  - Implement task-safe timer management
- [ ] **Event System**:
  - Use FreeRTOS queues for event handling
  - Implement task-safe event processing
  - Add event priority support
- [ ] **BLE Integration**:
  - Implement `EspIdfBleDriver::process_events()`
  - Use ESP-IDF BLE stack event handling
  - Implement GATT server/client event processing

#### Native Platform
- [ ] **Time Service**:
  - Implement `NativeTimeService` using `std::chrono`
  - Use `std::thread` for background timer processing
  - Implement thread-safe timer management
- [ ] **Event System**:
  - Use `std::queue` with mutex protection
  - Implement thread-safe event processing
  - Add event batching for performance
- [ ] **BLE Integration**:
  - Implement `NativeBleDriver::process_events()`
  - Use mock BLE implementation for testing
  - Add event injection for testing

### 7. Testing Strategy

#### Unit Testing
- [ ] **Time Service Tests**:
  - Test timer scheduling and cancellation
  - Test timer overflow handling
  - Test callback invocation timing
  - Test platform-specific time implementations
- [ ] **Event System Tests**:
  - Test event registration and unregistration
  - Test event dispatch and processing
  - Test event queue overflow handling
  - Test callback error handling
- [ ] **BLE Callback Tests**:
  - Test message callback registration
  - Test connection state callback handling
  - Test message deserialization and callback invocation
  - Test error handling for malformed messages

#### Integration Testing
- [ ] **State Machine Tests**:
  - Test state transitions based on events
  - Test state-specific callback registration
  - Test error handling in state transitions
- [ ] **End-to-End Tests**:
  - Test complete sensor-to-broker communication
  - Test timer-based broadcasting
  - Test connection state handling
  - Test session management

#### Mock Implementations
- [ ] **Mock Time Service**:
  - Implement controllable time advancement
  - Add timer event injection
  - Support deterministic testing
- [ ] **Mock BLE Driver**:
  - Implement message injection
  - Add connection state simulation
  - Support callback verification
- [ ] **Mock Event System**:
  - Implement event injection
  - Add callback verification
  - Support performance testing

### 8. Performance Considerations

#### Memory Management
- [ ] **Static Allocation**:
  - Use fixed-size arrays for event queues
  - Pre-allocate timer structures
  - Minimize dynamic allocation in embedded environments
- [ ] **Callback Storage**:
  - Use `std::function` with small buffer optimization
  - Implement callback pooling for high-frequency events
  - Add callback memory usage monitoring

#### Processing Efficiency
- [ ] **Event Batching**:
  - Batch similar events to reduce processing overhead
  - Implement event priority queues
  - Add event processing time monitoring
- [ ] **Lazy Evaluation**:
  - Only process events when needed
  - Implement event filtering
  - Add event processing optimization

#### Platform Optimization
- [ ] **Arduino Optimization**:
  - Minimize interrupt latency
  - Use efficient data structures
  - Optimize for memory constraints
- [ ] **ESP-IDF Optimization**:
  - Use FreeRTOS features efficiently
  - Implement task priority management
  - Optimize for real-time performance
- [ ] **Native Optimization**:
  - Use modern C++ features
  - Implement efficient threading
  - Optimize for development and testing

### 9. Error Handling and Debugging

#### Error Handling
- [ ] **Callback Errors**:
  - Implement callback exception handling
  - Add callback error logging
  - Implement callback recovery mechanisms
- [ ] **Timer Errors**:
  - Handle timer overflow gracefully
  - Implement timer error recovery
  - Add timer performance monitoring
- [ ] **Event System Errors**:
  - Handle event queue overflow
  - Implement event processing error recovery
  - Add event system health monitoring

#### Debugging Support
- [ ] **Logging System**:
  - Implement platform-specific logging
  - Add event processing logging
  - Implement callback invocation logging
- [ ] **Performance Monitoring**:
  - Add event processing time measurement
  - Implement callback execution time monitoring
  - Add memory usage tracking
- [ ] **Debug Tools**:
  - Implement event injection tools
  - Add callback verification tools
  - Create performance profiling tools

### 10. Documentation and Examples

#### API Documentation
- [ ] **Event System API**:
  - Document all event types and structures
  - Provide callback registration examples
  - Add error handling documentation
- [ ] **Time Service API**:
  - Document timer management functions
  - Provide timing examples
  - Add platform-specific notes
- [ ] **BLE Callback API**:
  - Document callback types and usage
  - Provide message handling examples
  - Add connection state handling examples

#### Example Implementations
- [ ] **Basic Sensor Example**:
  - Implement callback-based sensor example
  - Show timer-based broadcasting
  - Demonstrate state machine usage
- [ ] **Advanced Examples**:
  - Implement multi-sensor example
  - Show complex state transitions
  - Demonstrate error handling
- [ ] **Platform Examples**:
  - Provide Arduino-specific examples
  - Show ESP-IDF implementation
  - Demonstrate native testing

## Implementation Priority

### Phase 1: Core Foundation
1. Event system foundation
2. Basic time service
3. Enhanced BLE callbacks
4. Basic state machine refactoring

### Phase 2: Platform Implementation
1. Arduino platform implementation
2. ESP-IDF platform implementation
3. Native platform implementation
4. Basic testing framework

### Phase 3: Advanced Features
1. Performance optimization
2. Advanced error handling
3. Comprehensive testing
4. Documentation and examples

### Phase 4: Integration and Testing
1. End-to-end testing
2. Performance validation
3. Documentation completion
4. Example validation

## Success Criteria

- [ ] Eliminate manual timing loops in main application code
- [ ] Implement responsive callback-based event handling
- [ ] Support all target platforms (Arduino, ESP-IDF, native)
- [ ] Maintain backward compatibility with existing BLE interface
- [ ] Achieve < 1ms event processing latency
- [ ] Support 100+ concurrent timers
- [ ] Pass comprehensive test suite
- [ ] Provide clear documentation and examples
