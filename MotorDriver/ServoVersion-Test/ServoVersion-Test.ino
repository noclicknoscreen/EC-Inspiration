// http://www.bajdi.com
// Rotating a continuous servo (tested with a SpringRC SM-S4306R)

#include "NCNS-Servo.h"

ServoWrapper myServo;

// Uncomment to display debug messages on serial
//#define DEBUG
//#define SERVO_DEBUG

#define PIN_UP 2
#define PIN_DN 3
#define FC_DN 4
#define FC_UP 6
#define SERVO_CTRL_PIN 9
#define SERVO_ADJ A0

int upState, dnState;
int servoAdjust = 0, servoAdjustOld = 0;
float startTime, endTime = 0;
void setup()
{

  Serial.begin(9600);

  // attaches the servo on pin 9 to the servo object
  // And set the center value to 90 (half of 0 - 180)
  myServo.setup(SERVO_CTRL_PIN, 0);

  // Set the pins to PULL_UP (HIGH is default)
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DN, INPUT_PULLUP);
  // Set the end-of-course to Simple input
  // Those are 3 contacts
  pinMode(FC_UP, INPUT_PULLUP);
  pinMode(FC_DN, INPUT_PULLUP);
  // Set the the analog pin to center servo
  pinMode(SERVO_ADJ, INPUT);
  upState = 0;
  dnState = 0;
}

void loop()
{
  //  upState = digitalRead(PIN_UP);
  //  dnState = digitalRead(PIN_DN);
  //
  // -------------------------------------------------------
  // Reading command on serial
  // -------------------------------------------------------
  if (Serial.available() ) {
    int commande = Serial.read();
#ifdef DEBUG
    Serial.println(char(commande));
#endif
    if (char(commande) == '0') {
      // STOP
      cmd_stop();
    }
    if (char(commande) == '1') {
      // UP
      upState = 1;
      dnState = 0;
      startTime = millis();
    }
    if  (char(commande) == '2') {
      // D
      upState = 0;
      dnState = 1;
      startTime = millis();
    }
  }
#ifdef DEBUG
  Serial.print("States [UP, DN] : [");
  Serial.print(upState);
  Serial.print(",");
  Serial.print(dnState);
  Serial.print("]");
#endif
  // -------------------------------------------------------
  // Reading FC sensors
  // -------------------------------------------------------
  int fcUpState = digitalRead(FC_UP);
  int fcDnState = digitalRead(FC_DN);
  if ( (fcUpState == LOW && upState == HIGH)
       || (fcDnState == LOW && dnState == HIGH))
  {
    cmd_stop();
    // Debouncing The Limit Switches
    delay(200);
  }

#ifdef DEBUG
  Serial.print(" : ");
  Serial.print("FC States [UP, DN] : [");
  Serial.print(fcUpState);
  Serial.print(",");
  Serial.print(fcDnState);
  Serial.print("]");
#endif

  // -------------------------------------------------------
  // Reading adusting value for servo, if changed
  // -------------------------------------------------------
  int sensorValue = analogRead(SERVO_ADJ);
  servoAdjust = map(sensorValue, 0, 1023, -20, 20);
#ifdef DEBUG
  Serial.print(" : ");
  Serial.print("Read : ");
  Serial.print(sensorValue);
  Serial.print(", Adjusting value : ");
  Serial.println(servoAdjust);
#endif

  // Adusting Servo, if value has changed
  if (servoAdjust != servoAdjustOld ) {
    servoAdjustOld = servoAdjust;
    myServo.setup(SERVO_CTRL_PIN, servoAdjust);
  }

  // -------------------------------------------------------
  // Running the SERVO
  // -------------------------------------------------------
  if (upState == HIGH) {
    // Turn clockwise
    // 1 : Full speed clockwise
    // 0 : Would be stop
    // -1 : Full speed counterclockwise
    myServo.continousRotate(-1.5);

  } else if (dnState == HIGH) {
    // Turn counterclockwise
    myServo.continousRotate(0.25);

  } else {
    // Stop
    myServo.maintainCenter();
  }
}

/*
   Stopping the servo by setting all commands to zero
*/
void cmd_stop() {
  // STOP
  upState = 0;
  dnState = 0;
  endTime = millis();
  float timeElapsed = (endTime - startTime) / 1000;
  Serial.print("Time elapsed : ");
  Serial.print(String(timeElapsed));
  Serial.println(" seconds.");
}



