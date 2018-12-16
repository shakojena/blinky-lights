const int ledPin = 26;          //LED attached to GPIO 27
const int motionSensorPin = 14; //PIR sensor attached to GPIO 26
//timer variables:
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
#define timeSeconds 2//LED will turn off after the number of seconds defined
//**********************************************************************************
void setup() {  
  Serial.begin(115200);//initialise serial communication at 115200 bps  
  pinMode(motionSensorPin, INPUT_PULLUP);//PIR Motion Sensor mode INPUT_PULLUP
  //sets motionSensorPin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensorPin), detectsMovement, RISING);
//  /pinMode(ledPin, OUTPUT);//sets ledPin as OUTPUT
//  /digitalWrite(ledPin, LOW);//LED is off by default
}
void loop() {  
  now = millis();//current time
  //turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
//  /  digitalWrite(ledPin, LOW);
    startTimer = false;
  }
}
//checks if motion was detected, sets LED HIGH and starts a timer
void detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
//  /digitalWrite(ledPin, HIGH);
  startTimer = true;
  lastTrigger = millis();
}
