#include <Wire.h>
#include <SerialCommands.h>

#include "Ultrasound.h"

Ultrasound ultrasound(8, 2, 9, 3, 10, 4);
 
void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print(ultrasound.getDistance(1));
  Serial.print(" ");
  Serial.println(ultrasound.getDistance(2));
  delay(100);
}
