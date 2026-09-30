import requests

url = "https://webhook.site/1f21d33e-6bf5-4ed2-97a7-a08dc767e75a"

data = {
    "device_id": "sensor_01",
    "temperature": 25.5,
    "humidity": 65
}

response = requests.post(url, json=data)

print("Status code:", response.status_code)
print("Response:", response.text)