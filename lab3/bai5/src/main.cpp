#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

#define O_W 128
#define O_H 64
#define O_RST -1
#define O_ADDR 0x3C

Adafruit_SSD1306 oled(O_W, O_H, &Wire, O_RST);

const char* ap_ssid = "Wokwi-GUEST";
const char* ap_pwd = "";

const char* s_ntp = "pool.ntp.org";
const long  off_gmt = 7 * 3600;
const int   off_dst = 0;

const char* day_names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void refreshOLED(struct tm &ti, bool is_connected) {
    oled.clearDisplay();

    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 2);
    oled.printf("%s, %04d-%02d-%02d", 
                day_names[ti.tm_wday],
                ti.tm_year + 1900, 
                ti.tm_mon + 1, 
                ti.tm_mday);

    oled.drawLine(0, 14, 127, 14, SSD1306_WHITE);

    oled.setTextSize(2);
    oled.setCursor(16, 24);
    oled.printf("%02d:%02d:%02d", ti.tm_hour, ti.tm_min, ti.tm_sec);

    oled.drawLine(0, 48, 127, 48, SSD1306_WHITE);

    oled.setTextSize(1);
    oled.setCursor(0, 54);
    if (is_connected) {
        oled.printf("Net: OK | %s", WiFi.localIP().toString().c_str());
    } else {
        oled.print("Net: OFFLINE");
    }

    oled.display();
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n--- OLED NTP CLOCK ---");

    Wire.begin(21, 22);
    if (!oled.begin(SSD1306_SWITCHCAPVCC, O_ADDR)) {
        Serial.println(F("[Err] OLED init failed!"));
        while (true);
    }

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(5, 20);
    oled.println("OLED NTP CLOCK");
    oled.setCursor(5, 40);
    oled.println("Connecting...");
    oled.display();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ap_ssid, ap_pwd);
    Serial.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(400);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

    configTime(off_gmt, off_dst, s_ntp);
    Serial.println("Syncing NTP...");

    struct tm ti;
    while (!getLocalTime(&ti)) {
        delay(400);
        Serial.print(".");
    }
    Serial.println("\nNTP Synced!");
}

void loop() {
    static uint32_t t_last = 0;
    if (millis() - t_last >= 1000) {
        t_last = millis();

        struct tm ti;
        bool t_ok = getLocalTime(&ti);
        bool net_ok = (WiFi.status() == WL_CONNECTED);

        if (t_ok) {
            refreshOLED(ti, net_ok);
            Serial.printf("[%02d:%02d:%02d] Net: %s\n",
                          ti.tm_hour, ti.tm_min, ti.tm_sec,
                          net_ok ? "ON" : "OFF");
        }
    }
}
