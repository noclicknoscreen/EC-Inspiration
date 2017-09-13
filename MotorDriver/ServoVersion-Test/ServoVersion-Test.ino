// http://www.bajdi.com
// Rotating a continuous servo (tested with a SpringRC SM-S4306R)

#include "NCNS-Servo.h" 

ServoWrapper myServo;

#define PIN_UP 2
#define PIN_DN 3
#define FC_UP 4
#define FC_DN 5

int upState, dnState;
 
void setup() 
{ 

  Serial.begin(115200);
  
  // attaches the servo on pin 9 to the servo object 
  // And set the center value to 90 (half of 0 - 180)
  myServo.setup(9, -3);

  // Set the pins to PULL_UP (HIGH is default)
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DN, INPUT_PULLUP);
  // Set the end-of-course to Simple input
  // Those are 3 contacts
  pinMode(FC_UP, INPUT_PULLUP);
  pinMode(FC_DN, INPUT_PULLUP);
  
} 
 
void loop() 
{ 
  // -----------------------------------------------------------------------
  upState = digitalRead(PIN_UP);
  dnState = digitalRead(PIN_DN);

  Serial.print("States [UP, DN] : [");
  Serial.print(upState);
  Serial.print(",");
  Serial.print(dnState);
  Serial.print("]");

  // -----------------------------------------------------------------------
  int fcUpState = digitalRead(FC_UP);
  int fcDnState = digitalRead(FC_DN);
  
  Serial.print(" : ");
  Serial.print("FC States [UP, DN] : [");
  Serial.print(fcUpState);
  Serial.print(",");
  Serial.print(fcDnState);
  Serial.println("]");

  // -------------------------------------------------------
  if(upState == LOW && fcUpState == LOW){
    // Turn clockwise
    // 1 : Full speed clockwise
    // 0 : Would be stop
    // -1 : Full speed counterclockwise
    myServo.continousRotate(1);
    
  }else if(dnState == LOW && fcDnState == LOW){
    // Turn counterclockwise
    myServo.continousRotate(-1);
    
  }else{
    // Stop
    myServo.maintainCenter();
    
  }
  
  delay(100);

} 

