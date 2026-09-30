#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <esp_sntp.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define LED_ONLINE_PIN 2   // LED xanh: Trang thai Online / Sync tot
#define LED_ALERT_PIN 4    // LED do: Canh bao Mat ket noi
#define BTN_TOGGLE_PIN 18  // Nut nhan: Gia lap ngat / bat lai WiFi

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;   // UTC+7 (Viet Nam)
const int   daylightOffset_sec = 0;

const char* daysOfWeek[] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};

// Biến quản lý trạng thái
bool wasConnected = false;
bool ntpSynced = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // Thu ket noi lai moi 5s

// Button debounce
int lastBtnState = HIGH;
unsigned long lastDebounceTime = 0;

void syncNtpTime() {
    Serial.println("[NTP] Bat dau dong bo lai gio tu NTP Server...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 2000)) {
        ntpSynced = true;
        Serial.println("[NTP] => Dong bo gio thanh cong!");
    } else {
        Serial.println("[NTP] => Dong bo NTP chua phan hoi (se tiep tuc chay RTC noi).");
    }
}

void renderClock(struct tm &timeinfo, bool isOnline) {
    display.clearDisplay();

    // 1. Header Bar: Thứ, Ngày/Tháng/Năm + Icon mạng
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.printf("%s %02d/%02d/%04d", 
                   daysOfWeek[timeinfo.tm_wday],
                   timeinfo.tm_mday, 
                   timeinfo.tm_mon + 1, 
                   timeinfo.tm_year + 1900);

    // Biểu tượng trạng thái góc phải
    display.setCursor(104, 0);
    if (isOnline) {
        display.print("[ON]");
    } else {
        display.print("[!X]");
    }

    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

    // 2. Giờ:Phút:Giây (Font to - Size 2)
    display.setTextSize(2);
    display.setCursor(16, 18);
    display.printf("%02d:%02d:%02d", 
                   timeinfo.tm_hour, 
                   timeinfo.tm_min, 
                   timeinfo.tm_sec);

    display.drawLine(0, 39, 127, 39, SSD1306_WHITE);

    // 3. Status Bar: Thông tin mạng và chế độ đồng hồ
    display.setTextSize(1);
    display.setCursor(0, 43);
    if (isOnline) {
        display.printf("WiFi: OK (%d dBm)", WiFi.RSSI());
        display.setCursor(0, 53);
        display.printf("Mode: NTP SYNCED");
    } else {
        display.print("WiFi: DISCONNECTED!");
        display.setCursor(0, 53);
        display.print("Mode: RTC FREE-RUN");
    }

    display.display();
}

void handleButtonToggle() {
    int reading = digitalRead(BTN_TOGGLE_PIN);
    if (reading != lastBtnState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > 50) {
        static int btnState = HIGH;
        if (reading != btnState) {
            btnState = reading;
            if (btnState == LOW) { // Nhấn nút
                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("\r\n[Action] Nguoi dung bam nut => NGAT WIFI!");
                    WiFi.disconnect();
                } else {
                    Serial.println("\r\n[Action] Nguoi dung bam nut => KET NOI LAI WIFI!");
                    WiFi.reconnect();
                }
            }
        }
    }
    lastBtnState = reading;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_ONLINE_PIN, OUTPUT);
    pinMode(LED_ALERT_PIN, OUTPUT);
    pinMode(BTN_TOGGLE_PIN, INPUT_PULLUP);

    digitalWrite(LED_ONLINE_PIN, LOW);
    digitalWrite(LED_ALERT_PIN, LOW);

    Serial.println("\r\n============================================================");
    Serial.println("  LAB 3 - BAI 3_VN (BAI 8): DONG HO DE BAN THONG MINH CHIU LOI");
    Serial.println("============================================================");
    Serial.println("Mo ta: Mat WiFi van tiep tuc chay gio qua RTC noi, co mang tu dong resync.");
    Serial.println("Test: Nhan nut GPIO 18 de chu dong bat / tat WiFi.");

    // Khởi tạo OLED
    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("[OLED] Khoi tao OLED that bai!"));
        while (true);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("SMART DESK CLOCK");
    display.setCursor(10, 40);
    display.println("Connecting WiFi...");
    display.display();

    // Kết nối WiFi ban đầu
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("[WiFi] Dang ket noi toi ");
    Serial.print(ssid);

    unsigned long startWait = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startWait < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\r\n[WiFi] => Da ket noi WiFi!");
        wasConnected = true;
        digitalWrite(LED_ONLINE_PIN, HIGH);
        digitalWrite(LED_ALERT_PIN, LOW);
        syncNtpTime();
    } else {
        Serial.println("\r\n[WiFi] => Khong ket noi duoc WiFi! Chay o che do Offline.");
        digitalWrite(LED_ONLINE_PIN, LOW);
        digitalWrite(LED_ALERT_PIN, HIGH);
    }
}

void loop() {
    handleButtonToggle();

    unsigned long currentMillis = millis();
    bool isOnline = (WiFi.status() == WL_CONNECTED);

    // Xử lý chuyển đổi trạng thái kết nối mạng
    if (isOnline && !wasConnected) {
        // Vừa mới có mạng trở lại
        Serial.println("\r\n[Network Event] => DA KHOI PHUC KET NOI WIFI!");
        Serial.printf("[Network] IP: %s\r\n", WiFi.localIP().toString().c_str());
        wasConnected = true;
        digitalWrite(LED_ONLINE_PIN, HIGH);
        digitalWrite(LED_ALERT_PIN, LOW);
        // Tự động đồng bộ lại giờ NTP
        syncNtpTime();
    } 
    else if (!isOnline && wasConnected) {
        // Vừa bị mất mạng
        Serial.println("\r\n[Network Event] => MAT KET NOI WIFI! Chuyen sang bo dem thoi gian RTC noi...");
        wasConnected = false;
        digitalWrite(LED_ONLINE_PIN, LOW);
        digitalWrite(LED_ALERT_PIN, HIGH);
    }

    // Xử lý tự động kết nối lại ngầm (Non-blocking) khi đang mất mạng
    if (!isOnline) {
        // Nhấp nháy LED cảnh báo đỏ (chu kỳ 500ms)
        static unsigned long lastAlertBlink = 0;
        if (currentMillis - lastAlertBlink >= 500) {
            lastAlertBlink = currentMillis;
            digitalWrite(LED_ALERT_PIN, !digitalRead(LED_ALERT_PIN));
        }

        // Thử reconnect định kỳ mà không block màn hình
        if (currentMillis - lastReconnectAttempt >= reconnectInterval) {
            lastReconnectAttempt = currentMillis;
            Serial.println("[Background Reconnect] Dang thu ket noi lai WiFi...");
            WiFi.reconnect();
        }
    }

    // Cập nhật hiển thị đồng hồ mỗi 1 giây
    static unsigned long lastClockTick = 0;
    if (currentMillis - lastClockTick >= 1000) {
        lastClockTick = currentMillis;

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            renderClock(timeinfo, isOnline);
            Serial.printf("[%04d-%02d-%02d %02d:%02d:%02d] Trang thai: %s\r\n",
                          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                          isOnline ? "ONLINE (NTP)" : "OFFLINE (RTC)");
        }
    }
}
