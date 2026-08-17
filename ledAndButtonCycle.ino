/*
--------------------------------------------------
Pyranometer Self-Cleaning Prototype

Cycle Sequence:
1. Start raised at 50
2. Move arm down to 150 (cleaning position)
3. Pulse water pump
4. Run fan for 10 seconds
5. Return arm to 50 (raised)
6. End

LED Indicators:
- Blue   = Idle / ready
- Red    = Cycle in progress (solid)
- Yellow = Blinks during water pulse, solid during fan
- Green  = Cycle complete
--------------------------------------------------*/

#include <Servo.h>

Servo cleaningArm;

// Pin Definitions
const int servoPin   = 9;
const int buttonPin  = 6;
const int waterRelay = 8;  // Relay CH2
const int fanRelay   = 7;  // Relay CH1
const int ledBlue    = 2;
const int ledRed     = 3;
const int ledYellow  = 4;
const int ledGreen   = 5;

bool cycleRunning = false;

// --- LED Helpers ---

void setLeds(bool blue, bool red, bool yellow, bool green)
{
  digitalWrite(ledBlue,   blue   ? HIGH : LOW);
  digitalWrite(ledRed,    red    ? HIGH : LOW);
  digitalWrite(ledYellow, yellow ? HIGH : LOW);
  digitalWrite(ledGreen,  green  ? HIGH : LOW);
}

// --- Arm ---

void moveArmToClean()
{
  Serial.println("[INFO] Moving arm to cleaning position...");
  cleaningArm.write(150);
  delay(2000);
}

void moveArmHome()
{
  Serial.println("[INFO] Returning arm to home position...");
  cleaningArm.write(50);
  delay(2000);
}

// --- Actuators ---

void waterPulse()
{
  Serial.println("[INFO] Water Pulse ON");
  digitalWrite(waterRelay, LOW);

  // Blink yellow for 150ms pulse duration
  unsigned long start = millis();
  while (millis() - start < 250)
  {
    digitalWrite(ledYellow, HIGH);
    delay(50);
    digitalWrite(ledYellow, LOW);
    delay(50);
  }

  digitalWrite(waterRelay, HIGH);
  digitalWrite(ledYellow, LOW);
  Serial.println("[INFO] Water Pulse OFF");
}

void fanOn()
{
  Serial.println("[INFO] Fan ON");
  digitalWrite(fanRelay, LOW);
  digitalWrite(ledYellow, HIGH);  // Solid yellow while fan runs
}

void fanOff()
{
  Serial.println("[INFO] Fan OFF");
  digitalWrite(fanRelay, HIGH);
  digitalWrite(ledYellow, LOW);
}

// --- Cleaning Cycle ---

void runCleaningCycle()
{
  Serial.println();
  Serial.println("=================================");
  Serial.println("[INFO] STARTING CLEANING CYCLE");
  Serial.println("=================================");

  setLeds(false, true, false, false);  // Red ON — cycle in progress

  // Force home before starting
  cleaningArm.write(50);
  delay(2000);

  // Move arm to clean position
  moveArmToClean();
  delay(1000);

  // Tiny water squirt
  waterPulse();
  delay(5000);

  // Dry with fan
  fanOn();
  delay(20000);
  fanOff();
  delay(1000);

  Serial.println("[DEBUG] About to return home...");

  // Return arm
  moveArmHome();

  // Success
  setLeds(false, false, false, true);  // Green ON
  Serial.println("[SUCCESS] Cleaning Cycle Complete");
  delay(3000);

  setLeds(true, false, false, false);  // Back to blue / idle
}

void setup()
{
  Serial.begin(9600);
  Serial.println("[INFO] Initializing System...");

  pinMode(waterRelay, OUTPUT);
  pinMode(fanRelay,   OUTPUT);
  pinMode(buttonPin,  INPUT_PULLUP);
  pinMode(ledBlue,    OUTPUT);
  pinMode(ledRed,     OUTPUT);
  pinMode(ledYellow,  OUTPUT);
  pinMode(ledGreen,   OUTPUT);

  // Relays OFF
  digitalWrite(waterRelay, HIGH);
  digitalWrite(fanRelay,   HIGH);

  // Attach AFTER pinMode, write position immediately
  cleaningArm.attach(servoPin);
  cleaningArm.write(50);
  delay(2000);

  setLeds(true, false, false, false);  // Blue ON — idle
  Serial.println("[INFO] System Ready — Waiting for button press...");
}

void loop()
{
  if (digitalRead(buttonPin) == LOW && !cycleRunning)
  {
    delay(50);  // debounce
    cycleRunning = true;
    runCleaningCycle();
    cycleRunning = false;
    while (digitalRead(buttonPin) == LOW);
  }
}