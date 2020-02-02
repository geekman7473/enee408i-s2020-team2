#include<Wire.h>
#include<Encoder.h>

const uint8_t I2C_ADDRESS = 0x042;
const uint8_t ENC_PIN1 = 5;
const uint8_t ENC_PIN2 = 6;

const uint8_t REGISTER_COUNT = 0x0;
const uint8_t REGISTER_VELOCITY = 0x1;
const uint8_t REGISTER_RESET = 0x2;
uint8_t opcode;
Encoder encoder(ENC_PIN1, ENC_PIN2);
typedef union reading {
  byte b[4];
  float fval;
} reading;
reading rpm;

volatile int test = 0;

void setup() {
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  speed_setup();
}

void receiveEvent(int bytes) {
  opcode = Wire.read();

  if(bytes > 1)
  {
    if(opcode == REGISTER_RESET)
    {
      encoder.write(0);
    }
  }
}

void requestEvent() {
  switch(opcode)
  {
   case REGISTER_COUNT:
     int count = encoder.read();
     Wire.write(count);
  }
}

void loop(){
  // Spin forever
  Serial.println(encoder.get());
  Serial.println(test);
  delay(1000);
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

