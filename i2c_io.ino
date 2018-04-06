#include <Wire.h>
#include <Servo.h>
#include <util/atomic.h>

#define SLAVE_ADDRESS 0x04
bool debug = false;
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

struct rpt {
  byte sns[4];
  byte flipper;
  byte left;
  byte right;
} report;

void setup() {
    //if(debug)     Serial.begin(115200);

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
   /// INITIALIZE STRUCTS //////////////////////////////////////////////////////
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

}

void loop() {
    //unsigned long t = millis();//
//    delay(1); // aprox 5 ms, 200 hz
//    flipperLeft.write(max(min(report.flipper,180-flipperMax),flipperMax));
//    flipperRight.write(max(min(180. - report.flipper,180-flipperMax),flipperMax));
    if(slave)
    {
      motorLeft.write(report.left);
      motorRight.write(report.right);
    }
//    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      report.sns[3] = analogRead(3) < threshold ? 1 : 0;
      report.sns[0] = analogRead(0) < threshold ? 1 : 0;
      report.sns[1] = analogRead(1) < threshold ? 1 : 0;
      report.sns[2] = analogRead(2) < threshold ? 1 : 0;
//      Serial.print(report.sns[3]);Serial.print(" | ");Serial.println(analogRead(3));
    }
    if (debug)
    {
      Serial.println("");
    }

    if (~slave)
    {
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
