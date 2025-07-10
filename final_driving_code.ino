#include <Servo.h>
#include <SoftwareSerial.h>

Servo seedServo;
SoftwareSerial BT(10, 11);  // RX, TX for Bluetooth module

// Motor Pins
const int MR1 = 5;
const int MR2 = 6;
const int ML1 = 7;
const int ML2 = 8;

// Servo Pin
const int servoPin = 9;

// Constants
const float wheelCircumference = 18.85; // cm per rotation (2πr, r=3cm)
const unsigned long rotationInterval = 1000; // simulate 1 rotation every 1s

// Placement distance (cm) for each crop type
int getPlacementDistance(char crop) {
  switch (crop) {
    case 'W': return 20;
    case 'X': return 30;
    case 'Y': return 40;
    case 'Z': return 50;
    default: return 0;
  }
}

// State Variables
char currentCommand = 'S';
int placementDistance = 0;
float distanceSinceLastPlant = 0;
bool plantingMode = false;

unsigned long lastRotationTime = 0;

void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  pinMode(MR1, OUTPUT); 
  pinMode(MR2, OUTPUT);
  pinMode(ML1, OUTPUT);
  pinMode(ML2, OUTPUT);

  seedServo.attach(servoPin);
  seedServo.write(0);  // initial position
}

void loop() {
  if (BT.available()) {
    char cmd = BT.read();
    Serial.print("Received: ");
    Serial.println(cmd);

    if (cmd == 'F' || cmd == 'B' || cmd == 'L' || cmd == 'R' || cmd == 'S') {
      currentCommand = cmd;
      plantingMode = false;
      distanceSinceLastPlant = 0;
    } else if (cmd == 'W' || cmd == 'X' || cmd == 'Y' || cmd == 'Z') {
      currentCommand = 'F';
      plantingMode = true;
      placementDistance = getPlacementDistance(cmd);
      distanceSinceLastPlant = 0;
    }
  }

  executeCommand(currentCommand);

  if (plantingMode) {
    if (millis() - lastRotationTime >= rotationInterval) {
      lastRotationTime = millis();

      distanceSinceLastPlant += wheelCircumference;
      Serial.print("Distance since last plant: ");
      Serial.println(.distanceSinceLastPlant);

      if (distanceSinceLastPlant >= placementDistance) {
        plantSeed();
        distanceSinceLastPlant = 0;
      }
    }
  }
}

void executeCommand(char cmd) {
  switch (cmd) {
    case 'F': Forward(); break;
    case 'B': Backward(); break;
    case 'L': Left(); break;
    case 'R': Right(); break;
    case 'S': Stop(); break;
  }
}

void plantSeed() {
  Stop();
  Serial.println("Planting seed...");
  seedServo.write(90);   // rotate to place position
  delay(1000);
  seedServo.write(0);    // return to original
  delay(500);
  Forward();             // resume motion
}

void Forward() {
  digitalWrite(MR1, HIGH);
  digitalWrite(MR2, LOW);
  digitalWrite(ML1, LOW);
  digitalWrite(ML2, HIGH);
}

void Backward() {
  digitalWrite(MR1, LOW);
  digitalWrite(MR2, HIGH);
  digitalWrite(ML1, HIGH);
  digitalWrite(ML2, LOW);
}

void Left() {
  digitalWrite(MR1, HIGH);
  digitalWrite(MR2, LOW);
  digitalWrite(ML1, HIGH);
  digitalWrite(ML2, LOW);
}

void Right() {
  digitalWrite(MR1, LOW);
  digitalWrite(MR2, HIGH);
  digitalWrite(ML1, LOW);
  digitalWrite(ML2, HIGH);
}

void Stop() {
  digitalWrite(MR1, LOW);
  digitalWrite(MR2, LOW);
  digitalWrite(ML1, LOW);
  digitalWrite(ML2, LOW);
}