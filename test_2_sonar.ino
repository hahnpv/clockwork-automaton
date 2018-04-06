#include <NewPing.h>
#include <Servo.h>

NewPing sonar1(2, 3, 50); // Sensor 1: trigger pin, echo pin, maximum distance in cm
NewPing sonar2(4,  5, 50); // Sensor 2: same stuff
#define pingSpeed 20        // Ping frequency (in milliseconds), fastest we should ping is about 35ms per sensor

// Max range of 50 cm = 100cm round trip ~ 3 ms. Potential for reflections to mess up second sensor with 20ms duration but likely OK

int longestMillis = 0;
unsigned long int lastMillis;
unsigned long pingTimer1, pingTimer2;

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

sensor left, right;

void setup() {
  Serial.begin(115200);
  pingTimer1 = millis() + pingSpeed; // Sensor 1 fires after 100ms (pingSpeed)
  pingTimer2 = pingTimer1 + (pingSpeed / 2); // Sensor 2 fires 50ms later
}


void loop() {
 if (millis() >= pingTimer1) {
  pingTimer1 += pingSpeed; // Make sensor 1 fire again 100ms later (pingSpeed)
  left.val = sonar1.ping_cm();
  read_sensor(left);
 }
 if (millis() >= pingTimer2) {
   pingTimer2 = pingTimer1 + (pingSpeed / 2); // Make sensor 2 fire again 50ms after sensor 1 fires
   right.val = sonar2.ping_cm();
   read_sensor(right);


   int duration = millis()-lastMillis;
   lastMillis = millis();
   Serial.print(left.val);Serial.print(" | ");Serial.print(right.val);Serial.print(" || ");
   Serial.print(left.per);Serial.print(" | ");Serial.print(right.per);Serial.print(" || ");
   Serial.print(left.ma); Serial.print(" | ");Serial.print(right.ma); Serial.print(" || ");
   Serial.print(duration);Serial.print(" || ");
   
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

 // WITH NO CODE HERE: ~ 0 ms per loop. Longest loop: 12ms @ 10 Hz, 9 @ 20 Hz
/*
 // Do other stuff here, notice how there's no delays in this sketch, so you have processing cycles to do other things :)
 int duration = millis()-lastMillis;
 if (longestMillis < duration)
 {
  longestMillis = duration;
 }
 lastMillis = millis();
  Serial.println(longestMillis); 
 */
}
