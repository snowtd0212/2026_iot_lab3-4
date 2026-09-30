#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

const char* my_wifi_ssid = "Wokwi-GUEST";
const char* my_wifi_pass = "";

const char* ntp_server_primary = "pool.ntp.org";
const char* ntp_server_secondary = "time.google.com";
const long  tz_offset = 7 * 3600;
const int   dst_offset = 0;

const char* str_dow[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

void showTime() {
    struct tm ti;
    if (!getLocalTime(&ti)) {
        Serial.println("[Time] Waiting for NTP sync...");
        return;
    }

    time_t epoch_now;
    time(&epoch_now);

    char buf[128];
    snprintf(buf, sizeof(buf), 
             "[%04d-%02d-%02d %02d:%02d:%02d] %-3s | Epoch: %ld",
             ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
             ti.tm_hour, ti.tm_min, ti.tm_sec,
             str_dow[ti.tm_wday],
             (long)epoch_now);

    Serial.println(buf);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n--- NTP SYNC (UTC+7) ---");

    WiFi.mode(WIFI_STA);
    WiFi.begin(my_wifi_ssid, my_wifi_pass);
    Serial.print("[WiFi] Connecting...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(400);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());

    Serial.println("[NTP] Configuring time servers...");
    configTime(tz_offset, dst_offset, ntp_server_primary, ntp_server_secondary);

    struct tm ti;
    while (!getLocalTime(&ti)) {
        Serial.print("*");
        delay(400);
    }
    Serial.println("\n[NTP] Sync successful!");
    Serial.println("------------------------");
}

void loop() {
    static uint32_t t_last = 0;
    if (millis() - t_last >= 1000) {
        t_last = millis();
        showTime();
    }
}
