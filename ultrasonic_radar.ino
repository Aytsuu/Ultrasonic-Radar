#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define trigPin 10
#define echoPin 11

LiquidCrystal_I2C lcd(0x27, 16, 2);
long duration;
int distance  ;
Servo myservo;

// Keypad Setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String correctUsername = "ABCD";
String correctPassword = "1234";
String inputUsername = "";
String inputPassword = "";
bool enteringUsername = true;
bool radarActive = false;  // Controls radar operation
bool authenticated = false; // Tracks login state
int cursorPos = 0;

// Radar Sweep Variables
int currentAngle = 15;
int sweepDirection = 1;
unsigned long previousSweepTime = 0;
const int sweepDelay = 50;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  myservo.attach(6);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.print("Enter Username:");
}

void loop() {
  char key = keypad.getKey();

  // Handle Keypad Input (works even during radar operation)
  if (key) {
    handleKeyInput(key);
  }

  // Run Radar Sweep if authenticated
  if (authenticated) {
    runNonBlockingSweep();
  }
}

void handleKeyInput(char key) {
  // If authenticated, only * key resets the system
  if (authenticated && key == '*') {
    resetSystem();
    return;
  }

  // Normal login process when not authenticated
  if (!authenticated) {
    if (key == '#') {
      if (enteringUsername) {
        enteringUsername = false;
        lcd.clear();
        lcd.print("Enter Password:");
        cursorPos = 0;
      } else {
        verifyLogin();
      }
    } else if (key == '*') {
      resetInput();
    } else {
      processCharacterInput(key);
    }
  }
}

void verifyLogin() {
  lcd.clear();
  if (inputUsername.equals(correctUsername) && inputPassword.equals(correctPassword)) {
    lcd.print("Access Granted!");
    authenticated = true;
    radarActive = true;
    delay(1000);
    lcd.clear();
    lcd.print("Radar Active");
    lcd.setCursor(0, 1);
    lcd.print("* to reset");
  } else {
    lcd.print("Access Denied!");
    delay(2000);
    resetInput();
  }
}

void runNonBlockingSweep() {
  if (millis() - previousSweepTime >= sweepDelay) {
    previousSweepTime = millis();

    myservo.write(currentAngle);
    int distance = calculateDistance();
    
    Serial.print(currentAngle);
    Serial.print(",");
    Serial.print(distance);
    Serial.println(".");

    currentAngle += sweepDirection;
    
    if (currentAngle >= 165 || currentAngle <= 15) {
      sweepDirection *= -1;
    }
  }
}

void processCharacterInput(char key) {
  if (enteringUsername) {
    inputUsername += key;
    lcd.setCursor(cursorPos, 1);
    lcd.print(key);
  } else {
    inputPassword += key;
    lcd.setCursor(cursorPos, 1);
    lcd.print("*");
  }
  cursorPos++;
}

void resetSystem() {
  authenticated = false;
  radarActive = false;
  resetInput();
  lcd.clear();
  lcd.print("System Reset");
  delay(1000);
  lcd.clear();
  lcd.print("Enter Username:");
}

void resetInput() {
  inputUsername = "";
  inputPassword = "";
  enteringUsername = true;
  cursorPos = 0;
  currentAngle = 15;
  sweepDirection = 1;
}

int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}