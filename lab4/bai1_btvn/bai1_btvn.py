import requests
import random
import time
from datetime import datetime

WEBHOOK = "https://webhook.site/1f21d33e-6bf5-4ed2-97a7-a08dc767e75a"

print("-" * 50)
print("LAB 4 - HW 1: SENSOR DATA PUBLISHER")
print("-" * 50)
print("Interval: 10s")
print("Press Ctrl+C to stop.")
print()

try:
    while True:
        t_val = round(random.uniform(22.0, 38.0), 1)
        h_val = round(random.uniform(45.0, 85.0), 1)
        ts_now = datetime.now().isoformat()

        payload = {
            "sensor_id": "env_sensor_test_1",
            "time_recorded": ts_now,
            "temp_c": t_val,
            "hum_pct": h_val
        }

        try:
            res = requests.post(
                WEBHOOK,
                json=payload,
                timeout=4
            )

            print("~" * 50)

            if res.status_code == 200:
                print("[OK] Data sent successfully!")
            else:
                print(f"[WARN] Server returned: {res.status_code}")

            print(f"Time: {ts_now}")
            print(f"Temp: {t_val} C")
            print(f"Hum : {h_val} %")
            print(f"Code: {res.status_code}")

        except requests.exceptions.Timeout:
            print("[ERR] Request timeout!")
        except requests.exceptions.ConnectionError:
            print("[ERR] Connection failed!")
        except Exception as e:
            print(f"[ERR] Unexpected error: {e}")

        time.sleep(10)

except KeyboardInterrupt:
    print("\n" + "=" * 50)
    print("Program terminated by user.")
    print("=" * 50)