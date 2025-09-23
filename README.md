# Development library for the G1-H25 embedded project

## Overview

We have two different embedded architectures to work towards, that need to share a common communication library. A sensor (specified as Arduino R4 Wifi in the schoolwork requirements) and a broker (specified as Waveshare Esp32-S3-Zero) will communicate over bluetooth low energy due to some requirements of supporting 100 concurrent wireless sensors.

The goal of the library is to share code for communication and sensor readouts between the different components. 

## Example

```cpp
//! @section GPIO setup
static constexpr jenlib::gpio::ArduinoGpioDriver kArduinoDriver;
jenlib::gpio::GPIO Gpio(kArduinoDriver);

//! @section Pin defintion
enum class PinIndex : std::uint8_t {
    kLedPin = 3;
}

auto LedPin = jenlib::gpio::makeTypedPin<Digital>(kLedPin);

//! @section Time setup
static jenlib::time::ArduinoTimeDriver kTimeDriver;

//! @section Arduino functions

//! @brief Setup function
void setup(){
    jenlib::time::Time::setDriver(&kTimeDriver);

    LedPin.pinMode(jenlib::gpio::PinMode::kOutput);

    static jenlib::time::TimerId kBlinkTimer = jenlib::time::schedule_repeating_timer(1000, [](){
        static bool is_on = false;
        is_on = !is_on;
        LedPin.digitalWrite(is_on ? jenlib::gpio::DigitalValue::HIGH : jenlib::gpio::DigitalValue::LOW);
    });
}

//! @brief Main loop
void loop(){
    jenlib::time::Time::process_timers();
}

```
     

## Components

### GPIO abstraction
Strongly typed GPIO interface. No raw ints passed around for pin numbers or modes.

### Time and timer
Class for getting current device time and callback function for set time intervals

### State machine
Classes for state machines for the sensor and broker that share underlying mechanisms

### Event system for sensor and broker
Callback based event system for handling the different functions
