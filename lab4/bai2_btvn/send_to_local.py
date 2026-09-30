import requests
from datetime import datetime

TARGET_URL = "http://127.0.0.1:5000/api/v1/sensor"

payload = {
    "device_id": "NODE_TEST_01",
    "timestamp": datetime.now().isoformat(),
    "temperature": 27.2,
    "humidity": 65.4,
    "light_lux": 480,
    "batt_pct": 92
}

print("-" * 50)
print("LAB 4 HW 2 - LOCAL POST TEST")
print("-" * 50)

print(f"\nSending to: {TARGET_URL}")
print(f"Payload: {payload}")

try:
    res = requests.post(
        TARGET_URL,
        json=payload,
        timeout=4
    )

    print("\n--- RESPONSE ---")
    print(f"Code: {res.status_code}")
    print(f"Body: {res.text}")

    if res.status_code == 200:
        print("\n[OK] Success!")
    else:
        print("\n[WARN] Failed!")

except requests.exceptions.ConnectionError:
    print("\n[ERR] Connection refused! Is server running?")
except requests.exceptions.Timeout:
    print("\n[ERR] Timeout!")
except Exception as e:
    print(f"\n[ERR] Unexpected error: {e}")