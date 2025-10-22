# Time Service Examples

## Basic Timer Usage

```cpp
#include <jenlib/time/Time.h>

// Initialize time service
jenlib::time::Time::initialize();

// Schedule a one-shot timer
auto timer_id = jenlib::time::Time::schedule_callback(
    1000,  // 1 second
    []() { Serial.println("Timer fired!"); },
    false  // one-shot
);

// Schedule a repeating timer
auto repeat_timer = jenlib::time::Time::schedule_callback(
    500,   // 500ms
    []() { take_sensor_reading(); },
    true   // repeating
);
```

## Main Loop Integration

```cpp
void loop() {
    jenlib::time::Time::process_timers();

    // Get current time
    auto current_time = jenlib::time::Time::now();
}
```

## Timer Management

```cpp
// Cancel a timer
jenlib::time::Time::cancel_callback(timer_id);

// Get active timer count
auto count = jenlib::time::Time::get_active_timer_count();
```
