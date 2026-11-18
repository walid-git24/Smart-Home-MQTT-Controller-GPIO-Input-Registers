#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "soc/soc.h"
#include "soc/gpio_reg.h"

// WiFi
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// MQTT
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "asl/msti-arena/ch02/ak-7f3a/command"

// Input pins
#define BUTTON_1 27  // Red button
#define BUTTON_2 34  // Blue button

// LED pins
#define LED_1 26
#define LED_2 25

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// LED states
bool led1State = LOW;
bool led2State = LOW;

// Button debounce
const unsigned long DEBOUNCE_MS = 30;

bool lastButton1State = HIGH;
bool lastButton2State = HIGH;

bool stableButton1State = HIGH;
bool stableButton2State = HIGH;

unsigned long lastButton1Change = 0;
unsigned long lastButton2Change = 0;

unsigned long lastMqttAttempt = 0;

// Update LED outputs
void updateLEDs() {
    digitalWrite(LED_1, led1State);
    digitalWrite(LED_2, led2State);
}

// MQTT message handler
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;

    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    message.trim();

    Serial.print("MQTT Command: ");
    Serial.println(message);

    if (message == "ON") {
        led1State = HIGH;
        led2State = HIGH;
    }
    else if (message == "OFF") {
        led1State = LOW;
        led2State = LOW;
    }
    else {
        Serial.println("Unknown MQTT command");
        return;
    }

    updateLEDs();

    Serial.println("Both LEDs updated");
}

// MQTT connection
void reconnectMQTT() {
    if (mqttClient.connected()) return;

    if (millis() - lastMqttAttempt < 5000) return;

    lastMqttAttempt = millis();

    String clientId = "ASL-CH02-" +
                      String((uint32_t)esp_random(), HEX);

    Serial.print("Connecting to MQTT... ");

    if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected");

        mqttClient.subscribe(MQTT_TOPIC);

        Serial.print("Subscribed to: ");
        Serial.println(MQTT_TOPIC);
    }
    else {
        Serial.print("Failed. State: ");
        Serial.println(mqttClient.state());
    }
}

// Handle button debounce and toggle
void handleButton(
    bool rawState,
    bool &lastState,
    bool &stableState,
    unsigned long &lastChange,
    bool &ledState,
    int ledPin,
    const char* ledName
) {
    unsigned long now = millis();

    if (rawState != lastState) {
        lastChange = now;
        lastState = rawState;
    }

    if (now - lastChange >= DEBOUNCE_MS) {
        if (rawState != stableState) {
            stableState = rawState;

            // Toggle only when button is pressed
            if (stableState == LOW) {
                ledState = !ledState;
                digitalWrite(ledPin, ledState);

                Serial.print(ledName);
                Serial.println(ledState ? ": ON" : ": OFF");
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Button configuration
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT);

    // LED configuration
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);

    updateLEDs();

    Serial.println("ASL Challenge 02");
    Serial.println("Smart Home MQTT Controller");
    Serial.println("----------------------------");

    // WiFi connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");
    Serial.println(WiFi.localIP());

    // MQTT configuration
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    Serial.println("System Ready");
}

void loop() {
    // Maintain MQTT connection
    if (WiFi.status() == WL_CONNECTED) {
        reconnectMQTT();
        mqttClient.loop();
    }

    // Read ESP32 GPIO input registers
    uint32_t gpio_low = REG_READ(GPIO_IN_REG);
    uint32_t gpio_high = REG_READ(GPIO_IN1_REG);

    // Extract active-LOW button states
    bool button1State = (gpio_low & (1U << 27)) ? HIGH : LOW;
    bool button2State = (gpio_high & (1U << 2)) ? HIGH : LOW;

    // Handle Button 1 - LED 1
    handleButton(
        button1State,
        lastButton1State,
        stableButton1State,
        lastButton1Change,
        led1State,
        LED_1,
        "LED 1"
    );

    // Handle Button 2 - LED 2
    handleButton(
        button2State,
        lastButton2State,
        stableButton2State,
        lastButton2Change,
        led2State,
        LED_2,
        "LED 2"
    );
}
