#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#define LED_INDICATOR 2

const char* my_ssid = "Wokwi-GUEST";
const char* my_pass = "";

const char* ntp_sv1 = "pool.ntp.org";
const char* ntp_sv2 = "time.google.com";
const long  gmt_offset = 7 * 3600;
const int   dl_offset = 0;

const uint32_t INTERVAL_NTP = 6UL * 60 * 60 * 1000;

uint32_t t_last_ntp = 0;
int counter_sync = 0;

void format_time(time_t t, char* buf, size_t max_size) {
    struct tm* t_info = localtime(&t);
    strftime(buf, max_size, "%Y-%m-%d %H:%M:%S", t_info);
}

void trigger_ntp_sync() {
    counter_sync++;
    digitalWrite(LED_INDICATOR, HIGH);

    Serial.println("\n--- NTP SYNC STARTED ---");
    Serial.printf("[Info] Sync iteration #%d\n", counter_sync);

    time_t t_before;
    time(&t_before);
    char buf_before[32];
    format_time(t_before, buf_before, sizeof(buf_before));

    configTime(gmt_offset, dl_offset, ntp_sv1, ntp_sv2);

    struct tm ti;
    uint32_t wait_start = millis();
    bool is_success = false;

    while (millis() - wait_start < 8000) {
        if (getLocalTime(&ti, 500)) {
            is_success = true;
            break;
        }
        delay(250);
    }

    time_t t_after;
    time(&t_after);
    char buf_after[32];
    format_time(t_after, buf_after, sizeof(buf_after));

    double delta = difftime(t_after, t_before);

    if (is_success) {
        Serial.println("[Result] SUCCESS");
        Serial.printf("  Local (RTC)  : %s\n", buf_before);
        Serial.printf("  NTP (Server) : %s\n", buf_after);
        Serial.printf("  Drift        : %.2f sec\n", delta);
    } else {
        Serial.println("[Result] FAILED (Timeout)");
    }
    Serial.println("------------------------\n");

    digitalWrite(LED_INDICATOR, LOW);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(LED_INDICATOR, OUTPUT);
    digitalWrite(LED_INDICATOR, LOW);

    Serial.println("\n--- NTP SYNC DEMO ---");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(my_ssid, my_pass);
    Serial.print("[WiFi] Connecting...");

    while (WiFi.status() != WL_CONNECTED) {
        delay(350);
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected!");

    trigger_ntp_sync();
    t_last_ntp = millis();
}

void loop() {
    uint32_t now = millis();

    static uint32_t t_tick = 0;
    if (now - t_tick >= 1000) {
        t_tick = now;

        struct tm ti;
        if (getLocalTime(&ti)) {
            Serial.printf("[Clock] %04d-%02d-%02d %02d:%02d:%02d | Next sync: %lus\n",
                          ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
                          ti.tm_hour, ti.tm_min, ti.tm_sec,
                          (INTERVAL_NTP - (now - t_last_ntp)) / 1000);
        }
    }

    if (now - t_last_ntp >= INTERVAL_NTP) {
        t_last_ntp = now;
        trigger_ntp_sync();
    }
}
