// Replacement remote control for AIWA NSX-AV65 using an Arduino Uno.
// Got the IR-led from an old remote control.

/*
  Control TV using IR Library
  IR LED must use Pin #3
  DroneBot Workshop 2017
  http://dronebotworkshop.com
*/


#include <boarddefs.h>
#include <IRremoteInt.h>
#include <IRremote.h>


int const potPin = A0;
int potval;
 
IRsend irsend;
 
void setup() {
  Serial.begin(9600);
  pinMode(10, OUTPUT); // LED indicating 'vol down'
  pinMode(12, OUTPUT); // LED indicating 'vol up'
}

/* 
  http://lirc.sourceforge.net/remotes/aiwa/RC-T501
  KEY_POWER                0x0000000000007F80        #  POWER
  KEY_VOLUMEDOWN           0x00000000000046B9        #  VOL-
  KEY_VOLUMEUP             0x00000000000026D9        #  VOL+
*/

 
void loop() {
  potval = analogRead(potPin);  // read value of potentiometer

  if (potval == 1023) {
     Serial.println("vol up");
     irsend.sendAiwaRCT501(0x26D9); // VOL+
     digitalWrite(12, HIGH);  // turn on red LED
     delay(500);
     }
  
  if (potval == 0) {
     Serial.println("vol down");
     irsend.sendAiwaRCT501(0x46B9); // VOL-
     digitalWrite(10, HIGH); // turn on blue LED
     delay(500);
     }
   else {
    digitalWrite(10, LOW);
    digitalWrite(12, LOW);
    }
  //delay(200);
} 
