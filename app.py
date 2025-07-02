
import joblib
import requests
import json

# Load model and label encoder
model = joblib.load("crop_model.pkl")
label_encoder = joblib.load("label_encoder.pkl")

# ThingSpeak sensor channel details
READ_API_KEY = "C9AXAOBNNQ5P3V5P"
CHANNEL_ID = "2970238"  # Replace with your sensor channel ID

# Get latest sensor values
url = f"https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds/last.json?api_key={READ_API_KEY}"
response = requests.get(url)
data = response.json()

# Extract sensor readings
temperature= float(data['field1'])
humidity= float(data['field2'])
moisture= float(data['field3'])

# Make prediction
prediction = model.predict([[temperature, humidity, moisture]])
predicted_crop = label_encoder.inverse_transform(prediction)[0]

# Save prediction locally to a JSON file
with open("latest_prediction.json", "w") as f:
    json.dump({"predicted_crop": predicted_crop}, f)

print(f"Predicted crop saved: {predicted_crop}")
