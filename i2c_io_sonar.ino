#include <Wire.h>
#include <Servo.h>
#include <NewPing.h>
#include <util/atomic.h>

#define SLAVE_ADDRESS 0x04
bool debug = true;
bool slave = false;

// TODO add radio enable
// TODO add radio override

////STARTING CONFIGURATION////////////////////////////////////////////////////
int number = 90;             // flipper pos FIXME rename
int threshold = 500;        // reflectance threshold
int leftCmd = 90;
int rightCmd = 90;
int flipperMax = 45;        // degrees off neutral
////STARTING CONFIGURATION////////////////////////////////////////////////////

Servo flipperLeft;  
Servo flipperRight;
Servo motorLeft;  
Servo motorRight;

NewPing sonar1(2, 3, 50); // Sensor 1: trigger pin, echo pin, maximum distance in cm
NewPing sonar2(4,  5, 50); // Sensor 2: same stuff
#define pingSpeed 20        // Ping frequency (in milliseconds), fastest we should ping is about 35ms per sensor


struct rpt {
  byte sns[4];
  byte flipper;
  byte left;
  byte right;
} report;

struct sensor
{
  int    val;     // current sensor value
  float   ma;     // moving average
  float  per;     // persistence, count
  bool  good;     // logic in read_sensor deems this sensor to be "good"
};

void read_sensor(sensor & s)
{
   s.per = 0.8*s.per + 0.2*s.val;
   s.ma =  0.8*s.ma + 0.2*s.val;

   if(s.per > 0.8)  // 4 out of 5
   {
     s.good = true;
   } 
   else 
   {
     s.good = false; 
   }
}

void sensor_logic(sensor & left, sensor & right)
{
   float delta = 0.2;    // percentage required to take action

   // if no target, do nothing
   if (!left.good && !right.good)
   {
      // DO NOTHING     
      Serial.println("NO TARGET");
   }
   // if left valid
   else if (left.good && !right.good)
   {
     Serial.println("ONLY  LEFT"); 
   }
   // if right valid
   else if (!left.good && right.good)
   {
     Serial.println("ONLY  RIGHT"); 
   }
   // if two valid readings
   else
   {
     /* FIXME these only hit a couple ticks. Ignore until needed.
     if (left.ma*(1+delta) < right.ma)
     {
       // if left, turn left
       Serial.println("TURN LEFT"); 
     } 
     else if (left.ma > right.ma*(1+delta))
     {
       // if right, turn right
       Serial.println("TURN RIGHT"); 
     }
     else
     */
     {
       // if right, turn right
       Serial.println("FORWARD");  
     }
   }
}

sensor left, right;

int longestMillis = 0;
unsigned long int lastMillis;
unsigned long pingTimer1, pingTimer2;

void setup() {
  if(debug)  Serial.begin(115200);

  if(slave)
  {
    /// INITIALIZE STRUCTS //////////////////////////////////////////////////////
    report.flipper = (byte)number;
    report.left    = (byte)leftCmd;
    report.right   = (byte)rightCmd;

    /// CONFIGURE i2c ///////////////////////////////////////////////////////////
    Wire.begin(SLAVE_ADDRESS);
    Wire.onReceive(receiveData);
    Wire.onRequest(sendData);
    Serial.println("i2c ready!");
  } else {
    /// INITIALIZE STRUCTS ////////////////////////////////////////////////////
    report.flipper = (byte)number;
    report.left    = (byte)90;
    report.right   = (byte)90;
  }
  /// ATTACH SERVOS ///////////////////////////////////////////////////////////
  flipperLeft.attach(12);  // attaches the servo on pin 9 to the servo object
  flipperRight.attach(13);  // attaches the servo on pin 9 to the servo object
  motorLeft.attach(11);  // attaches the servo on pin 9 to the servo object
  motorRight.attach(06);  // attaches the servo on pin 9 to the servo object

  if(debug) Serial.println("Arm");

  /// ARM SPEED CONTROLLERS AND SET TO NEUTRAL POINT //////////////////////////
  // TODO function to sync flippers
  flipperLeft.write(report.flipper);  // attaches the servo on pin 9 to the servo object
  flipperRight.write(report.flipper);  // attaches the servo on pin 9 to the servo object

  motorLeft.write(0);  
  motorRight.write(0);  
  delay(1000);
  motorLeft.write(180); 
  motorRight.write(180);
  delay(1000);
  motorLeft.write(report.left);
  motorRight.write(report.right);

  pingTimer1 = millis() + pingSpeed; // Sensor 1 fires after 100ms (pingSpeed)
  pingTimer2 = pingTimer1 + (pingSpeed / 2); // Sensor 2 fires 50ms later
}

void loop() {
  if(slave)
  {
    motorLeft.write(report.left);
    motorRight.write(report.right);
    flipperLeft.write(max(min(report.flipper,180-flipperMax),flipperMax));
    flipperRight.write(max(min(180. - report.flipper,180-flipperMax),flipperMax));
  }

  report.sns[3] = analogRead(3) < threshold ? 1 : 0;
  report.sns[0] = analogRead(0) < threshold ? 1 : 0;
  report.sns[1] = analogRead(1) < threshold ? 1 : 0;
  report.sns[2] = analogRead(2) < threshold ? 1 : 0;

  if (~slave)
  {
    if (millis() >= pingTimer1) 
    {
      pingTimer1 += pingSpeed; // Make sensor 1 fire again 100ms later (pingSpeed)
      left.val = sonar1.ping_cm();
      read_sensor(left);
    }
    if (millis() >= pingTimer2) 
    {
      pingTimer2 = pingTimer1 + (pingSpeed / 2); // Make sensor 2 fire again 50ms after sensor 1 fires
      right.val = sonar2.ping_cm();
      read_sensor(right);

      // both sensors read. Do logic.
      sensor_logic(left, right);

      int duration = millis()-lastMillis;
      lastMillis = millis();
      Serial.print(left.val);Serial.print(" | ");Serial.print(right.val);Serial.print(" || ");
      Serial.print(left.per);Serial.print(" | ");Serial.print(right.per);Serial.print(" || ");
      Serial.print(left.ma); Serial.print(" | ");Serial.print(right.ma); Serial.print(" || ");
      Serial.print(duration);Serial.print(" || ");
    }

    if(report.sns[0])
      motorLeft.write(180-90-15);
    if(report.sns[1])
      motorRight.write(90+15);
    if(report.sns[2])
      motorLeft.write(180-90+15);
    if(report.sns[3])
      motorRight.write(90-15);
    delay(5);
  }
}

// callback for received data
void receiveData(int byteCount){
  int i = 0;
  byte * buf = new byte[byteCount];
  while(Wire.available())    // slave may send less than requested
  { 
    buf[i++] = Wire.read();
  }
  if (byteCount == 1)
  {}
  else
  {
    // buf[0] is the offset! ignore
    report.flipper = buf[1];
    report.left    = buf[2];
    report.right   = 180.-buf[3]; // FIXME esc is running opposite direction
  }
  if(debug)
  {
    Serial.println("received data");
    for (int i = 0; i < byteCount; i++)
    {
      Serial.print(buf[i]); 
      if (i < byteCount-1)
        Serial.print(" | ");  
    }  
    Serial.println(" ");
  }
  delete buf;
}

// callback for sending data
void sendData(){
  Wire.write((byte *)&report, sizeof(report));
}
