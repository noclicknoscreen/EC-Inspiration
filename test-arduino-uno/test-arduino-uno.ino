// Initializing pins
const int pwr_a = 3;
const int pwr_b = 9;
const int dir_a = 2;
const int dir_b = 8;

// Step delay, is used as delay between each half-step, in the motor driver. 
// This delay is measured in microseconds.
int stepDelay = 600;

void setup(){
  // Initialize the pins, in the correct type of mode.
  pinMode(pwr_a,OUTPUT);
  pinMode(pwr_b,OUTPUT);
  pinMode(dir_a,OUTPUT);
  pinMode(dir_b,OUTPUT);
}

void loop(){
  // Test loop, should make your stepper go 40 steps forewards and backwards, with 1 second delay.
  Step4FWD(10);
  delay(1000);
  Step4BWD(10);
  delay(1000);
}


// This method, is called in order to make the stepper motor make 4 steps forwards (depending on your wiring).
void Step4FWD(int NumberOfTimes){
  
  // The function will run for the amount of times called in the method.
  // This is accomplished by a while loop, where it will subtract 1 from the amount, after every run.
  
  while(NumberOfTimes!=0){
    // Starting position (if repeated, ful step (4))
    // EXPLINATION: in this case, both our power are high.
    // Therefore both coils are activated, with their standard polarities for their magnetic fields.
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,HIGH);
    digitalWrite(dir_b,HIGH);
 
    delayMicroseconds(stepDelay);
 
    //Half step (½)
    // EXPLINATION: In this case, only out b-coil is active, still with it's stand polarity.
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,LOW);
    digitalWrite(dir_a,HIGH);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    //Full step (1)
    // EXPLINATION: In this case, the b-coil is activated as in previous cases.
    // But the a-coil now has it's direction turned on. So now it's active, but with the reversered polarity.
    // By continuing this pattern (for reference: http://www.8051projects.net/stepper-motor-interfacing/full-step.gif) , you'll get the axis to turn.
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,HIGH);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Half step (1½)
    digitalWrite(pwr_a,LOW);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Full step (2)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Half step (2½)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,LOW);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Full step (3)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,HIGH);
 
    delayMicroseconds(stepDelay);
 
    // Half step (3½)
    digitalWrite(pwr_a,LOW);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,HIGH);
 
    NumberOfTimes--; 
  }
 
 TurnOfMotors();
 }


// This method, is called in order to make the stepper motor make 4 steps backwards (depending on your wiring).
void Step4BWD(int NumberOfTimes){

  while(NumberOfTimes!=0){
 
    // Starting position (if repeated, ful step (4))
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Half step (½)
    digitalWrite(pwr_a,LOW);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Full step (1)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,HIGH);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Half step (1½)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,LOW);
    digitalWrite(dir_a,HIGH);
    digitalWrite(dir_b,LOW);
 
    delayMicroseconds(stepDelay);
 
    // Full step (2)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,HIGH);
    digitalWrite(dir_b,HIGH);
 
    delayMicroseconds(stepDelay);
 
    // Half step (2½)
    digitalWrite(pwr_a,LOW);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,HIGH);
 
    delayMicroseconds(stepDelay);
 
    // Full step (3)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,HIGH);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,HIGH);
 
    delayMicroseconds(stepDelay);

    // Half step (3½)
    digitalWrite(pwr_a,HIGH);
    digitalWrite(pwr_b,LOW);
    digitalWrite(dir_a,LOW);
    digitalWrite(dir_b,LOW); 
 
    NumberOfTimes--;
  }
 
 TurnOfMotors();
}

// This method simply just turn of the motors, called when ever we don't need the motors anymore.
// In this way, we won't fray the circuit or coils.
void TurnOfMotors(){
  digitalWrite(pwr_a,LOW);
  digitalWrite(pwr_b,LOW);
  digitalWrite(dir_a,LOW);
  digitalWrite(dir_b,LOW);
}
