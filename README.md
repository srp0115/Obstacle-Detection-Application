# Obstacle-Detection-Application

This project is an Arduino-based Obstacle Detection Application that utilizes an ultrasonic sensor, LEDs, an LCD display, a photoresistor, a pushbutton, and an IR remote for versatile obstacle sensing and user interaction.

## Features

- **Ultrasonic Distance Measurement:** Continuously measures distance to detect obstacles.
- **Visual Alerts:** 
  - Warning (Yellow) LED blinks at a rate based on distance to the obstacle.
  - Error (Red) LED blinks when an object is too close (lock condition).
  - Green LED brightness adjusts according to ambient light.
- **LCD Display:** Shows distance, warning/error messages, and ambient luminosity.
- **Lock Mechanism:** Application locks if an obstacle is detected too close; unlocks via button or IR remote.
- **Infrared (IR) Remote Control:** 
  - Switch display modes (distance, settings, luminosity).
  - Change distance units (cm/inches).
  - Reset settings.
- **EEPROM Support:** Remembers distance unit preference between power cycles.
- **Debounced Pushbutton:** Reliable user interaction for unlocking and other features.
- **Photoresistor:** Adjusts an LED's brightness and provides luminosity readings on the LCD.

## Hardware Requirements

- Arduino UNO (or compatible)
- Ultrasonic sensor (HC-SR04 or similar)
- LCD 16x2 (with 6 control/data pins)
- IR receiver module
- IR remote controller (compatible with IRremote library)
- 3 LEDs (Yellow, Red, Green)
- Photoresistor (LDR) and suitable resistor
- Pushbutton
- EEPROM (onboard on most Arduinos)
- Jumper wires, breadboard, etc.

## Pinout

| Component        | Arduino Pin        |
| ---------------- | ----------------- |
| LCD RS           | A5                |
| LCD E            | A4                |
| LCD D4           | 6                 |
| LCD D5           | 7                 |
| LCD D6           | 8                 |
| LCD D7           | 9                 |
| IR Receiver      | 5                 |
| Photoresistor    | A0                |
| Ultrasonic Echo  | 3                 |
| Ultrasonic Trigger| 4                |
| Warning LED      | 11                |
| Error LED        | 12                |
| Light Green LED  | 10 (PWM)          |
| Pushbutton       | 2                 |

## How It Works

1. **Startup:** LCD displays a welcome message.
2. **Distance Monitoring:** Continuously checks for nearby obstacles. If too close, system locks and alerts the user.
3. **Unlocking:** Press the button or use the IR remote to unlock.
4. **Modes:** Switch between distance, settings, and luminosity display using the IR remote.
5. **Settings:** Change between centimeters and inches, reset settings, or view luminosity.

## Remote Control Mapping

| Button    | Function                |
| --------- | ----------------------- |
| PLAY      | Unlock application      |
| UP        | Next LCD mode           |
| DOWN      | Previous LCD mode       |
| OFF       | Reset settings (in settings mode) |
| EQ        | Change distance unit    |

## Getting Started

1. Clone the repository.
2. Open the `.ino` file in the Arduino IDE.
3. Connect your hardware according to the pinout above.
4. Upload the code to your Arduino.
5. Power on and test the system with the remote, LEDs, and sensors.

## Libraries Used

- `LiquidCrystal`
- `IRremote`
- `EEPROM`

Install these libraries via Arduino Library Manager if not already available.

## License

This project is for educational and personal use. Feel free to modify and adapt!

---

**Author:** srp0115  
**Repository:** [Obstacle-Detection-Application](https://github.com/srp0115/Obstacle-Detection-Application)
