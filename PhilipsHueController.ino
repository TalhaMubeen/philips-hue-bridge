#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>
#include <ESP32Touch.h>
#include <EEPROM.h>
#include <WiFiManager.h>
#include <ESP32Ping.h>
#include "config.h"

// LED Configuration
#define LED_PIN     2
#define NUM_LEDS    10
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

// Touch Configuration
#define TOUCH_PIN   4
#define TOUCH_THRESHOLD 40

// Pattern Configuration
#define NUM_PATTERNS 6
#define BRIGHTNESS_STEPS 5

// Global Variables
CRGB leds[NUM_LEDS];
AsyncWebServer server(80);
Touch touch(TOUCH_PIN);

// Pattern States
uint8_t currentPattern = 0;
uint8_t currentBrightness = 128;
uint8_t currentColor = 0;
unsigned long lastTouchTime = 0;
unsigned long touchStartTime = 0;
bool isLongPress = false;

// Persistent Storage Structure
struct Settings {
    uint8_t currentPattern;
    uint8_t currentBrightness;
    uint8_t currentColor;
    bool isOn;
    char hueUsername[32];
    char hueBridgeIP[16];
};

Settings settings;
const int EEPROM_SIZE = sizeof(Settings);

// Power Management
#define BATTERY_PIN A0
#define BATTERY_CHECK_INTERVAL 300000  // 5 minutes
unsigned long lastBatteryCheck = 0;
uint8_t batteryLevel = 0;

#define BATTERY_SAMPLES 10
uint16_t batteryReadings[BATTERY_SAMPLES];
uint8_t batteryIndex = 0;

// Watchdog Timer
hw_timer_t *watchdogTimer = NULL;

// Error Handling
bool isError = false;
String errorMessage = "";

// Timing Management
unsigned long lastPatternUpdate = 0;
const unsigned long PATTERN_UPDATE_INTERVAL = 20;
unsigned long lastErrorBlink = 0;
const unsigned long ERROR_BLINK_INTERVAL = 1000;
bool errorLedState = false;

// Pattern Functions
void solidColor() {
    fill_solid(leds, NUM_LEDS, CHSV(currentColor, 255, currentBrightness));
}

void rainbowWave() {
    static uint8_t hue = 0;
    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 256 / NUM_LEDS), 255, currentBrightness);
    }
    hue++;
}

void breathingEffect() {
    static uint8_t brightness = 0;
    static int8_t direction = 1;
    
    brightness += direction;
    if(brightness == 0 || brightness == currentBrightness) direction *= -1;
    
    fill_solid(leds, NUM_LEDS, CHSV(currentColor, 255, brightness));
}

void chaseEffect() {
    static uint8_t pos = 0;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[pos] = CHSV(currentColor, 255, currentBrightness);
    pos = (pos + 1) % NUM_LEDS;
}

void twinkleEffect() {
    static uint8_t twinkle[NUM_LEDS];
    for(int i = 0; i < NUM_LEDS; i++) {
        if(random8() < 128) {
            twinkle[i] = random8();
        }
        leds[i] = CHSV(currentColor, 255, twinkle[i]);
    }
}

void colorCycle() {
    static uint8_t hue = 0;
    fill_solid(leds, NUM_LEDS, CHSV(hue, 255, currentBrightness));
    hue++;
}

// Pattern Array
typedef void (*PatternFunction)();
PatternFunction patterns[] = {
    solidColor,
    rainbowWave,
    breathingEffect,
    chaseEffect,
    twinkleEffect,
    colorCycle
};

// Touch Handling
void handleTouch() {
    if(touch.read() < TOUCH_THRESHOLD) {
        if(!isLongPress) {
            unsigned long touchDuration = millis() - touchStartTime;
            if(touchDuration > 1000) { // Long press
                isLongPress = true;
                currentColor = (currentColor + 32) % 256;
            }
        }
    } else {
        if(isLongPress) {
            isLongPress = false;
        } else {
            unsigned long currentTime = millis();
            if(currentTime - lastTouchTime < 300) { // Double tap
                currentBrightness = (currentBrightness + 51) % 256;
            } else { // Single tap
                currentPattern = (currentPattern + 1) % NUM_PATTERNS;
            }
            lastTouchTime = currentTime;
        }
        touchStartTime = millis();
    }
}

// Philips Hue API Endpoints
void setupHueEndpoints() {
    server.on("/api/newdeveloper/lights", HTTP_GET, [](AsyncWebServerRequest *request){
        String response = "{\"1\":{\"state\":{\"on\":true,\"bri\":" + String(currentBrightness) + 
                         ",\"hue\":" + String(currentColor * 182) + 
                         ",\"sat\":255,\"effect\":\"none\",\"xy\":[0.5,0.5],\"ct\":500,\"alert\":\"none\",\"colormode\":\"hs\",\"reachable\":true},\"type\":\"extended color light\",\"name\":\"XIAO LED Controller\",\"modelid\":\"LCT007\",\"manufacturername\":\"Philips\",\"uniqueid\":\"00:17:88:01:00:d4:12:08-0b\",\"swversion\":\"66009461\",\"pointsymbol\":{\"1\":\"none\",\"2\":\"none\",\"3\":\"none\",\"4\":\"none\",\"5\":\"none\",\"6\":\"none\",\"7\":\"none\",\"8\":\"none\"}}}";
        request->send(200, "application/json", response);
    });

    server.on("/api/newdeveloper/lights/1/state", HTTP_PUT, [](AsyncWebServerRequest *request){
        if(request->hasParam("on", true)) {
            // Handle power state
        }
        if(request->hasParam("bri", true)) {
            currentBrightness = request->getParam("bri", true)->value().toInt();
        }
        if(request->hasParam("hue", true)) {
            currentColor = request->getParam("hue", true)->value().toInt() / 182;
        }
        request->send(200, "application/json", "[{\"success\":{\"/lights/1/state/on\":true}}]");
    });
}

// Load Settings from EEPROM
void loadSettings() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, settings);
    EEPROM.end();
    
    // Validate settings
    if (settings.currentPattern >= NUM_PATTERNS) settings.currentPattern = 0;
    if (settings.currentBrightness > 255) settings.currentBrightness = 128;
    if (settings.currentColor > 255) settings.currentColor = 0;
    
    currentPattern = settings.currentPattern;
    currentBrightness = settings.currentBrightness;
    currentColor = settings.currentColor;
}

// Save Settings to EEPROM
void saveSettings() {
    settings.currentPattern = currentPattern;
    settings.currentBrightness = currentBrightness;
    settings.currentColor = currentColor;
    
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, settings);
    EEPROM.commit();
    EEPROM.end();
}

// Battery Management
void checkBattery() {
    if (millis() - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
        // Take a new reading
        batteryReadings[batteryIndex] = analogRead(BATTERY_PIN);
        batteryIndex = (batteryIndex + 1) % BATTERY_SAMPLES;
        
        // Calculate average
        uint32_t sum = 0;
        for (int i = 0; i < BATTERY_SAMPLES; i++) {
            sum += batteryReadings[i];
        }
        
        uint16_t average = sum / BATTERY_SAMPLES;
        
        // Validate reading
        if (average > 0 && average < 4095) {
            batteryLevel = map(average, 0, 4095, 0, 100);
            
            // Power management
            if (batteryLevel < 20) {
                // Reduce brightness to save power
                currentBrightness = currentBrightness / 2;
                FastLED.setBrightness(currentBrightness);
                
                // Set error state for low battery
                isError = true;
                errorMessage = "Low Battery";
            } else if (batteryLevel > 80) {
                // Clear error state if battery is good
                isError = false;
            }
        }
        
        lastBatteryCheck = millis();
    }
}

// Watchdog Timer ISR
void IRAM_ATTR resetModule() {
    esp_restart();
}

// Initialize Watchdog Timer
void setupWatchdog() {
    watchdogTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(watchdogTimer, &resetModule, true);
    timerAlarmWrite(watchdogTimer, 30000000, true);
    timerAlarmEnable(watchdogTimer);
}

// WiFi Connection Management
void setupWiFi() {
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(180);
    
    int retryCount = 0;
    while (retryCount < 3) {
        if (wifiManager.autoConnect("XIAO_Hue_Controller")) {
            break;
        }
        retryCount++;
        delay(5000);
    }
    
    if (retryCount >= 3) {
        isError = true;
        errorMessage = "Failed to connect to WiFi after 3 attempts";
        return;
    }
    
    // Test Hue Bridge connection with retry
    retryCount = 0;
    while (retryCount < 3) {
        if (Ping.ping(settings.hueBridgeIP)) {
            return;
        }
        retryCount++;
        delay(2000);
    }
    
    isError = true;
    errorMessage = "Hue Bridge not reachable after 3 attempts";
}

// Error Handling
void handleError() {
    if (isError) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastErrorBlink >= ERROR_BLINK_INTERVAL) {
            errorLedState = !errorLedState;
            fill_solid(leds, NUM_LEDS, errorLedState ? CRGB::Red : CRGB::Black);
            FastLED.show();
            lastErrorBlink = currentMillis;
        }
    }
}

// Touch Sensor Calibration
bool calibrateTouch() {
    static int sum = 0;
    static int count = 0;
    static unsigned long lastCalibration = 0;
    
    if (millis() - lastCalibration >= 100) {
        sum += touch.read();
        count++;
        lastCalibration = millis();
        
        if (count >= 10) {
            TOUCH_THRESHOLD = (sum / 10) + 20;
            return true;
        }
    }
    return false;
}

void setup() {
    // Initialize watchdog
    setupWatchdog();
    
    // Load settings
    loadSettings();
    
    // Initialize LED strip
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(currentBrightness);
    
    // Initialize touch sensor
    touch.begin();
    calibrateTouch();
    
    // Setup WiFi
    setupWiFi();
    
    // Setup web server
    setupHueEndpoints();
    server.begin();
    
    // Initialize battery monitoring
    pinMode(BATTERY_PIN, INPUT);
}

void loop() {
    // Reset watchdog timer
    timerWrite(watchdogTimer, 0);
    
    unsigned long currentMillis = millis();
    
    // Check battery
    checkBattery();
    
    // Handle errors
    handleError();
    
    // Handle touch input
    handleTouch();
    
    // Update pattern if interval has elapsed
    if (currentMillis - lastPatternUpdate >= PATTERN_UPDATE_INTERVAL) {
        patterns[currentPattern]();
        FastLED.show();
        lastPatternUpdate = currentMillis;
    }
    
    // Save settings periodically
    static unsigned long lastSave = 0;
    if (currentMillis - lastSave >= 300000) { // Every 5 minutes
        saveSettings();
        lastSave = currentMillis;
    }
}