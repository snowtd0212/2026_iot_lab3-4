#include <Arduino.h>
#include <WiFi.h>

#define PIN_LED_OK 2       // LED xanh
#define PIN_LED_ERR 4      // LED do
#define PIN_BTN_TEST 18    // Nut test ngat WiFi

const char* wifi_ssid = "Wokwi-GUEST";
const char* wifi_pass = "";

const char* target_host = "1.1.1.1";
const uint16_t target_port = 80;

const uint32_t INTERVAL_CHECK = 8000;
const int MAX_FAILS = 3;

int err_count = 0;
uint32_t time_last_check = 0;

bool checkInternet() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NetCheck] No WiFi connection!");
        return false;
    }

    WiFiClient client;
    client.setTimeout(2500);

    uint32_t t_start = millis();
    Serial.printf("[NetCheck] Pinging %s:%d... ", target_host, target_port);

    if (client.connect(target_host, target_port)) {
        uint32_t p_time = millis() - t_start;
        client.stop();
        Serial.printf("SUCCESS (%lu ms)\n", p_time);
        return true;
    } else {
        Serial.println("FAILED!");
        return false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_LED_OK, OUTPUT);
    pinMode(PIN_LED_ERR, OUTPUT);
    pinMode(PIN_BTN_TEST, INPUT_PULLUP);

    digitalWrite(PIN_LED_OK, LOW);
    digitalWrite(PIN_LED_ERR, LOW);

    Serial.println("\n--- WATCHDOG INTERNET SYSTEM ---");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_pass);
    Serial.print("[WiFi] Connecting");

    while (WiFi.status() != WL_CONNECTED) {
        delay(400);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());

    digitalWrite(PIN_LED_OK, HIGH);
}

void checkTestButton() {
    static int prev_state = HIGH;
    int curr_state = digitalRead(PIN_BTN_TEST);
    if (prev_state == HIGH && curr_state == LOW) {
        Serial.println("\n[Test] Button pressed -> Disconnecting WiFi manually!");
        WiFi.disconnect();
        digitalWrite(PIN_LED_OK, LOW);
        digitalWrite(PIN_LED_ERR, HIGH);
    }
    prev_state = curr_state;
}

void loop() {
    checkTestButton();

    uint32_t now = millis();

    if (now - time_last_check >= INTERVAL_CHECK) {
        time_last_check = now;

        Serial.println("\n--- [SYSTEM CHECK] ---");
        bool is_ok = checkInternet();

        if (is_ok) {
            err_count = 0;
            digitalWrite(PIN_LED_OK, HIGH);
            digitalWrite(PIN_LED_ERR, LOW);
            Serial.printf("[Sys] Online. Fails: 0/%d\n", MAX_FAILS);
        } else {
            err_count++;
            digitalWrite(PIN_LED_OK, LOW);
            digitalWrite(PIN_LED_ERR, HIGH);
            Serial.printf("[Warn] Offline! Fails: %d/%d\n", err_count, MAX_FAILS);

            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[Sys] Reconnecting WiFi...");
                WiFi.reconnect();
            }

            if (err_count >= MAX_FAILS) {
                Serial.println("\n!!! CRITICAL FAILURE !!!");
                Serial.println("!!! REBOOTING ESP32 NOW !!!\n");

                for (int i = 0; i < 8; i++) {
                    digitalWrite(PIN_LED_ERR, !digitalRead(PIN_LED_ERR));
                    delay(150);
                }
                ESP.restart();
            }
        }
    }
}
