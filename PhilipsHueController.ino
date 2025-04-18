#include "config.h"
#include <FastLED.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#define ASYNCWEBSERVER_REGEX
#include <ESPAsyncWebServer.h>

// Global Variables
CRGB leds[NUM_LEDS];
AsyncWebServer server(HUE_BRIDGE_PORT);
HTTPClient http;

// Pattern States
uint8_t currentPattern = 0;
uint8_t currentBrightness = DEFAULT_BRIGHTNESS;
uint8_t currentColor = 0;
unsigned long lastButtonTime = 0;
unsigned long buttonStartTime = 0;
bool isLongPress = false;

// Button Variables
bool lastButtonState = false;
bool currentButtonState = false;

// Hue Bridge Authentication
bool isHueAuthenticated = false;
String hueUsername = HUE_DEVICE_TYPE;
String hueBridgeIP = HUE_BRIDGE_IP;

// Persistent Storage Structure
struct Settings {
    uint8_t currentPattern;
    uint8_t currentBrightness;
    uint8_t currentColor;
    bool isOn;
    char hueBridgeIP[HUE_MAX_BRIDGE_IP_LENGTH];
};
Settings settings;
const int EEPROM_SIZE = sizeof(Settings);

// Battery and Timing
unsigned long lastBatteryCheck = 0;
unsigned long lastPatternUpdate = 0;
unsigned long lastDiscoveryAttempt = 0;

// Pattern Functions
void solidColor() {
    fill_solid(leds, NUM_LEDS, CHSV(currentColor, 255, currentBrightness));
}

void rainbowWave() {
    static uint8_t hue = 0;
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 256 / NUM_LEDS), 255, currentBrightness);
    }
    hue++;
}

void breathingEffect() {
    static uint8_t brightness = 0;
    static int8_t direction = 1;
    brightness += direction;
    if (brightness == 0 || brightness == currentBrightness) direction *= -1;
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
    for (int i = 0; i < NUM_LEDS; i++) {
        if (random8() < 128) twinkle[i] = random8();
        leds[i] = CHSV(currentColor, 255, twinkle[i]);
    }
}

void colorCycle() {
    static uint8_t hue = 0;
    fill_solid(leds, NUM_LEDS, CHSV(hue, 255, currentBrightness));
    hue++;
}

typedef void (*PatternFunction)();
PatternFunction patterns[] = {solidColor, rainbowWave, breathingEffect, chaseEffect, twinkleEffect, colorCycle};

// Button Functions
void handleButton() {
    static unsigned long lastDebounceTime = 0;
    static bool lastRawButtonState = false;
    bool rawButtonState = digitalRead(BUTTON_PIN);
    Serial.println("Raw button state on pin " + String(BUTTON_PIN) + ": " + String(rawButtonState));

    static unsigned long lastChangeTime = 0;
    static bool lastLoggedState = rawButtonState;
    if (rawButtonState != lastLoggedState) {
        lastChangeTime = millis();
        lastLoggedState = rawButtonState;
    }
    if (millis() - lastChangeTime > 10000) {
        Serial.println("Warning: Button state on pin " + String(BUTTON_PIN) + " has not changed in 10 seconds. Possible wiring or pin issue.");
    }

    if (rawButtonState != lastRawButtonState) {
        lastDebounceTime = millis();
        Serial.println("Debounce triggered, time: " + String(lastDebounceTime));
    }

    if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_TIME) {
        currentButtonState = !rawButtonState;
        Serial.println("Current button state: " + String(currentButtonState));
        if (currentButtonState && !lastButtonState) {
            unsigned long currentTime = millis();
            if (currentTime - lastButtonTime < BUTTON_DOUBLE_TAP_DURATION) {
                currentBrightness = (currentBrightness + (MAX_BRIGHTNESS / BRIGHTNESS_STEPS)) % (MAX_BRIGHTNESS + 1);
                FastLED.setBrightness(currentBrightness);
                saveSettings();
                Serial.println("Double tap - brightness changed to " + String(currentBrightness));
            } else {
                currentPattern = (currentPattern + 1) % NUM_PATTERNS;
                saveSettings();
                Serial.println("Single tap - pattern changed to " + String(currentPattern));
            }
            lastButtonTime = currentTime;
            buttonStartTime = currentTime;
        } else if (currentButtonState && (millis() - buttonStartTime > BUTTON_LONG_PRESS_DURATION)) {
            if (!isLongPress) {
                isLongPress = true;
                currentColor = (currentColor + 32) % 256;
                saveSettings();
                Serial.println("Long press - color changed to " + String(currentColor));
            }
        } else if (!currentButtonState && isLongPress) {
            isLongPress = false;
        }
        lastButtonState = currentButtonState;
    }
    lastRawButtonState = rawButtonState;
}

// mDNS Discovery for Hue Bridge
bool discoverHueBridgeViaMDNS() {
    try {
        Serial.println("Starting mDNS discovery for Hue Bridge...");
        if (!MDNS.begin("xiao-hue")) {
            Serial.println("Error setting up mDNS responder!");
            return false;
        }

        // Try _hue._tcp first
        Serial.println("Querying for _hue._tcp...");
        int n = MDNS.queryService("hue", "tcp");
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                String name = MDNS.hostname(i);
                IPAddress ip = MDNS.address(i);
                String ipStr = ip.toString();
                Serial.println("Found Hue Bridge: " + name + " at IP: " + ipStr);
                String bridgeid = MDNS.txt(i, "bridgeid");
                String modelid = MDNS.txt(i, "modelid");
                Serial.println("Bridge ID: " + bridgeid + ", Model ID: " + modelid);
                if (name.startsWith("Philips Hue")) {
                    hueBridgeIP = ipStr;
                    Serial.println("Selected Hue Bridge IP: " + hueBridgeIP);
                    strncpy(settings.hueBridgeIP, hueBridgeIP.c_str(), HUE_MAX_BRIDGE_IP_LENGTH);
                    saveSettings();
                    return true;
                }
            }
        } else {
            Serial.println("No Hue Bridges found via _hue._tcp");
        }

        // Fall back to _huesync._tcp
        Serial.println("Querying for _huesync._tcp...");
        n = MDNS.queryService("huesync", "tcp");
        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                String name = MDNS.hostname(i);
                IPAddress ip = MDNS.address(i);
                String ipStr = ip.toString();
                Serial.println("Found HueSync device: " + name + " at IP: " + ipStr);
                String devicetype = MDNS.txt(i, "devicetype");
                String uniqueid = MDNS.txt(i, "uniqueid");
                Serial.println("Device Type: " + devicetype + ", Unique ID: " + uniqueid);
                if (devicetype.indexOf("bridge") >= 0 || name.startsWith("Philips Hue")) {
                    hueBridgeIP = ipStr;
                    Serial.println("Selected Hue Bridge IP via _huesync: " + hueBridgeIP);
                    strncpy(settings.hueBridgeIP, hueBridgeIP.c_str(), HUE_MAX_BRIDGE_IP_LENGTH);
                    saveSettings();
                    return true;
                }
            }
        } else {
            Serial.println("No HueSync devices found via _huesync._tcp");
        }

        Serial.println("No matching Hue Bridge found via mDNS");
        return false;
    } catch (const std::exception& e) {
        Serial.println("Error in mDNS discovery: " + String(e.what()));
        return false;
    }
}

// Cloud Discovery for Hue Bridge
bool discoverHueBridgeViaCloud() {
    try {
        Serial.println("Attempting Hue Bridge discovery via Cloud...");
        http.begin(HUE_DISCOVERY_URL);
        http.setTimeout(10000);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Discovery response: " + payload);
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (error) {
                Serial.println("JSON parsing failed: " + String(error.c_str()));
                return false;
            }
            if (doc.size() > 0) {
                hueBridgeIP = doc[0]["internalipaddress"].as<String>();
                Serial.println("Discovered Hue Bridge IP: " + hueBridgeIP);
                strncpy(settings.hueBridgeIP, hueBridgeIP.c_str(), HUE_MAX_BRIDGE_IP_LENGTH);
                saveSettings();
                return true;
            } else {
                Serial.println("No Hue Bridge found in discovery response");
                return false;
            }
        } else {
            Serial.println("Hue discovery failed: " + String(httpCode));
            return false;
        }
    } catch (const std::exception& e) {
        Serial.println("Error in cloud discovery: " + String(e.what()));
        return false;
    }
    http.end();  // Moved outside try-catch to ensure cleanup
    return false;
}

// Hue Bridge Authentication
bool authenticateHueBridge() {
    try {
        if (hueBridgeIP == HUE_BRIDGE_IP || hueBridgeIP.length() == 0) {
            if (discoverHueBridgeViaMDNS()) {
                Serial.println("mDNS discovery successful, using IP: " + hueBridgeIP);
            } else {
                Serial.println("mDNS discovery failed, falling back to Cloud discovery...");
                if (millis() - lastDiscoveryAttempt >= 900000) {
                    if (discoverHueBridgeViaCloud()) {
                        lastDiscoveryAttempt = millis();
                        Serial.println("Cloud discovery successful, using IP: " + hueBridgeIP);
                    } else {
                        Serial.println("Cloud discovery failed");
                        return false;
                    }
                } else {
                    Serial.println("Cloud discovery skipped due to rate limit");
                    return false;
                }
            }
        }

        Serial.println("Attempting authentication with Hue Bridge at: " + hueBridgeIP);
        String url = "http://" + hueBridgeIP + "/api";
        String payload = "{\"devicetype\":\"" + String(HUE_DEVICE_TYPE) + "\"}";
        Serial.println("POST URL: " + url);
        Serial.println("POST Payload: " + payload);
        http.begin(url);
        http.setTimeout(10000);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);
        if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            Serial.println("Auth response: " + response);
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, response);
            if (error) {
                Serial.println("JSON parsing failed: " + String(error.c_str()));
                return false;
            }
            if (doc[0]["success"]) {
                hueUsername = doc[0]["success"]["username"].as<String>();
                isHueAuthenticated = true;
                Serial.println("Hue authenticated with username: " + hueUsername);
                return true;
            } else {
                Serial.println("Auth response contains no success field");
                if (doc[0]["error"]["type"].as<int>() == 101) {
                    Serial.println("Link button not pressed on Hue Bridge. Please press the link button and try again.");
                }
            }
        }
        Serial.println("Hue auth failed: " + String(httpCode));
        return false;
    } catch (const std::exception& e) {
        Serial.println("Error in Hue authentication: " + String(e.what()));
        return false;
    }
    http.end();  // Moved outside try-catch to ensure cleanup
    return false;
}

// Hue API Endpoints
void setupHueEndpoints() {
    server.on("/api/newdeveloper/lights", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!isHueAuthenticated) {
            request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
            return;
        }
        String response = "{\"1\":{\"state\":{\"on\":" + String(settings.isOn ? "true" : "false") +
                          ",\"bri\":" + String(currentBrightness) +
                          ",\"hue\":" + String(currentColor * 182) +
                          ",\"sat\":255,\"effect\":\"pattern" + String(currentPattern) + "\"}," +
                          "\"type\":\"Extended color light\",\"name\":\"" + String(HUE_DEVICE_NAME) + "\"," +
                          "\"modelid\":\"LCT007\",\"manufacturername\":\"Seeed\"}}";
        request->send(200, "application/json", response);
    });

    server.on("/api/newdeveloper/lights/1/state", HTTP_PUT, [](AsyncWebServerRequest *request) {
        if (!isHueAuthenticated) {
            request->send(401, "application/json", "{\"error\":\"Unauthorized\"}");
            return;
        }
        if (request->hasParam("on", true)) settings.isOn = request->getParam("on", true)->value() == "true";
        if (request->hasParam("bri", true)) currentBrightness = constrain(request->getParam("bri", true)->value().toInt(), MIN_BRIGHTNESS, MAX_BRIGHTNESS);
        if (request->hasParam("hue", true)) currentColor = constrain(request->getParam("hue", true)->value().toInt() / 182, 0, 255);
        if (request->hasParam("effect", true)) {
            String effect = request->getParam("effect", true)->value();
            for (int i = 0; i < NUM_PATTERNS; i++) {
                if (effect == "pattern" + String(i)) {
                    currentPattern = i;
                    break;
                }
            }
        }
        FastLED.setBrightness(currentBrightness);
        saveSettings();
        request->send(200, "application/json", "[{\"success\":{\"/lights/1/state/on\":" + String(settings.isOn ? "true" : "false") + "}}]");
    });
}

// Load/Save Settings
void loadSettings() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(0, settings);
    EEPROM.end();
    currentPattern = constrain(settings.currentPattern, 0, NUM_PATTERNS - 1);
    currentBrightness = constrain(settings.currentBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    currentColor = settings.currentColor;
    hueBridgeIP = String(settings.hueBridgeIP);
    if (hueBridgeIP.length() == 0) hueBridgeIP = HUE_BRIDGE_IP;
}

void saveSettings() {
    settings.currentPattern = currentPattern;
    settings.currentBrightness = currentBrightness;
    settings.currentColor = currentColor;
    settings.isOn = true;
    strncpy(settings.hueBridgeIP, hueBridgeIP.c_str(), HUE_MAX_BRIDGE_IP_LENGTH);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, settings);
    EEPROM.commit();
    EEPROM.end();
}

// Battery Management
void checkBattery() {
    if (millis() - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
        int batteryValue = 0;
        for (int i = 0; i < BATTERY_SAMPLES; i++) {
            batteryValue += analogRead(BATTERY_PIN);
            delay(10);
        }
        batteryValue /= BATTERY_SAMPLES;
        int batteryPercent = map(batteryValue, 0, 4095, 0, 100);
        if (batteryPercent < BATTERY_LOW_THRESHOLD) {
            currentBrightness = min(currentBrightness, (uint8_t)(MAX_BRIGHTNESS / 4));
            FastLED.setBrightness(currentBrightness);
            Serial.println("Low battery: " + String(batteryPercent) + "%");
        }
        lastBatteryCheck = millis();
    }
}

// WiFi Setup
void setupWiFi() {
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(WIFI_CONFIG_TIMEOUT);
    int retries = 0;
    while (!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD) && retries < WIFI_MAX_RETRIES) {
        Serial.println("WiFi connection failed, retrying...");
        retries++;
        delay(5000);
    }
    if (retries >= WIFI_MAX_RETRIES) {
        Serial.println("WiFi failed after max retries, restarting...");
        ESP.restart();
    }
}

// Error Indication
void indicateError() {
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    if (millis() - lastBlink >= ERROR_BLINK_INTERVAL) {
        ledState = !ledState;
        fill_solid(leds, NUM_LEDS, ledState ? CRGB::Red : CRGB::Black);
        FastLED.show();
        lastBlink = millis();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(currentBrightness);
    settings.isOn = true;
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();

    loadSettings();
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BATTERY_PIN, INPUT);

    setupWiFi();
    if (!isHueAuthenticated) {
        for (int i = 0; i < HUE_RETRY_COUNT; i++) {
            if (authenticateHueBridge()) break;
            delay(HUE_RETRY_DELAY);
            if (i == HUE_RETRY_COUNT - 1) indicateError();
        }
    }
    setupHueEndpoints();
    server.begin();

    Serial.println("System initialized");
}

void loop() {
    unsigned long currentMillis = millis();

    try {
        if (!WiFi.isConnected()) {
            indicateError();
            setupWiFi();
            Serial.println("WiFi disconnected, attempting reconnect");
        }
        if (!isHueAuthenticated) {
            indicateError();
            Serial.println("Hue not authenticated");
            if (currentMillis - lastDiscoveryAttempt >= 60000) {
                if (authenticateHueBridge()) {
                    Serial.println("Hue authentication successful after retry");
                }
                lastDiscoveryAttempt = currentMillis;
            }
        }

        checkBattery();
        handleButton();

        if (currentMillis - lastPatternUpdate >= PATTERN_UPDATE_INTERVAL) {
            Serial.println("Pattern: " + String(currentPattern) + ", Brightness: " + String(currentBrightness) + ", On: " + String(settings.isOn));
            if (settings.isOn) patterns[currentPattern]();
            else fill_solid(leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
            lastPatternUpdate = currentMillis;
        }

        static unsigned long lastSave = 0;
        if (currentMillis - lastSave >= SETTINGS_SAVE_INTERVAL) {
            saveSettings();
            lastSave = currentMillis;
        }
    } catch (const std::exception& e) {
        Serial.println("Error in loop: " + String(e.what()));
        indicateError();
    }
}