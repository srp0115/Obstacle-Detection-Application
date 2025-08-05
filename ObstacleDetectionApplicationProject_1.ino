#include <LiquidCrystal.h>
#define LCD_RS_PIN A5 
#define LCD_E_PIN A4
#define LCD_D4_PIN 6
#define LCD_D5_PIN 7
#define LCD_D6_PIN 8
#define LCD_D7_PIN 9
LiquidCrystal lcd(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN,
                  LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

#include <IRremote.h>
#define IR_RECEIVE_PIN 5
#define IR_BUTTON_PLAY 64
#define IR_BUTTON_EQ 25
#define IR_BUTTON_OFF 69
#define IR_BUTTON_UP 9
#define IR_BUTTON_DOWN 7

#define PHOTORESISTOR_PIN A0

#include <EEPROM.h>
#define EEPROM_DISTANCE_UNIT 10

#define ECHO_PIN 3
#define TRIGGER_PIN 4

#define WARNING_YELLOW_PIN 11
#define ERROR_RED_PIN 12
#define LIGHT_GREEN_PIN 10

#define PUSHBUTTON_PIN 2

#define DISTANCE_UNIT_CM 0
#define DISTANCE_UNIT_IN 1
int distanceUnit = DISTANCE_UNIT_CM;
#define CM_TO_INCHES 0.393701

#define LOCK_DISTANCE 5.0
#define WARNING_DISTANCE 10.0

unsigned long lastTimeUltrasonicTrigger = millis();
unsigned long ultrasonicTriggerDelay = 60;

volatile unsigned long pulseInTimeBegin;
volatile unsigned long pulseInTimeEnd;
volatile bool newDistanceAvailable = false;

double previousDistance = 400.0;
//warning/yellow led
unsigned long lastTimeWarningLEDBlinked = millis();
unsigned long warningLEDDelay = 200;
byte warningLEDState = LOW;

//error/red led
byte errorLEDState = LOW;
unsigned long lastTimeErrorLEDBlinked = millis();
unsigned long errorLEDDelay = 200;

volatile bool isLocked = false;
//bool to check if locked/unlocked

//push button
unsigned long lastTimeButtonChanged = millis();
unsigned long debounceDelay = 50;
byte buttonState= LOW;

//photoresistor
unsigned long lastTimeReadLuminosity = millis();
unsigned long luminosityDelay = 100;

#define LCD_MODE_LUMINOSITY 2
#define LCD_MODE_DISTANCE 0
#define LCD_MODE_SETTINGS 1
int lcdMode = LCD_MODE_DISTANCE;

void triggerUltrasonicSensor() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
}

double getUltrasonicDistance() {
  double durationMicros = pulseInTimeEnd - pulseInTimeBegin;
  double distance = durationMicros / 58.0;
  if (distance > previousDistance) {
    return previousDistance;
  }
  distance = previousDistance * 0.6 + distance * 0.4;
  previousDistance = distance;
  return distance;
}

void echoPinInterrupt() {
  if (digitalRead(ECHO_PIN) == HIGH) {
    pulseInTimeBegin = micros();
  }
  else {
    pulseInTimeEnd = micros();
    newDistanceAvailable = true;
  }
}

void toggleWarningLED() {
  if (warningLEDState == HIGH) {
    warningLEDState = LOW;
  }
  else {
    warningLEDState = HIGH;
  }
  digitalWrite(WARNING_YELLOW_PIN, warningLEDState);
}

void toggleErrorLED() {
  if (errorLEDState == HIGH) {
    errorLEDState = LOW;
  }
  else {
    errorLEDState = HIGH;
  }
  digitalWrite(ERROR_RED_PIN, errorLEDState);
}

void lockApplication() {
  if (!isLocked) {
    isLocked = true;
    lcd.clear();
    errorLEDState = LOW;
    warningLEDState = LOW;
  }
}

void unlockApplication() {
  if (isLocked) {
    isLocked = false;
    lcd.clear();
    errorLEDState = LOW;
    digitalWrite(ERROR_RED_PIN, errorLEDState);
    lcdMode = LCD_MODE_DISTANCE;
  }
}

void setWarningLEDBlinkRateFromDistance(double distance) {
  // to get 0-400 cm to 0-1600 ms -> multiply by 4
  warningLEDDelay = distance * 4;
}

void distanceLCDDisplay(double distance) {
  if (isLocked) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  !!Obstacle!!  ");
    lcd.setCursor(0, 1);
    lcd.print("Press to unlock.    ");
  }
  else {
    if (lcdMode == LCD_MODE_DISTANCE) {
      lcd.setCursor(0, 0);
      lcd.print("Dist:  ");
      if (distanceUnit == DISTANCE_UNIT_IN) {
        lcd.print(distance * CM_TO_INCHES);
        //cm to in conversion
        lcd.print(" in       ");
      }
      else {
        lcd.print(distance);
        lcd.print(" cm      ");
      }
    }
    lcd.setCursor(0, 1);
    if (distance > WARNING_DISTANCE && (lcdMode == LCD_MODE_DISTANCE || lcdMode == LCD_MODE_LUMINOSITY)) {
      lcd.print("No obstacle.        ");
    }
    if (distance < WARNING_DISTANCE && (lcdMode == LCD_MODE_DISTANCE || lcdMode == LCD_MODE_LUMINOSITY)) {
      lcd.print("  !!Warning!!   ");
    }
  }
}

void changeDistanceUnit() {
  if (!isLocked) {
    if (distanceUnit == DISTANCE_UNIT_CM) {
      distanceUnit = DISTANCE_UNIT_IN;
    }
    else {
      distanceUnit = DISTANCE_UNIT_CM;
    }
    EEPROM.write(EEPROM_DISTANCE_UNIT, distanceUnit);
  }
}

void resetSettings() {
  if (!isLocked) { 
    if (lcdMode == LCD_MODE_SETTINGS) {
      distanceUnit = DISTANCE_UNIT_CM;
      EEPROM.write(EEPROM_DISTANCE_UNIT, distanceUnit);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Settings reset! ");
    }
  } 
}

void changeLightLEDFromLuminosity(int luminosity) {
  byte brightness = 255 - luminosity / 4;
  analogWrite(LIGHT_GREEN_PIN, brightness);
}

void luminosityLCDScreen(int luminosity) {
  if (lcdMode == LCD_MODE_LUMINOSITY) {
    lcd.setCursor(0, 0);
    lcd.print("Luminosity:  ");
    lcd.print(luminosity);
    lcd.print("      ");
  }
}

void toggleLCDScreen(bool next) {
  if (!isLocked) {
    switch(lcdMode) {
      case LCD_MODE_DISTANCE: {
        lcdMode = (next) ? LCD_MODE_SETTINGS : LCD_MODE_LUMINOSITY;
        break;
      }
      case LCD_MODE_SETTINGS: {
        lcdMode = (next) ? LCD_MODE_LUMINOSITY : LCD_MODE_DISTANCE;
        break;
      }
      case LCD_MODE_LUMINOSITY: {
        lcdMode = (next) ? LCD_MODE_DISTANCE : LCD_MODE_SETTINGS;
        break;
      }
      default: {
        lcdMode = LCD_MODE_DISTANCE;
      }
    }
    lcd.clear();
    if (lcdMode == LCD_MODE_SETTINGS) {
      lcd.setCursor(0, 0);
      lcd.print("Press on OFF to   ");
      lcd.setCursor(0, 1);
      lcd.print("reset settings.   ");
    }
  } 
}

void irCommand(long command) {
  switch (command) {
    case IR_BUTTON_PLAY: {
      unlockApplication();
      break;
    }
    case IR_BUTTON_UP: {
      toggleLCDScreen(true);
      break;
    }
    case IR_BUTTON_DOWN: {
      toggleLCDScreen(false);
      break;
    }
    case IR_BUTTON_OFF: {
      resetSettings();
      break;
    }
    case IR_BUTTON_EQ: {
      changeDistanceUnit();
      break;
    }
    default: {
      break;
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ECHO_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echoPinInterrupt, CHANGE);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(WARNING_YELLOW_PIN, OUTPUT);
  pinMode(ERROR_RED_PIN, OUTPUT);
  pinMode(LIGHT_GREEN_PIN, OUTPUT);
  pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);

  buttonState = digitalRead(PUSHBUTTON_PIN);

  lcd.begin(16,2);
  lcd.print("Hello!");
  delay(2000);
  lcd.clear();
  lcd.print("Welcome to my");
  lcd.setCursor(0,1);
  lcd.print("app!");
  delay(2000);
  lcd.clear();

  IrReceiver.begin(IR_RECEIVE_PIN);

  distanceUnit = EEPROM.read(EEPROM_DISTANCE_UNIT);
  if (distanceUnit == 255) {
    distanceUnit = DISTANCE_UNIT_CM;
  }
}

void loop() {
  unsigned long timeNow = millis();
  if (timeNow - lastTimeUltrasonicTrigger > ultrasonicTriggerDelay) {
    lastTimeUltrasonicTrigger += ultrasonicTriggerDelay;
    triggerUltrasonicSensor();
  }
  if (newDistanceAvailable) {
    newDistanceAvailable = false;
    double distance = getUltrasonicDistance();
    setWarningLEDBlinkRateFromDistance(distance);
    if (distance < LOCK_DISTANCE) {
      //check if obstacle too close to sensor
      lockApplication();
    }
    distanceLCDDisplay(distance);
  }
  if (isLocked) {
    if (timeNow - lastTimeErrorLEDBlinked > errorLEDDelay){
    lastTimeErrorLEDBlinked += errorLEDDelay;
    toggleErrorLED();
    toggleWarningLED();
    }
    if (timeNow - lastTimeButtonChanged > debounceDelay) {
      byte newButtonState = digitalRead(PUSHBUTTON_PIN);
      if (newButtonState != buttonState) {
        lastTimeButtonChanged = timeNow;
        buttonState = newButtonState;
        if (buttonState == LOW) {
          unlockApplication();
        }
      }
    }
  }
  if (!isLocked) {
    if (timeNow - lastTimeWarningLEDBlinked > warningLEDDelay){
      lastTimeWarningLEDBlinked += warningLEDDelay;
      toggleWarningLED();
    }
    if (timeNow - lastTimeReadLuminosity > luminosityDelay) {
      lastTimeReadLuminosity += luminosityDelay;
      int luminosity = analogRead(PHOTORESISTOR_PIN);
      changeLightLEDFromLuminosity(luminosity);
      luminosityLCDScreen(luminosity);
    } 
  }
  if (IrReceiver.decode()) {
    IrReceiver.resume();
    long command = IrReceiver.decodedIRData.command;
    irCommand(command);
  }
}
  


// Step 1: get the distance from the ultrasonic sensor, using interrupts, and print on serial
// to start.
// - Step 2: Make the warning LED blink. Change the blink rate depending on the measured
// distance (0-400 cm → 0-1600 ms).
// - Step 3: Lock the application when the distance is below a certain threshold (10cm),
// using a global boolean variable for the lock. When the app is locked, make the error +
// warning LEDs blink at the same time, every 300ms.
// - Step 4: Debounce the button with polling (not interrupts here), and check for when we
// release the button while the app is locked. In that case, unlock the app and come back
// to normal behavior.
// - Step 5: Setup LCD screen and print a message for 1 second in the setup(), before
// starting the loop().
// - Step 6: Print measured distance and warning message on the LCD when the app is not
// locked. If locked, print an error message that only disappears when the app is unlocked.
// - Step 7: Setup IR remote controller (with version 3 of the library here, and I’ll show how
// to do with version 2 at the end), and map the buttons we’ll need for the project.
// - Step 8: Create the switch for all commands coming from the remote controller. When
// pressing on PLAY while the app is locked, also unlock the app.
// - Step 9: Change the distance unit when pressing on the EQ button, and also save it into
// EEPROM. In the setup(), read the value from EEPROM. Update the distance message
// accordingly on the LCD screen, using the correct unit.
// - Step 10: Add different modes (screens) for the LCD screen. The first one is the
// “distance” mode we already have. Second is a screen with a message saying “press on
// OFF to reset settings”. Toggle between the 2 screens with the UP/DOWN buttons. When
// on the “reset” screen, press on OFF to reset default settings, and print a confirmation
// message.
// - Step 11: Read value from photoresistor. Set the brightness for the light LED depending
// on the luminosity. Add a 3rd screen on the LCD screen to print the current luminosity.
