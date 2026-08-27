//library adding
#include <LiquidCrystal.h>
//settting it up
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//ultrasonic distance sensor
const int trigPin = 9;
const int echoPin = 8;

//
const int lightPin = A0;
const int buttonPin = 7;
const int ledPin = 6;
const int buzzerPin = 10;

const int LIGHT_THRESHOLD = 512;    // half of 0-1023
const int DISTANCE_THRESHOLD = 100; // cm
const unsigned long WRECK_TIME = 5000; // ms

enum State { OPEN_SEA, ANCHOR_DROPPED, STORM, CHARYBDIS, WRECKED };
State currentState = OPEN_SEA;
State lastDisplayedState = WRECKED; // force first LCD draw

bool anchored = false;
unsigned long dangerStart = 0;

bool lastButtonReading = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

unsigned long lastBlink = 0;
bool ledOn = false;

long readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return 9999;
  return duration * 0.034 / 2;
}

bool checkButtonPressed() {
  bool reading = digitalRead(buttonPin);
  bool pressed = false;

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        pressed = true;
      }
    }
  }

  lastButtonReading = reading;
  return pressed;
}

void updateLCD() {
  if (currentState == lastDisplayedState) return;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("State:");
  lcd.setCursor(0, 1);
  switch (currentState) {
    case OPEN_SEA:        lcd.print("OPEN SEA"); break;
    case ANCHOR_DROPPED:  lcd.print("ANCHOR DROPPED"); break;
    case STORM:           lcd.print("STORM"); break;
    case CHARYBDIS:       lcd.print("CHARYBDIS"); break;
    case WRECKED:         lcd.print("WRECKED"); break;
  }
  lastDisplayedState = currentState;
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
}

void loop() {
  if (currentState == WRECKED) {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
    updateLCD();
    return;
  }

  long distance = readDistanceCM();
  int light = analogRead(lightPin);

  Serial.print("Light: ");
  Serial.print(light);
  Serial.print("  Distance: ");
  Serial.println(distance);

  bool stormCondition = (light < LIGHT_THRESHOLD);
  bool charybdisCondition = (distance < DISTANCE_THRESHOLD);
  bool buttonPressed = checkButtonPressed();

  if (buttonPressed) {
    if (!anchored) {
      anchored = true;
      currentState = ANCHOR_DROPPED;
    } else {
      anchored = false;
      currentState = OPEN_SEA;
    }
  }

  if (!anchored) {
    switch (currentState) {
      case OPEN_SEA:
        if (charybdisCondition) {
          currentState = CHARYBDIS;
          dangerStart = millis();
        } else if (stormCondition) {
          currentState = STORM;
          dangerStart = millis();
        }
        break;

      case STORM:
        if (millis() - lastBlink > 250) {
          ledOn = !ledOn;
          digitalWrite(ledPin, ledOn ? HIGH : LOW);
          lastBlink = millis();
        }
        if (!stormCondition) {
          currentState = OPEN_SEA;
          digitalWrite(ledPin, LOW);
        } else if (millis() - dangerStart >= WRECK_TIME) {
          currentState = WRECKED;
        }
        break;

      case CHARYBDIS:
        digitalWrite(buzzerPin, HIGH);
        if (!charybdisCondition) {
          currentState = OPEN_SEA;
          digitalWrite(buzzerPin, LOW);
        } else if (millis() - dangerStart >= WRECK_TIME) {
          currentState = WRECKED;
        }
        break;

      case ANCHOR_DROPPED:
        break;

      default:
        break;
    }
  }

  if (currentState != STORM) digitalWrite(ledPin, LOW);
  if (currentState != CHARYBDIS) digitalWrite(buzzerPin, LOW);

  updateLCD();
}
