#include<Wire.h>
#include "Encoder.h"

const uint8_t I2C_ADDRESS = 0x042;
const uint8_t ENC_1_PIN1 = 17;
const uint8_t ENC_1_PIN2 = 7;
const uint8_t ENC_2_PIN1 = 18;
const uint8_t ENC_2_PIN2 = 6;

const uint8_t REGISTER_COUNT = 0x0;
const uint8_t REGISTER_VELOCITY = 0x1;
const uint8_t REGISTER_RESET = 0x2;
uint8_t opcode;

Encoder encoder_left(ENC_1_PIN1, ENC_1_PIN2);
Encoder encoder_right(ENC_2_PIN1, ENC_2_PIN2);

typedef union reading {
  byte b[4];
  float fval;
} reading;
reading rpm;

volatile int test = 0;

void setup() {
  //Wire.begin(I2C_ADDRESS);
  //Wire.onRequest(requestEvent);
  //Wire.onReceive(receiveEvent);
  encoder_left.write(0);
  encoder_right.write(0);
  Serial.begin(9600);
  //speed_setup();
}

void receiveEvent(int bytes) {
  opcode = Wire.read();

  if(bytes > 1)
  {
    if(opcode == REGISTER_RESET)
    {
      encoder_left.write(0);
    }
  }
}

void requestEvent() {
  switch(opcode)
  {
   case REGISTER_COUNT:
     int count = encoder_left.read();
     Wire.write(count);
  }
}

void loop(){
  // Spin forever
  Serial.print(encoder_left.read());
  Serial.print(" ");
  Serial.println(encoder_right.read());
  //Serial.print(test);
}

// Speed timer setup:
void speed_setup()
{
  // Write TOP:
  TCA0.SINGLE.PER = F_CPU / 1000 - 1;   // Overflow after 1 ms
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;   // Enable overflow interrupt
	TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;   // Start without prescaler, at full CPU clock Speed
}

ISR(TCA0_OVF_vect)
{
  test++;
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_OVF_bm;
}
