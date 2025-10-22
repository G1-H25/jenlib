# Event System Examples

## Basic Event Handling

```cpp
#include <jenlib/events/EventDispatcher.h>

// Global event dispatcher
jenlib::events::EventDispatcher event_dispatcher;

// Register event handlers
event_dispatcher.register_callback(
    jenlib::events::EventType::kBleMessage,
    [](const jenlib::events::Event& event) {
        // Handle BLE message event
    });

// Dispatch events
jenlib::events::Event event(
    jenlib::events::EventType::kConnectionStateChange,
    jenlib::time::Time::now(),
    connected ? 1 : 0);
event_dispatcher.dispatch_event(event);
```

## Main Loop Integration

```cpp
void loop() {
    event_dispatcher.process_events();
    sensor.process_events();
    jenlib::time::Time::process_timers();
}
```

## Event Types

- `kBleMessage` - BLE communication events
- `kConnectionStateChange` - Connection status changes
- `kTimeTick` - Timer-based events
- `kCustom` - User-defined events
