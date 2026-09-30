import requests
from datetime import datetime

TARGET_HOOK = "https://webhook.site/1f21d33e-6bf5-4ed2-97a7-a08dc767e75a"
# TARGET_HOOK = "https://not-exists-url.local"

payload = {
    "sensor_id": "node_03",
    "time_logged": datetime.now().isoformat(),
    "t": 29.1,
    "h": 61.5,
    "req_timeout": 3
}

print("-" * 50)
print("LAB 4 HW 5 - EXCEPTION HANDLING & TIMEOUTS")
print("-" * 50)

try:
    print(f"\n[Info] Sending POST to {TARGET_HOOK}...")
    
    res = requests.post(
        TARGET_HOOK,
        json=payload,
        timeout=3
    )

    print("\n--- POST RESPONSE ---")
    print(f"HTTP Status : {res.status_code}")
    print(f"HTTP Body   : {res.text}")

    if res.status_code == 200:
        print("\n[Success] Data received by webhook.")
    else:
        print(f"\n[Warn] Server responded with code {res.status_code}")

except requests.exceptions.Timeout:
    print("\n[Error] The request timed out!")
    print("No response from server within 3 seconds.")

except requests.exceptions.ConnectionError:
    print("\n[Error] Connection refused or unreachable!")
    print("Check your internet connection or the URL.")

except requests.exceptions.RequestException as rx:
    print("\n[Error] A request error occurred:")
    print(rx)

except Exception as ex:
    print("\n[Error] An unknown error occurred:")
    print(ex)