#include <Wire.h>
#include <EnableInterrupt.h>

#define SLAVE_ADDRESS 0xC4
volatile byte bam = 128;                       // BAM byte from LIDAR

#define ontime_duration 240                    // seconds until bot deactivates self due to runtime
#define radio_duration    2                    // seconds until bot deactivates self due to radio drop
unsigned long int deadtimer = 0.;              // seconds of ontime
bool dead = false;                             // if deadman > deadman_duration * 1000, true

#define rc_switch_pin_in 9                      // switch pin
volatile int rc_switch_pulsewidth_in = -100;    // inital dummy value
volatile long rc_switch_pulse_start;            // start of pulse
volatile unsigned long int rc_switch_time = 0.; // Timestamp of last interrupt to kill bot if radio dies

#define latch_out 7                             // latch pin output to rpi

bool motor = true;  // disable motor commands
bool debug = true; // disable serial output

////STARTING CONFIGURATION////////////////////////////////////////////////////
int incr =  75;              // bsae robot speed
int number = 90;             // flipper pos FIXME rename
int threshold = 150;         // reflectance threshold. 500 on old bot
int leftCmd = incr;
int rightCmd = incr;
////STARTING CONFIGURATION////////////////////////////////////////////////////

typedef struct Motor
{
  int pin1;
  int pin2;
};
Motor leftmotor  = {6, 5};
Motor rightmotor = {3,11};

byte sns[4];

int longestMillis = 0;
unsigned long int lastMillis;
unsigned long pingTimer1, pingTimer2;

void setup() {
  if(debug)  Serial.begin(115200);
    
  /// INITIALIZE RADIO ////////////////////////////////////////////////////
  pinMode(rc_switch_pin_in, INPUT);
  pinMode(latch_out, OUTPUT);
  enableInterrupt(rc_switch_pin_in, isrSwitchRising, RISING);

  /// CONFIGURE i2c ///////////////////////////////////////////////////////////
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(receiveData);
  if (debug) Serial.println("i2c ready!");
}

bool pause = false;

void loop() {
  sns[0] = analogRead(2) < threshold ? 1 : 0;
  sns[1] = analogRead(3) < threshold ? 1 : 0;
  sns[2] = analogRead(0) < threshold ? 1 : 0;
  sns[3] = analogRead(1) < threshold ? 1 : 0;

  if (debug)
  {
    int duration = millis()-lastMillis;
    lastMillis = millis();
    Serial.print(analogRead(2));Serial.print(" | ");Serial.print(analogRead(3));Serial.print(" | ");
    Serial.print(analogRead(0));Serial.print(" | ");Serial.print(analogRead(1));Serial.print(" | ");
    Serial.print(duration);Serial.print(" || ");Serial.print(pause); Serial.print(" || ");
    Serial.print(rc_switch_pulsewidth_in);;Serial.print(" || ");Serial.println(deadtimer);
  }

  if ( (rc_switch_pulsewidth_in > 1600) && !dead)
  {
    if(sns[0])
    {
      leftCmd = incr;
    }
    if(sns[1])
    {
      rightCmd = incr;
    }
    if(sns[2])
    {
      leftCmd = -incr;
    }
    if(sns[3])
    {
      rightCmd = -incr;
    }

    // ADD BAM STEERING

      // TODO mix in steering here -> do a +/- delta for left and right to do a harder turn?
      motorSpeed(leftmotor, leftCmd /*+ leftSteer*/);  
      motorSpeed(rightmotor, rightCmd /*+ rightSteer*/);
      digitalWrite(latch_out, HIGH);
  }
  else
  {
      // motors off 
      motorSpeed(leftmotor, 0);
      motorSpeed(rightmotor, 0);
      digitalWrite(latch_out, LOW);
      if (debug) Serial.println("DEAD");  
  }

  // CHECK IF YOU SHOULD START TIMING
  if (!dead && (rc_switch_pulsewidth_in > 1600) && deadtimer==0)
  {
    deadtimer = millis();
    if (debug) Serial.println("START TIMING");
  }

  /// CHECK IF YOU SHOULD BE DEAD
  if (!dead && (rc_switch_pulsewidth_in > 1600) && ((millis() - deadtimer) >= ontime_duration*1000UL))
  {
    dead = true;
    if (debug) Serial.println("SETTING DEADMAN");
  }  
  
  // CHECK IF YOU SHOULD RESET
  if (dead && (rc_switch_pulsewidth_in < 1400))
  {
      dead = false;
      deadtimer = 0;
      if (debug) Serial.println("RESETTING DEADMAN");  
  }

  // CHECK IF RADIO HAS DROPPED OUT
  if (!dead && (rc_switch_pulsewidth_in > 1600) && ((millis()-rc_switch_time) > radio_duration*1000))
  {
      rc_switch_pulsewidth_in = -100;
      if (debug) Serial.println("RADIO DISCONNECTED");
  }
}

void motorSpeed(Motor m, int cmd) {
  if (cmd < 0) {
    analogWrite(m.pin1, abs(cmd));
    digitalWrite(m.pin2, LOW);
  }
  else if (cmd > 0) {
    digitalWrite(m.pin1, LOW);
    analogWrite(m.pin2, cmd);
  }
  else {
    digitalWrite(m.pin1, LOW);
    digitalWrite(m.pin2, LOW);
  }
}

/// ISR for switch activation
void isrSwitchFalling() {
  rc_switch_pulsewidth_in = micros() - rc_switch_pulse_start;
  rc_switch_time = millis();
  disableInterrupt(rc_switch_pin_in);
  enableInterrupt(rc_switch_pin_in, isrSwitchRising, RISING);
}
void isrSwitchRising() {
  rc_switch_pulse_start = micros();
  disableInterrupt(rc_switch_pin_in);
  enableInterrupt(rc_switch_pin_in, isrSwitchFalling, FALLING);
}

// callback for received data
void receiveData(int byteCount){
  while(Wire.available())    // slave may send less than requested
  { 
    bam = Wire.read();
  }
}
