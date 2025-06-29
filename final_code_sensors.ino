#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// Replace with your network credentials
const char* ssid = "motorola";
const char* password = "9606256753";

// ThingSpeak API
String apiKey = "RV6VZW3FMDJ8KTJD";
const char* server = "http://api.thingspeak.com/update";

// DHT setup
#define DHTPIN 4        // GPIO pin connected to DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil Moisture Sensor
#define SOIL_PIN 34     // Analog pin for soil moisture

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi.");
}

void loop() {
  // Read sensors
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius
  int rawSoilValue = analogRead(SOIL_PIN);

  // Map raw value (dry=4095, wet=0) to 20–70% range
  float soilMoisturePercent = ((4095.0 - rawSoilValue) / 4095.0) * 50.0 + 20.0;
  soilMoisturePercent = constrain(soilMoisturePercent, 20.0, 70.0);

  // Print values
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("Soil Moisture: "); Serial.print(soilMoisturePercent); Serial.println(" %");

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(temperature, 2) +
                 "&field2=" + String(humidity, 2) +
                 "&field3=" + String(soilMoisturePercent, 2); // 2 decimal places

    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent to ThingSpeak.");
    } else {
      Serial.print("Error sending data. HTTP code: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }

  delay(20000); // ThingSpeak accepts data every 15+ seconds
}