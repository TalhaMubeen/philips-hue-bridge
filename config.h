#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "XIAO_Hue_Controller"  // Default AP name for configuration
#define WIFI_PASSWORD ""                  // No password for configuration AP
#define WIFI_CONFIG_TIMEOUT 180           // 3 minutes timeout for configuration portal

// Philips Hue Bridge Configuration
#define HUE_BRIDGE_IP "192.168.1.2"      // Replace with your Hue Bridge IP
#define HUE_BRIDGE_PORT 80               // Default Hue Bridge port
#define HUE_RETRY_COUNT 3                // Number of connection retries
#define HUE_RETRY_DELAY 2000             // Delay between retries in ms
#define HUE_DEVICE_TYPE "XIAO_Hue_Light" // Device type for Hue Bridge registration
#define HUE_DEVICE_NAME "XIAO LED Controller" // Device name for Hue Bridge
#define HUE_MAX_USERNAME_LENGTH 32       // Maximum length for Hue username
#define HUE_MAX_BRIDGE_IP_LENGTH 16      // Maximum length for Hue Bridge IP
#define HUE_DISCOVERY_URL "https://discovery.meethue.com" // Hue Bridge discovery URL
#define HUE_AUTH_TIMEOUT 30000           // Timeout for authentication in ms (30 seconds)

// LED Configuration
#define LED_PIN 2                        // GPIO pin for WS2812 LEDs
#define NUM_LEDS 10                      // Number of LEDs in the strip
#define LED_TYPE WS2812                  // LED type
#define COLOR_ORDER GRB                  // Color order for the LED strip
#define DEFAULT_BRIGHTNESS 128           // Default brightness (0-255)
#define MAX_BRIGHTNESS 255               // Maximum brightness
#define MIN_BRIGHTNESS 0                 // Minimum brightness

// Touch Configuration
#define TOUCH_PIN 4                      // GPIO pin for touch sensor
#define TOUCH_THRESHOLD 40               // Default touch threshold
#define TOUCH_CALIBRATION_SAMPLES 10     // Number of samples for calibration
#define TOUCH_CALIBRATION_INTERVAL 100   // Interval between calibration samples in ms
#define TOUCH_LONG_PRESS_DURATION 1000   // Duration for long press in ms
#define TOUCH_DOUBLE_TAP_DURATION 300    // Duration for double tap in ms

// Pattern Configuration
#define NUM_PATTERNS 6                   // Total number of available patterns
#define BRIGHTNESS_STEPS 5               // Number of brightness adjustment steps
#define PATTERN_UPDATE_INTERVAL 20       // Pattern update interval in ms

// Battery Management
#define BATTERY_PIN A0                   // Analog pin for battery monitoring
#define BATTERY_CHECK_INTERVAL 300000    // Battery check interval in ms (5 minutes)
#define BATTERY_SAMPLES 10               // Number of samples for battery reading
#define BATTERY_LOW_THRESHOLD 20         // Low battery threshold percentage
#define BATTERY_GOOD_THRESHOLD 80        // Good battery threshold percentage

// Error Handling
#define ERROR_BLINK_INTERVAL 1000        // Error LED blink interval in ms
#define WATCHDOG_TIMEOUT 30000000        // Watchdog timeout in microseconds (30 seconds)

// Settings Storage
#define SETTINGS_SAVE_INTERVAL 300000    // Settings save interval in ms (5 minutes)

// System Configuration
#define CPU_FREQUENCY 160                // CPU frequency in MHz
#define UPLOAD_SPEED 921600              // Upload speed in baud
#define FLASH_SIZE 4                     // Flash size in MB
#define PARTITION_SCHEME "Huge APP"      // Partition scheme

#endif // CONFIG_H 