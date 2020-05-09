#include<Wire.h>
#include "Encoder.h"
#include<Arduino.h>

const uint8_t I2C_ADDRESS = 0x42;
const uint8_t ENC_1_PIN1 = 4;
const uint8_t ENC_1_PIN2 = 5;
const uint8_t ENC_2_PIN1 = 6;
const uint8_t ENC_2_PIN2 = 7;

// TODO: Move this to comms library.
const uint8_t REGISTER_COUNT_LEFT = 0x01;
const uint8_t REGISTER_COUNT_RIGHT = 0x02;
const uint8_t REGISTER_VELOCITY_LEFT = 0x11;
const uint8_t REGISTER_VELOCITY_RIGHT = 0x12;
const uint8_t REGISTER_RESET_LEFT = 0x21;
const uint8_t REGISTER_RESET_RIGHT = 0x22;
const uint8_t REGISTER_RESET_BOTH = 0x23;

uint8_t opcode;

Encoder encoder_right(ENC_1_PIN1, ENC_1_PIN2);
Encoder encoder_left(ENC_2_PIN2, ENC_2_PIN1);

typedef union int32_i2c {
  byte buf[4];
  int32_t ival;
} int32_i2c_t;

typedef union float_i2c {
  byte buf[4];
  float fval;
} float_i2c_t;

volatile int32_t prev_left;
volatile int32_t prev_right;

volatile int32_i2c_t speed_left;
volatile int32_i2c_t speed_right;

volatile int test = 0;

void setup()
{
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  encoder_left.write(0);
  encoder_right.write(0);
  prev_left = 0;
  prev_right = 0;
  Serial.begin(9600);
  speed_setup();
}

// TODO: move this to comms library
void receiveEvent(int bytes)
{
  opcode = Wire.read();

  if(bytes > 1)
  {
    int32_i2c_t val;
    switch(opcode)
    {
     case REGISTER_RESET_LEFT:
       for(int i = 0; i < 4; i++)
       {
         val.buf[i] = Wire.read();
       }
       encoder_left.write(val.ival);
       break;

     case REGISTER_RESET_RIGHT:
       for(int i = 0; i < 4; i++)
       {
         val.buf[i] = Wire.read();
       }
       encoder_right.write(val.ival);
       break;

     case REGISTER_RESET_BOTH:
       for(int i = 0; i < 4; i++)
       {
         val.buf[i] = Wire.read();
       }
       encoder_left.write(val.ival);
       encoder_right.write(val.ival);
       break;
    }
  }
}

// TODO: move this to comms library
void requestEvent()
{
  int32_i2c_t count;
  
  switch(opcode)
  {
   case REGISTER_COUNT_LEFT:
     count.ival = encoder_left.read();
     Wire.write(count.buf, sizeof(int32_i2c_t));
     break;

   case REGISTER_COUNT_RIGHT:
     count.ival = encoder_right.read();
     Wire.write(count.buf, sizeof(int32_i2c_t));
     break;

   case REGISTER_VELOCITY_LEFT:
     Wire.write((byte*) speed_left.buf, sizeof(int32_i2c_t));
     break;

   case REGISTER_VELOCITY_RIGHT:
     Wire.write((byte*) speed_right.buf, sizeof(int32_i2c_t));
     break;
  }
}

void loop()
{
  // Spin forever

  //Serial.print(encoder_left.read());
  //Serial.print(" ");
  //Serial.println(encoder_right.read());
  //Serial.print(test);
  /*

  delay(10);
  */

  //Serial.print("Left speed: ");Serial.print(speed_left.ival);
  //Serial.print("Right speed: ");Serial.println(speed_right.ival);
  
  delay(100);
}

// Speed timer setup:
void speed_setup()
{
  // Write TOP:
  TCB2.CCMP =  2500; // Overflow after 10 ms using the TCA clk 
  TCB2.INTCTRL = TCB_CAPT_bm;
  TCB2.CTRLB = TCB_CNTMODE_INT_gc; // Setup timer into periodic interupt mode
  TCB2.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc; // Enable counter Using the TCA clock
}

ISR(TCB2_INT_vect)
{
  int32_t curr_left = encoder_left.read();
  int32_t curr_right = encoder_right.read();
  // Measure ticks per second:
  speed_left.ival = (curr_left - prev_left) * 100;
  speed_right.ival= (curr_right - prev_right) * 100;
  
  prev_left = curr_left;
  prev_right = curr_right;
  
  TCB2.INTFLAGS = TCB_CAPT_bm;
}
