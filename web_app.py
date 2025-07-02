from flask import Flask, render_template_string
import requests
import joblib
import numpy as np

app = Flask(__name__)

CHANNEL_ID = "2970238"
READ_API_KEY = "C9AXAOBNNQ5P3V5P"

# Load model and label encoder
model = joblib.load("crop_model.pkl")
label_encoder = joblib.load("label_encoder.pkl")

@app.route('/')
def show_prediction():
    try:
        # Fetch latest sensor data
        url = f"https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds/last.json?api_key={READ_API_KEY}"
        response = requests.get(url).json()

        # Extract fields
        field1 = response.get('field1')
        field2 = response.get('field2')
        field3 = response.get('field3')

        # Check if data is available
        if not field1 or not field2 or not field3:
            raise ValueError("No data provided")

        temperature = float(field1)
        humidity = float(field2)
        moisture = float(field3)

        # Get top 3 crop predictions
        probs = model.predict_proba([[temperature, humidity, moisture]])[0]
        top_indices = np.argsort(probs)[::-1][:3]
        top_crops = label_encoder.inverse_transform(top_indices)

        # Generate HTML list of predictions
        predicted_crop_html = "<ol>"
        for crop in top_crops:
            predicted_crop_html += f"<li>{crop}</li>"
        predicted_crop_html += "</ol>"

    except Exception as e:
        predicted_crop_html = f"<p style='color:red;'>Error: {e}</p>"
        temperature = humidity = moisture = "No data provided"

    # HTML template
    html = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Crop Prediction</title>
        <meta http-equiv="refresh" content="10">
        <style>
            body {{
                margin: 0;
                padding: 0;
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                background-image: url('https://www.niti.gov.in/sites/default/files/2022-11/agriculture-development.jpeg');
                background-size: cover;
                background-position: center;
                color: white;
            }}
            .overlay {{
                background-color: rgba(0, 0, 0, 0.6);
                padding: 50px;
                max-width: 600px;
                margin: 100px auto;
                border-radius: 15px;
                box-shadow: 0 4px 15px rgba(0,0,0,0.4);
            }}
            h1 {{
                font-size: 3em;
                margin-bottom: 10px;
                color: #90ee90;
            }}
            ol {{
                font-size: 1.8em;
                padding-left: 20px;
            }}
            .params {{
                margin-top: 30px;
                font-size: 1.2em;
            }}
            .params div {{
                margin-bottom: 10px;
            }}
            .footer {{
                margin-top: 30px;
                font-size: 0.9em;
                color: #ccc;
            }}
        </style>
    </head>
    <body>
        <div class="overlay">
            <h1>Top Predicted Crops</h1>
            {predicted_crop_html}
            <div class="params">
                <div><strong>Temperature:</strong> {temperature} °C</div>
                <div><strong>Humidity:</strong> {humidity} %</div>
                <div><strong>Moisture:</strong> {moisture}</div>
            </div>
            <div class="footer">Auto-refreshes every 10 seconds</div>
        </div>
    </body>
    </html>
    """
    return render_template_string(html)

if __name__ == '__main__':
    app.run(debug=True)
