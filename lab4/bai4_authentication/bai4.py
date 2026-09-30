import requests
from datetime import datetime

HOOK_URL = "https://webhook.site/1f21d33e-6bf5-4ed2-97a7-a08dc767e75a"

MY_API_KEY = "sk_test_88889999"
MY_BEARER = "tok_abc_xyz_123"

req_headers = {
    "Content-Type": "application/json",
    "X-API-Key": MY_API_KEY,
    "Authorization": f"Bearer {MY_BEARER}"
}

payload = {
    "node_id": "station_02",
    "recorded_at": datetime.now().isoformat(),
    "env": {
        "temp_c": 31.4,
        "hum_pct": 69.2,
        "light_lux": 550,
    },
    "sys": {
        "batt_pct": 82.5,
    },
    "gps": {
        "lat": 10.762,
        "lng": 106.660,
        "city": "HCMC"
    }
}

print("-" * 50)
print("LAB 4 HW 4: AUTHENTICATED REQUEST")
print("-" * 50)

print(f"\nTarget: {HOOK_URL}")

print("\n[ Headers ]")
for k, v in req_headers.items():
    print(f" > {k}: {v}")

print("\n[ Payload ]")
print(payload)

try:
    res = requests.post(
        HOOK_URL,
        headers=req_headers,
        json=payload,
        timeout=5
    )

    print("\n--- RESULT ---")
    print(f"Code: {res.status_code}")
    print(f"Text: {res.text}")

    if res.status_code == 200:
        print("\n[+] Sent data successfully!")
    else:
        print(f"\n[-] Failed with status: {res.status_code}")

except requests.exceptions.RequestException as ex:
    print(f"\n[!] Network Error: {ex}")