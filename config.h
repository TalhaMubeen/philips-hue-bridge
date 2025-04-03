#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "XIAO_Hue_Controller"
#define WIFI_PASSWORD ""
#define WIFI_CONFIG_TIMEOUT 180
#define WIFI_MAX_RETRIES 5  // Added for production stability

// Philips Hue Bridge Configuration
#define HUE_BRIDGE_IP "192.168.1.2"
#define HUE_BRIDGE_PORT 80
#define HUE_RETRY_COUNT 3
#define HUE_RETRY_DELAY 2000
#define HUE_DEVICE_TYPE "XIAO_Hue_Light"
#define HUE_DEVICE_NAME "XIAO LED Controller"
#define HUE_MAX_USERNAME_LENGTH 32
#define HUE_MAX_BRIDGE_IP_LENGTH 16
#define HUE_DISCOVERY_URL "https://discovery.meethue.com"
#define HUE_AUTH_TIMEOUT 30000

// LED Configuration
#define LED_PIN 2
#define NUM_LEDS 10
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define DEFAULT_BRIGHTNESS 128
#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0

// Button Configuration
#define BUTTON_PIN 4
#define BUTTON_LONG_PRESS_DURATION 1000
#define BUTTON_DOUBLE_TAP_DURATION 300
#define BUTTON_DEBOUNCE_TIME 50

// Pattern Configuration
#define NUM_PATTERNS 6
#define BRIGHTNESS_STEPS 5
#define PATTERN_UPDATE_INTERVAL 20

// Battery Management
#define BATTERY_PIN A0
#define BATTERY_CHECK_INTERVAL 300000
#define BATTERY_SAMPLES 10
#define BATTERY_LOW_THRESHOLD 20
#define BATTERY_GOOD_THRESHOLD 80

// Error Handling
#define ERROR_BLINK_INTERVAL 1000
#define WATCHDOG_TIMEOUT 30000000

// Settings Storage
#define SETTINGS_SAVE_INTERVAL 300000

// System Configuration
#define CPU_FREQUENCY 160
#define UPLOAD_SPEED 921600
#define FLASH_SIZE 4
#define PARTITION_SCHEME "Huge APP"

#endif // CONFIG_H