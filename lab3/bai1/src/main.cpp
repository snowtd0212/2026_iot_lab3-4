#include <Arduino.h>
#include <WiFi.h>

// Thay doi ten wifi
const char* wifi_ssid = "Wokwi-GUEST";
const char* wifi_password = "";

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n--- KET NOI WIFI (STA) ---");

    // 1. Cau hinh WiFi
    WiFi.mode(WIFI_STA);
    Serial.println("[Info] Set mode Station...");

    // 2. Bat dau ket noi
    Serial.printf("[Info] Connecting to %s\n", wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);

    // 3. Cho doi ket noi, timeout 15s
    uint32_t t_start = millis();
    const uint32_t T_OUT = 15000;

    while (WiFi.status() != WL_CONNECTED && (millis() - t_start) < T_OUT) {
        delay(400);
        Serial.print("*");
    }
    Serial.println();

    // 4. In ket qua
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[Success] WiFi connected!");
        Serial.print("IP: "); Serial.println(WiFi.localIP());
        Serial.print("Subnet: "); Serial.println(WiFi.subnetMask());
        Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
        Serial.print("DNS: "); Serial.println(WiFi.dnsIP());
        Serial.print("MAC: "); Serial.println(WiFi.macAddress());
        Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    } else {
        Serial.println("[Error] WiFi timeout.");
    }
}

void loop() {
    // Kiem tra dinh ky 4 giay
    static uint32_t timer_check = 0;
    if (millis() - timer_check >= 4000) {
        timer_check = millis();
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[Alive] IP: %s | RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
        } else {
            Serial.println("[Alert] Disconnected!");
        }
    }
}
