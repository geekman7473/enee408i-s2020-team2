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

typedef union int32_i2c {
  byte buf[4];
  int32_t ival;
} int32_i2c_t;

typedef union float_i2c {
  byte buf[4];
  float fval;
} float_i2c_t;

Ultrasound ultrasound(20, 21, 9, 3, 10, 4);

void setup() {
  Serial.begin(9600);

  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop() {
  Serial.print(ultrasound.getDistance(1));
  Serial.print(" ");
  Serial.print(ultrasound.getDistance(2));
  Serial.print(" ");
  Serial.println(ultrasound.getDistance(3))
  delay(1000);
}
