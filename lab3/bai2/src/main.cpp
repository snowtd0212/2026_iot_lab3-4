#include <Arduino.h>
#include <WiFi.h>

#define PIN_WIFI_LED 2
#define PIN_USER_BTN 18

const char* ap_ssid = "Wokwi-GUEST";
const char* ap_pwd = "";

bool wifi_connected = false;
uint32_t t_last_reconnect = 0;
const uint32_t T_RECONNECT_DELAY = 4000;
int retry_counter = 0;

uint32_t t_blink = 0;
bool st_led = false;

int st_btn_prev = HIGH;
uint32_t t_debounce = 0;

void onWiFiEvent(WiFiEvent_t evt) {
    switch (evt) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[Event] WiFi Station started.");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.printf("[Event] Connected to %s, waiting for IP...\n", ap_ssid);
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[Event] IP acquired: ");
            Serial.println(WiFi.localIP());
            wifi_connected = true;
            retry_counter = 0;
            digitalWrite(PIN_WIFI_LED, HIGH);
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("[Event] WiFi disconnected!");
            wifi_connected = false;
            digitalWrite(PIN_WIFI_LED, LOW);
            break;
        default: break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_WIFI_LED, OUTPUT);
    pinMode(PIN_USER_BTN, INPUT_PULLUP);
    digitalWrite(PIN_WIFI_LED, LOW);

    Serial.println("\n--- AUTO-RECONNECT & EVENTS ---");

    WiFi.onEvent(onWiFiEvent);
    WiFi.mode(WIFI_STA);

    Serial.printf("[WiFi] Initial connect to %s\n", ap_ssid);
    WiFi.begin(ap_ssid, ap_pwd);
}

void processButton() {
    int val = digitalRead(PIN_USER_BTN);
    if (val != st_btn_prev) {
        t_debounce = millis();
    }

    if ((millis() - t_debounce) > 50) {
        static int st_btn = HIGH;
        if (val != st_btn) {
            st_btn = val;
            if (st_btn == LOW) {
                if (wifi_connected) {
                    Serial.println("\n[Cmd] User disconnect!");
                    WiFi.disconnect();
                } else {
                    Serial.println("\n[Cmd] User reconnect!");
                    WiFi.reconnect();
                }
            }
        }
    }
    st_btn_prev = val;
}

void loop() {
    processButton();

    uint32_t now = millis();

    if (!wifi_connected) {
        if (now - t_blink >= 250) {
            t_blink = now;
            st_led = !st_led;
            digitalWrite(PIN_WIFI_LED, st_led);
        }

        if (now - t_last_reconnect >= T_RECONNECT_DELAY) {
            t_last_reconnect = now;
            retry_counter++;
            Serial.printf("[Auto] Reconnecting (Try #%d)...\n", retry_counter);
            WiFi.reconnect();
        }
    } else {
        digitalWrite(PIN_WIFI_LED, HIGH);

        static uint32_t t_log = 0;
        if (now - t_log >= 8000) {
            t_log = now;
            Serial.printf("[Status] Connection OK. IP: %s\n", WiFi.localIP().toString().c_str());
        }
    }
}
