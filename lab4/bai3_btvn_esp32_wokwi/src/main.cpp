#include <Arduino.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#define DHT_DATA_PIN 15
#define SENSOR_TYPE DHT22

const char *ap_name = "Wokwi-GUEST";
const char *ap_pass = "";

const char *hook_url =
    "https://webhook.site/1f21d33e-6bf5-4ed2-97a7-a08dc767e75a";

const uint32_t INTERVAL_MS = 10000;

DHT my_dht(DHT_DATA_PIN, SENSOR_TYPE);
uint32_t t_last_pub = 0;
int tx_counter = 0;

void setupWifi() {
  Serial.println("\n--- WiFi Initialization ---");
  Serial.printf("Connecting to: %s\n", ap_name);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ap_name, ap_pass);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print("*");
    tries++;
    if (tries > 25) {
      Serial.println("\n[Error] WiFi connection failed!");
      return;
    }
  }

  Serial.println("\n[Success] Connected!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void publishData(float t, float h) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Warn] No WiFi. Reconnecting...");
    setupWifi();
    return;
  }

  WiFiClientSecure sec_client;
  sec_client.setInsecure();

  HTTPClient http_cli;
  http_cli.begin(sec_client, String(hook_url));
  http_cli.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"node_id\":\"WOKWI_NODE_01\",";
  payload += "\"temp_c\":" + String(t, 2) + ",";
  payload += "\"hum_pct\":" + String(h, 2) + ",";
  payload += "\"pkt_num\":" + String(tx_counter) + ",";
  payload += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  payload += "\"uptime\":" + String(millis());
  payload += "}";

  Serial.println("[HTTP] Sending POST...");
  Serial.println("Payload: " + payload);

  int status_code = http_cli.POST(payload);

  if (status_code > 0) {
    String res_body = http_cli.getString();
    Serial.printf("[HTTP] OK. Code: %d\n", status_code);
    if (res_body.length() > 200) {
      Serial.println(res_body.substring(0, 200));
    } else {
        Serial.println(res_body);
    }
  } else {
    Serial.printf("[HTTP] Failed! Code: %d\n", status_code);
  }

  http_cli.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n*** DHT22 HTTP POST SENDER ***");

  my_dht.begin();
  Serial.printf("Sensor initialized on pin %d\n", DHT_DATA_PIN);

  setupWifi();
}

void loop() {
  uint32_t now = millis();

  if (now - t_last_pub >= INTERVAL_MS) {
    t_last_pub = now;
    tx_counter++;

    float h_val = my_dht.readHumidity();
    float t_val = my_dht.readTemperature();

    Serial.println("\n----------------------------------");
    Serial.printf("Reading #%d | Uptime: %ds\n", tx_counter, now / 1000);

    if (isnan(h_val) || isnan(t_val)) {
      Serial.println("[Error] Failed to read from DHT22!");
      return;
    }

    Serial.printf("Temperature: %.2f C\n", t_val);
    Serial.printf("Humidity   : %.2f %%\n", h_val);

    publishData(t_val, h_val);
  }
}
