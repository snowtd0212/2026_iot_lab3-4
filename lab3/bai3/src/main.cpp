#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#define PIN_LED 2
#define PIN_RESET_BTN 18

WiFiManager wifi_mgr;
uint32_t t_press_start = 0;
bool st_pressed = false;

void onConfigMode(WiFiManager *mgr) {
    Serial.println("\n[Portal] ENTERING CAPTIVE PORTAL MODE");
    Serial.printf("[Portal] AP SSID: %s\n", mgr->getConfigPortalSSID().c_str());
    Serial.printf("[Portal] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.println("[Portal] Please connect to this AP and go to http://192.168.4.1");
    Serial.println("=========================================\n");

    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        delay(100);
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_RESET_BTN, INPUT_PULLUP);
    digitalWrite(PIN_LED, LOW);

    Serial.println("\n--- WIFIMANAGER DEMO ---");
    Serial.println("Hold GPIO 18 for 3s to reset.");

    wifi_mgr.setAPCallback(onConfigMode);
    wifi_mgr.setConfigPortalTimeout(120);

    Serial.println("[Mgr] AutoConnect starting...");
    bool connected = wifi_mgr.autoConnect("IoT_Config_AP");

    if (!connected) {
        Serial.println("[Mgr] Failed to connect or timeout!");
        digitalWrite(PIN_LED, LOW);
    } else {
        Serial.println("\n[Mgr] CONNECTED SUCCESSFULLY!");
        Serial.printf("[Mgr] SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("[Mgr] IP  : %s\n", WiFi.localIP().toString().c_str());
        digitalWrite(PIN_LED, HIGH);
    }
}

void processReset() {
    if (digitalRead(PIN_RESET_BTN) == LOW) {
        if (!st_pressed) {
            st_pressed = true;
            t_press_start = millis();
        } else {
            if (millis() - t_press_start >= 3000) {
                Serial.println("\n[Sys] Factory reset triggered!");
                digitalWrite(PIN_LED, LOW);
                wifi_mgr.resetSettings();
                delay(500);
                ESP.restart();
            }
        }
    } else {
        st_pressed = false;
    }
}

void loop() {
    processReset();

    static uint32_t t_last = 0;
    if (millis() - t_last >= 8000) {
        t_last = millis();
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[Status] Connected | SSID: %s | IP: %s\n", 
                          WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
        } else {
            Serial.println("[Status] Offline!");
        }
    }
}
