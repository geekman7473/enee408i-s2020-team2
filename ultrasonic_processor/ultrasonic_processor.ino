#include <Wire.h>
#include <SerialCommands.h>
#include <Arduino.h>

#include "Ultrasound.h"

const uint8_t I2C_ADDRESS = 0x43;

// TODO: Move this to comms library.
const uint8_t REGISTER_DISTANCE_LEFT = 0x01;
const uint8_t REGISTER_DISTANCE_CENTER = 0x02;
const uint8_t REGISTER_DISTANCE_RIGHT = 0x11;

uint8_t opcode;

typedef union float_i2c {
  byte buf[4];
  float fval;
} float_i2c_t;

Ultrasound ultrasound(21, 20, 10, 9, 8, 7);

void setup()
{
  Serial.begin(9600);

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes)
{
  opcode = Wire.read();
}

void requestEvent()
{
  float_i2c_t distance;
  
  switch(opcode) {
   case REGISTER_DISTANCE_LEFT:
     distance.fval = ultrasound.getDistance(1);
     break;

   case REGISTER_DISTANCE_CENTER:
     distance.fval = ultrasound.getDistance(2);
     break;

   case REGISTER_DISTANCE_RIGHT:
     distance.fval = ultrasound.getDistance(3);
     break;
  }

  Wire.write(distance.buf, sizeof(float_i2c_t));
}

void loop()
{
  Serial.print(ultrasound.getDistance(1));
  Serial.print(" ");
  Serial.print(ultrasound.getDistance(2));
  Serial.print(" ");
  Serial.println(ultrasound.getDistance(3));
  delay(1000);
}
