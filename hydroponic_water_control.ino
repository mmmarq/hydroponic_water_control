// Define Variables
volatile uint16_t waterSensorReading = 0;           // measuring the rising edges of the signal from water sensor
volatile uint8_t lastflowpinstate;
unsigned long currTime = 0;                         // store current time in milliseconds since board is up
unsigned long prevTime = 0;                         // store last time lapse reading
unsigned long previousBlinkMillis = 0;              // store last time green led blink
boolean pumpState = false;                          // define pump state (true = on - false = off)
boolean newBumpState = false;                       // set new pump state (true = on - false = off)
boolean nightTime = false;                          // define night time
int greenLedStatus = HIGH;                          // set initial green led status
const unsigned long PUMPONINTERVAL = 900000;        // milliseconds - 15 minutes
const unsigned long PUMPOFFNIGHTINTERVAL = 1800000; // milliseconds - 30 minutes
const unsigned long PUMPOFFDAYINTERVAL = 900000;    // milliseconds - 15 minutes
const unsigned long WATERDELAY = 10000;             // milliseconds - 10 seconds
const unsigned long BLINKINTERVAL = 250;            // milliseconds - 0.25 seconds
const byte LDRPIN = A0;                             // set LDR analogic pin
const byte GREENLED = 4;                            // set digital GREEN led pin - system is up
const byte YELLOWLED = 5;                           // set digital BLUE led pin - pump is on
const byte REDLED = 6;                              // set digital RED led pin - no water flow detected
const byte PUMPPIN = 3;                             // set digital water pump pin
const byte WATERSENSORPIN = 2;                      // water sensor HALL effect pin - Uno, Nano, Mini, other 328-based = Pin 2, 3
const int MINLIGHTLEVEL = 80;                       // set minimum light level to determine if it is day or night
const boolean DEBUG = true;                         // enable or disable debug messages

// Interrupt is called once a millisecond, looks for any pulses from the water sensor!
// https://github.com/adafruit/Adafruit-Flow-Meter
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(WATERSENSORPIN);
  
  if (x == lastflowpinstate) {
    return; // nothing changed!
  }
  if (x == HIGH) {
    //low to high transition!
    waterSensorReading++;
  }
  lastflowpinstate = x;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void setup() {
  Serial.begin(9600);                                         // set serial communication speed
  if (DEBUG) Serial.println("Starting system setup...");
  pinMode(GREENLED, OUTPUT);                                  // set GREEN led pin configuration
  pinMode(YELLOWLED, OUTPUT);                                 // set BLUE led pin configuration
  pinMode(REDLED, OUTPUT);                                    // set RED led pin configuration
  pinMode(PUMPPIN, OUTPUT);                                   // set PUMP pin configuration
  digitalWrite(PUMPPIN,HIGH);                                 // set water pump off
  pinMode(WATERSENSORPIN, INPUT);                             // set water pump pin configuration
  digitalWrite(WATERSENSORPIN, LOW);                          // turn water pump on
  newBumpState = true;                                        // start system with water pump on
  lastflowpinstate = digitalRead(WATERSENSORPIN);             // set initial water flow sensor pin state
  useInterrupt(true);                                         // turn water sensor interrupt on
  if (DEBUG) Serial.println("System setup concluded!");
}

void loop() {
  currTime = millis();                                        // read current time in milliseconds since system is UP
  if (currTime < prevTime) currTime = prevTime;               // check for currTime overflow (after aprox. 50 days);
  nightTime = isNightTime();                                  // check if it is night time
  setGreenLedStatus();                                        // set green led status
  if (nightTime){                                             // check if it is night time
    if (pumpState){                                           // check if current pump state is ON
      if ((currTime - prevTime) >= PUMPONINTERVAL){           // check if it is time to turn pump OFF
        if (DEBUG) Serial.println("Its time to stop water flow...");
        newBumpState = false;                                 // set new pump status to OFF
        prevTime = currTime;                                  // store last state change update
      }
    }else{                                                    // check if current pump state is OFF
      if ((currTime - prevTime) >= PUMPOFFNIGHTINTERVAL){     // check if it is time to turn pump ON 
        if (DEBUG) Serial.println("Its time to start water flow...");
        newBumpState = true;                                  // set new pump status to ON
        prevTime = currTime;                                  // store last state change update
      }
    }
  }else{                                                      // check if it is night time
    if (pumpState){                                           // check if current pump state is ON
      if ((currTime - prevTime) >= PUMPONINTERVAL){           // check if it is time to turn pump OFF
        if (DEBUG) Serial.println("Its time to stop water flow...");
        newBumpState = false;                                 // set new pump status to OFF
        prevTime = currTime;                                  // store last state change update
      }
    }else{                                                    // check if current pump state is OFF
      if ((currTime - prevTime) >= PUMPOFFDAYINTERVAL){       // check if it is time to turn pump ON 
        if (DEBUG) Serial.println("Its time to start water flow...");
        newBumpState = true;                                  // set new pump status to ON
        prevTime = currTime;                                  // store last state change update
      }
    }
  }

  pumpConfig(newBumpState);                                   // set new pump state

  if (pumpState && ((currTime - prevTime) >= WATERDELAY)){    // if water pump is ON, check if it is ON for time enough
    if (DEBUG) Serial.println("Checking water flow...");
    if (hasWatherFlow()){                                     // check if water sensor has detected water flow
      digitalWrite(REDLED,LOW);                               // if water flow was detected, turn RED led OFF
    }else{
      digitalWrite(REDLED,HIGH);                              // if water flow was not detected, turn RED led ON
    }
  }
  myDelay(1000);                                              // delay without delay function due HW interruption
  if (DEBUG) Serial.print("Water flow counter: ");
  if (DEBUG) Serial.println(waterSensorReading);
}

void myDelay(unsigned long mSec){
  unsigned long locMillis = millis();
  while ((millis() - locMillis) < mSec){
    // do nothing
  }
}

boolean isNightTime(){                                        // function to check if it is night time based on env light level
  int lightLevel = analogRead(LDRPIN);                        // read LDR
  if (lightLevel < MINLIGHTLEVEL ){                           // if env light level lower than minimal
    if (DEBUG) Serial.print("Its night - ");
    if (DEBUG) Serial.println(lightLevel);
    return true;                                              // it is NIGHT
  }else{
    if (DEBUG) Serial.print("Day light - ");
    if (DEBUG) Serial.println(lightLevel);
    return false;                                             // it is DAY
  }
}

void setGreenLedStatus(){                                     // function to set green led status (blink at night and continuous at day light)
  if (nightTime){                                             // check if it is night time
    if ((currTime - previousBlinkMillis) >= BLINKINTERVAL){   // check if it is time to change led status
      previousBlinkMillis = currTime;                         // reset led blink interval
      if (greenLedStatus == HIGH){                            // check green led status
        greenLedStatus = LOW;                                // change green led status
      }else{
        greenLedStatus = HIGH;                                // change green led status
      }
    }
  }else{
    greenLedStatus = HIGH;                                    // if not night time, keep green led on
  }
  digitalWrite(GREENLED,greenLedStatus);                      // set green led status
}

void pumpConfig(int newState){                                // function to set water pump ON or OFF
  if (newState != pumpState){                                 // check if pump state changes
    if (newState){                                            // if newState TRUE, turn pump on setting pin LOW
      if (DEBUG) Serial.println("Water pump ON!");
      digitalWrite(PUMPPIN,LOW);                              // turn water pump ON
      digitalWrite(YELLOWLED,HIGH);                           // turn water pump led indicator ON
    }else{                                                    // if newState FALSE, turn pump off setting pin HIGH
      if (DEBUG) Serial.println("Water pump OFF!");
      digitalWrite(PUMPPIN,HIGH);                             // turn water pump ON
      digitalWrite(YELLOWLED,LOW);                            // turn water pump led indicator OFF
    }
    waterSensorReading = 0;                                   // Reset interrupt counter
    pumpState = newState;                                     // store new water pump state
  }
}

boolean hasWatherFlow(){                                      // function to check if there is water flow on system
  if (waterSensorReading > 0){                                // if water sensor counter greater than zero
    if (DEBUG) Serial.println("Great, water flow detected!");
    waterSensorReading = 0;
    return true;                                              // water flow was detected
  }else{
    if (DEBUG) Serial.println("ALARM - water flow not detected!");
    waterSensorReading = 0;
    return false;                                             // water flow was not detected
  }
}


