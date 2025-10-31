//! @file examples/arduino/simple_gpio/simple_gpio.ino
//! @brief Simple GPIO example for Arduino using JenLib.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#include <jenlib/gpio/GPIO.h>
#include <jenlib/gpio/drivers/ArduinoGpioDriver.h>
#include <jenlib/time/Time.h>
#include <jenlib/time/drivers/ArduinoTimeDriver.h>

//! @section Pin definitions
enum class PinIndex : std::uint8_t {
    kLedPin = LED_BUILTIN,
    kButtonPin = 2
};

//! @section Driver instances
static jenlib::gpio::ArduinoGpioDriver gpio_driver;
static jenlib::time::ArduinoTimeDriver time_driver;

//! @section GPIO instances
auto led_pin = jenlib::gpio::makeTypedPin<jenlib::gpio::Digital>(static_cast<jenlib::gpio::PinIndex>(PinIndex::kLedPin));
auto button_pin = jenlib::gpio::makeTypedPin<jenlib::gpio::Digital>(static_cast<jenlib::gpio::PinIndex>(PinIndex::kButtonPin));

//! @section Timer ID
static jenlib::time::TimerId blink_timer;

//! @section Arduino setup function
void setup() {
    Serial.begin(115200);
    Serial.println("JenLib Simple GPIO Example");
    
    // Initialize time service
    jenlib::time::Time::initialize();
    
    // Configure GPIO pins
    led_pin.pinMode(jenlib::gpio::PinMode::kOutput);
    button_pin.pinMode(jenlib::gpio::PinMode::kInputPullup);
    
    // Start blinking timer
    blink_timer = jenlib::time::schedule_repeating_timer(1000, [](){
        static bool led_state = false;
        led_state = !led_state;
        led_pin.digitalWrite(led_state ? jenlib::gpio::DigitalValue::HIGH : jenlib::gpio::DigitalValue::LOW);
        Serial.print("LED: ");
        Serial.println(led_state ? "ON" : "OFF");
    });
    
    Serial.println("Setup complete - LED will blink every second");
    Serial.println("Press button to read its state");
}

//! @section Arduino loop function
void loop() {
    // Process timers
    jenlib::time::Time::process_timers();
    
    // Read button state
    jenlib::gpio::DigitalValue button_state = button_pin.digitalRead();
    static jenlib::gpio::DigitalValue last_button_state = jenlib::gpio::DigitalValue::HIGH;
    
    // Detect button press (falling edge)
    if (last_button_state == jenlib::gpio::DigitalValue::HIGH && 
        button_state == jenlib::gpio::DigitalValue::LOW) {
        Serial.println("Button pressed!");
    }
    
    last_button_state = button_state;
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}
