from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route("/api/v1/sensor", methods=["POST"])
def handle_sensor_post():
    payload = request.get_json()

    print("-" * 50)
    print(">>> SENSOR DATA RECEIVED <<<")
    print("-" * 50)

    print(f"Device   : {payload.get('device_id')}")
    print(f"Temp     : {payload.get('temperature')}")
    print(f"Humidity : {payload.get('humidity')}")
    print(f"Time     : {payload.get('timestamp')}")

    print("\nRaw Payload:")
    print(payload)

    return jsonify({
        "status": "ok",
        "info": "Data processed"
    }), 200

if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )