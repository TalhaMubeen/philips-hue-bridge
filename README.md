# Philips Hue LED Controller for XIAO ESP32C3

This project implements a Philips Hue-compatible LED controller using the Seeed Studio XIAO ESP32C3 microcontroller. It controls 10 WS2812 RGB LEDs with predefined patterns, supports physical button input for local control, and integrates with the Philips Hue iOS app for remote control.

## Features

- **Philips Hue Integration**: Connects to the Hue Bridge and appears as a controllable light in the Hue iOS app.
- **LED Control**: Drives 10 WS2812 RGB LEDs with 6 predefined patterns.
- **Physical Button Input**:
  - Single tap: Cycle through patterns.
  - Long press: Change colors.
  - Double tap: Adjust brightness in steps.
- **Persistent Storage**: Saves pattern, color, and brightness settings to EEPROM across power cycles.
- **Battery-Powered**: Supports operation with a battery pack (e.g., 3 AAA or rechargeable via USB).
- **Power Management**: Monitors battery level and reduces brightness when low.
- **WiFi Configuration**: Automatic setup via a captive portal with WiFiManager.

## Hardware Requirements

- Seeed Studio XIAO ESP32C3
- 10 WS2812 RGB LEDs (connected to GPIO 2)
- Physical button (connected to GPIO 4)
- Battery pack (3 AAA or rechargeable battery with USB charging via XIAO's built-in circuit)
- Battery voltage monitoring circuit (optional, connected to A0)

## Software Requirements

- Arduino IDE with ESP32 support
  ESP32 Core by Espressif@2.0.17
- Required Libraries:
  - `FastLED@3.9.14`
  - `ESPAsyncWebServer@2.0.1`
  - `WiFiManager@2.0.17`
  - `ArduinoJson@7.3.1`
  - `EEPROM` (built-in with ESP32 core)
  - `HTTPClient` (built-in with ESP32 core)

## Setup Instructions

1. **Install Libraries**:
   - Open Arduino IDE, go to **Sketch > Include Library > Manage Libraries**, and install:
     - `FastLED`
     - `ESPAsyncWebServer`
     - `WiFiManager`
     - `ArduinoJson`
   - `EEPROM` and `HTTPClient` are included with the ESP32 core.

2. **Configure Arduino IDE**:
   - **Board**: Seeed Studio XIAO ESP32C3
   - **Flash Size**: 4MB
   - **Partition Scheme**: Huge APP
   - **CPU Frequency**: 160MHz
   - **Upload Speed**: 921600

3. **Hardware Connections**:
   - Connect WS2812 LEDs data line to GPIO 2.
   - Connect a physical button between GPIO 4 and GND (using internal pull-up resistor).
   - (Optional) Connect a battery monitoring circuit to A0 (e.g., via a voltage divider for 3 AAA batteries).
   - Power the XIAO and LEDs via a battery pack or USB.

4. **Upload Firmware**:
   - Place `config.h` and the `.ino` file in the same directory.
   - Open the sketch in Arduino IDE, compile, and upload to the XIAO ESP32C3.

5. **WiFi Setup**:
   - On first boot, connect to the "XIAO_Hue_Controller" WiFi network.
   - Open a browser and configure your WiFi credentials in the captive portal.
   - The device will connect to your network automatically.

6. **Hue Bridge Pairing**:
   - Power on your Hue Bridge and press the link button.
   - Power on the XIAO within 30 seconds.
   - The controller will discover and register with the Hue Bridge.

## Hue Bridge Setup

1. **First-time Connection**:
   - Ensure the Hue Bridge is powered on and connected to the same network.
   - Press the link button on the Hue Bridge.
   - Power on the XIAO LED Controller; it will automatically discover and authenticate with the bridge.
   - The generated username is stored in EEPROM for future connections.

2. **Using the Hue App**:
   - Open the Philips Hue iOS app.
   - Add the device as a new light (it appears as "XIAO LED Controller").
   - Control on/off, brightness, and hue via the app.

3. **Troubleshooting Hue Connection**:
   - If pairing fails:
     1. Ensure the Hue Bridge is on and the link button was pressed.
     2. Verify the XIAO and Hue Bridge are on the same WiFi network.
     3. Power cycle both devices and retry.

## Pin Configuration

- **WS2812 LEDs**: GPIO 2
- **Physical Button**: GPIO 4 (with internal pull-up resistor)
- **Battery Monitor**: A0 (optional)
- **USB**: For programming and charging (via XIAO's built-in circuit)

## Button Operation

The physical button supports three types of interactions:
1. **Single Tap**: Cycles through the available LED patterns
2. **Double Tap**: Adjusts the brightness in steps
3. **Long Press**: Changes the color of the current pattern

The button uses internal debouncing to prevent false triggers and has configurable timing parameters in `config.h`:
- `BUTTON_LONG_PRESS_DURATION`: Time required for a long press (default: 1000ms)
- `BUTTON_DOUBLE_TAP_DURATION`: Maximum time between taps for double tap (default: 300ms)
- `BUTTON_DEBOUNCE_TIME`: Debounce time to prevent false triggers (default: 50ms)

## Usage

1. **Power On**:
   - Connect the battery pack or USB power.
   - The device loads the last saved pattern, color, and brightness.

2. **Button Controls**:
   - **Single Tap**: Switch to the next pattern (6 available).
   - **Long Press**: Cycle through colors (hue increments by 32).
   - **Double Tap**: Adjust brightness (steps through 5 levels).

3. **Hue App Controls**:
   - Turn the light on/off.
   - Adjust brightness (0-255).
   - Change hue (mapped to color).

## Power Management

- **Battery Monitoring**: Checks voltage on A0 every 5 minutes; reduces brightness if below 20%.
- **Efficiency**: Settings are saved every 5 minutes to minimize EEPROM writes.
- **Low Battery**: Brightness is capped at 25% of maximum when battery is low.

## Patterns

The device includes 6 predefined patterns:
1. **Solid Color**: Uniform color across all LEDs.
2. **Rainbow Wave**: Gradual color shift across LEDs.
3. **Breathing Effect**: Pulsing brightness.
4. **Chase Effect**: Single lit LED moves along the strip.
5. **Twinkle Effect**: Random twinkling lights.
6. **Color Cycle**: Smooth hue transition across all LEDs.

Patterns are cycled via button input; the Hue app controls basic on/off, brightness, and hue.

## Troubleshooting

1. **WiFi Connection Issues**:
   - If it doesn't connect, power cycle the device and reconnect to "XIAO_Hue_Controller" to reconfigure WiFi.
   - Ensure the network is 2.4GHz (ESP32C3 doesn't support 5GHz).

2. **Button Not Responding**:
   - Verify the button is connected to GPIO 4 and the internal pull-up resistor is in place.
   - Check for loose connections or interference.
   - Power cycle to reset.

3. **LEDs Not Working**:
   - Confirm the WS2812 data line is on GPIO 2 and power is sufficient.
   - Check if the device is set to "off" via the Hue app.

4. **Battery Issues**:
   - If using 3 AAA batteries, ensure voltage is within the XIAO's 3.3V-5V range.
   - For rechargeable setups, connect USB to charge.
   - Adjust the `checkBattery()` threshold in the code if needed.

---

### Key Updates to `README.md`
1. **Simplified Features**: Removed unused features like watchdog timer, error blinking, and touch auto-calibration (not implemented in the final code).
2. **Updated Libraries**: Removed `ESPAsyncTCP`, `ESP32Touch`, and `ESP32Ping` as they're not used.
3. **Hardware**: Clarified battery options and optional monitoring circuit.
4. **Usage**: Aligned with the button controls (single tap, long press, double tap) and Hue app functionality.
5. **Patterns**: Listed the 6 implemented patterns, noting local vs. app control.
6. **Troubleshooting**: Streamlined to focus on common issues with the current setup.
