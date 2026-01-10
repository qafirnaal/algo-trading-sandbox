from flask import Flask, request, jsonify
from flask_cors import CORS
import json
import subprocess

app = Flask(__name__)
CORS(app)

@app.route("/")
def home():
    return "Hello, server is running!"

@app.route("/simulate", methods=["POST"])
def simulate():
    if not request.is_json:
        return jsonify({"error": "Invalid JSON"}), 400

    data = request.json

    # Write input.json
    with open("input.json", "w") as f:
        json.dump(data, f)

    # Run engine
    completed = subprocess.run(
        ["./Engine/engine.exe", "input.json", "output.json"],
        check=True,
        capture_output=True,
        text=True
    )

    engine_output = completed.stdout.strip()
    print("ENGINE STDOUT:", repr(engine_output))

    # Write raw JSON (NOT json.dump)
    with open("output.json", "w") as f:
        f.write(engine_output)

    
    parsed = json.loads(engine_output)

    # Load JSON properly
    with open("output.json", "w") as f:
        result = json.dump(parsed,f,indent=2)

    print("RETURNING TO FRONTEND:", parsed)

    return jsonify(parsed)

if __name__ == "__main__":  
    app.run(port=8000)
