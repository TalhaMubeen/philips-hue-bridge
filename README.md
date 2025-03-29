# Philips Hue LED Controller for XIAO ESP32C3

This project implements a Philips Hue-compatible LED controller using the Seeed Studio XIAO ESP32C3 microcontroller. It controls 10 WS2812 RGB LEDs and provides touch-based pattern control.

## Features

- Philips Hue Bridge integration with automatic reconnection
- Control of 10 WS2812 RGB LEDs
- Capacitive touch input for pattern control:
  - Single tap: Switch between patterns
  - Long press: Cycle through colors
  - Double tap: Adjust brightness
- Persistent pattern storage using EEPROM
- Battery-powered operation with USB charging support
- Power management and battery monitoring
- Automatic WiFi configuration with retry mechanism
- Error handling and recovery
- Watchdog timer for system stability
- Touch sensor auto-calibration
- Visual error indicators for system status

## Hardware Requirements

- Seeed Studio XIAO ESP32C3
- 10 WS2812 RGB LEDs
- Battery pack (3 AAA or rechargeable battery)
- USB charging circuit
- Capacitive touch sensor
- Battery voltage monitoring circuit (optional)

## Software Requirements

- Arduino IDE with ESP32 support
- Required Libraries:
  - FastLED@3.5.0
  - ESPAsyncWebServer@1.2.3
  - ESPAsyncTCP@1.2.2
  - ArduinoJson@6.21.3
  - ESP32Touch@1.0.0
  - WiFiManager@2.0.16
  - ESP32Ping@1.7
  - EEPROM@1.0.1

## Setup Instructions

1. Install the required libraries in Arduino IDE
2. Configure Arduino IDE settings:
   - Enable EEPROM
   - Set Flash Size to 4MB
   - Set Partition Scheme to "Huge APP"
   - Set CPU Frequency to 160MHz
   - Set Upload Speed to 921600
3. Connect the WS2812 LEDs to GPIO 2
4. Connect the capacitive touch sensor to GPIO 4
5. Connect battery monitoring circuit to A0 (optional)
6. Upload the firmware to your XIAO ESP32C3
7. On first boot:
   - Connect to the "XIAO_Hue_Controller" WiFi network
   - Configure your WiFi credentials in the web portal
   - Wait for automatic connection to your network
8. Connect to the Philips Hue Bridge using the Hue app

## Hue Bridge Setup

1. First-time Connection:
   - Power on your Hue Bridge
   - Press the link button on your Hue Bridge
   - Power on the XIAO LED Controller
   - The controller will automatically discover and connect to your Hue Bridge
   - The link button must be pressed within 30 seconds of powering on the controller

2. Authentication Process:
   - The controller will automatically discover your Hue Bridge on the network
   - It will register itself as a new device
   - The generated username will be stored in EEPROM
   - No manual configuration is required

3. Troubleshooting Hue Connection:
   - If the controller shows "Press link button on Hue Bridge":
     1. Press the link button on your Hue Bridge
     2. Wait for the controller to authenticate
   - If the controller shows "Could not discover Hue Bridge":
     1. Ensure the Hue Bridge is powered on
     2. Check if the controller and bridge are on the same network
     3. Try power cycling both devices
   - If authentication fails:
     1. Power cycle both devices
     2. Press the link button on the Hue Bridge
     3. Wait for the controller to reconnect

## Pin Configuration

- WS2812 LEDs: GPIO 2
- Touch Sensor: GPIO 4
- Battery Monitor: A0 (optional)
- USB: For programming and charging

## Usage

1. Power on the device
2. Connect to the Philips Hue Bridge using the Hue app
3. Use touch gestures to control patterns:
   - Single tap to cycle through patterns
   - Long press to change colors
   - Double tap to adjust brightness

## Error Handling

The device includes visual error indicators:
- Red LED flash: WiFi connection error
- Alternating red/black: Hue Bridge connection error
- System will automatically attempt to recover from errors
- Watchdog timer will restart the device if it becomes unresponsive

## Power Management

- Automatic brightness reduction at low battery
- Periodic battery level monitoring with averaging
- Power-efficient LED control
- Settings persistence to maintain state after power cycles
- Visual indication of low battery status

## Patterns

The device includes several predefined patterns:
- Solid Color
- Rainbow Wave
- Breathing Effect
- Chase Effect
- Twinkle Effect
- Color Cycle

## Troubleshooting

1. If the device doesn't connect to WiFi:
   - Power cycle the device
   - Connect to the "XIAO_Hue_Controller" network
   - Reconfigure WiFi settings
   - System will retry connection 3 times

2. If touch sensor is not responsive:
   - Power cycle the device to trigger auto-calibration
   - Ensure the touch sensor is properly connected
   - Check for error indicators

3. If LEDs don't respond:
   - Check the LED connection
   - Verify the power supply
   - Check for error indicators
   - Ensure the pattern is not set to "off"

4. If battery level is low:
   - Connect to USB power
   - Brightness will automatically reduce to save power
   - Visual indicator will show low battery status