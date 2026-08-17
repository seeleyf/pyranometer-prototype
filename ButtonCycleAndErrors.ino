/*
--------------------------------------------------
Pyranometer Self-Cleaning Prototype

ONLY DETECTS ERROR IF ARDUINO PIN PULLED


Cycle Sequence:
1. Start raised at 50
2. Move arm down to 150 (cleaning position)
3. Pulse water pump (2 seconds)
4. Soak for 5 seconds
5. Run fan for 20 seconds
6. Return arm to 50 (raised)
7. End

Triggers:
- Manual button press

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

// --- Error Handler ---

void reportError(const char* step)
{
  Serial.println();
  Serial.println("=================================");
  Serial.print("[ERROR] CYCLE FAILED AT STEP: ");
  Serial.println(step);
  Serial.println("=================================");

  // Flash red rapidly to signal fault
  for (int i = 0; i < 10; i++)
  {
    digitalWrite(ledRed, HIGH);
    delay(100);
    digitalWrite(ledRed, LOW);
    delay(100);
  }

  setLeds(true, false, false, false);  // Return to idle blue
}

// --- Arm ---

bool moveArmToClean()
{
  Serial.println("[INFO] Moving arm to cleaning position...");
  cleaningArm.write(150);
  delay(2000);

  if (cleaningArm.read() != 150)
  {
    return false;
  }
  return true;
}

bool moveArmHome()
{
  Serial.println("[INFO] Returning arm to home position...");
  cleaningArm.write(50);
  delay(2000);

  if (cleaningArm.read() != 50)
  {
    return false;
  }
  return true;
}

// --- Actuators ---

bool waterPulse()
{
  Serial.println("[INFO] Water Pulse ON");

  if (digitalRead(waterRelay) != HIGH)
  {
    return false;
  }

  digitalWrite(waterRelay, LOW);

  unsigned long start = millis();
  while (millis() - start < 2000)
  {
    digitalWrite(ledYellow, HIGH);
    delay(50);
    digitalWrite(ledYellow, LOW);
    delay(50);
  }

  digitalWrite(waterRelay, HIGH);
  digitalWrite(ledYellow, LOW);
  Serial.println("[INFO] Water Pulse OFF");
  return true;
}

bool fanRun()
{
  Serial.println("[INFO] Fan ON");

  if (digitalRead(fanRelay) != HIGH)
  {
    return false;
  }

  digitalWrite(fanRelay, LOW);
  digitalWrite(ledYellow, HIGH);
  delay(20000);
  digitalWrite(fanRelay, HIGH);
  digitalWrite(ledYellow, LOW);
  Serial.println("[INFO] Fan OFF");
  return true;
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

  // Step 1 — Move arm to clean position
  Serial.println("[STEP 1] Moving arm to cleaning position...");
  if (!moveArmToClean())
  {
    reportError("Step 1 — Arm failed to reach cleaning position");
    cycleRunning = false;
    return;
  }
  delay(1000);

  // Step 2 — Water pulse
  Serial.println("[STEP 2] Water pulse...");
  if (!waterPulse())
  {
    reportError("Step 2 — Water relay failed to fire");
    cycleRunning = false;
    return;
  }

  // Step 3 — Soak
  Serial.println("[STEP 3] Soaking for 5 seconds...");
  delay(5000);

  // Step 4 — Fan dry
  Serial.println("[STEP 4] Running fan...");
  if (!fanRun())
  {
    reportError("Step 4 — Fan relay failed to fire");
    cycleRunning = false;
    return;
  }
  delay(1000);

  // Step 5 — Return arm home
  Serial.println("[STEP 5] Returning arm home...");
  if (!moveArmHome())
  {
    reportError("Step 5 — Arm failed to return home");
    cycleRunning = false;
    return;
  }

  // Success
  setLeds(false, false, false, true);  // Green ON
  Serial.println();
  Serial.println("=================================");
  Serial.println("[SUCCESS] Cleaning Cycle Complete");
  Serial.println("=================================");
  delay(3000);

  setLeds(true, false, false, false);  // Back to blue / idle
}

// --- Setup ---

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

// --- Loop ---

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