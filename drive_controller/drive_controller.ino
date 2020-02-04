//#include <PID_v1.h>
#include <Wire.h>

#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1

#define RIGHT_MOTOR_PWM 5
#define RIGHT_MOTOR_INA 3 
#define RIGHT_MOTOR_INB 2
#define LEFT_MOTOR_PWM 9
#define LEFT_MOTOR_INA 7
#define LEFT_MOTOR_INB 8

#define TICKS_PER_REV 3200
#define M_PI 3.14159265358979323846
#define WHEEL_DIAMETER 6.0

// TODO: move this to comms library
typedef union int32_i2c {
  byte buf[4];
  int32_t ival;
} int32_i2c_t;

typedef union float_i2c {
  byte buf[4];
  float fval;
} float_i2c_t;

const uint8_t REGISTER_COUNT_LEFT = 0x01;
const uint8_t REGISTER_COUNT_RIGHT = 0x02;
const uint8_t REGISTER_VELOCITY_LEFT = 0x11;
const uint8_t REGISTER_VELOCITY_RIGHT = 0x12;
const uint8_t REGISTER_RESET_LEFT = 0x21;
const uint8_t REGISTER_RESET_RIGHT = 0x22;
const uint8_t REGISTER_RESET_BOTH = 0x23;

const uint8_t ENCODER_SLAVE_ADDRESS = 0x42;

double leftMotorSetpoint, rightMotorSetpoint, leftMotorSpeed, rightMotorSpeed;
int32_t leftMotorCount, rightMotorCount;
int32_t leftMotorCPS, rightMotorCPS;
uint8_t leftMotorPWM, rghtMotorPWM;
int leftKp = 2, rightKp = 2, leftKi = 5, rightKi = 5, leftKd = 0, rightKd = 0;

//PID leftMotorPID(&leftMotorSpeed, &leftMotorPWM, &leftMotorSetpoint, leftKp, leftKi, leftKd, DIRECT);
//PID rightMotorPID(&rightMotorSpeed, &rightMotorPWM, &rightMotorSetpoint, rightKp, rightKi, rightKd, DIRECT);

void setup()
{
  leftMotorSetpoint = 0;
  rightMotorSetpoint = 0;

  Wire.begin();
  Serial.begin(9600);
  //leftMotorPID.SetMode(AUTOMATIC);
  //rightMotorPID.SetMode(AUTOMATIC);
  pinMode(RIGHT_MOTOR_INA, OUTPUT);
  pinMode(RIGHT_MOTOR_INB, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(LEFT_MOTOR_INA, OUTPUT);
  pinMode(LEFT_MOTOR_INB, OUTPUT);
  pinMode(LEFT_MOTOR_INB, OUTPUT);

  digitalWrite(LEFT_MOTOR_INA, HIGH);
  digitalWrite(LEFT_MOTOR_INB, LOW);
  analogWrite(LEFT_MOTOR_PWM, 100);
  digitalWrite(RIGHT_MOTOR_INA, HIGH);
  digitalWrite(RIGHT_MOTOR_INB, LOW);
  analogWrite(RIGHT_MOTOR_PWM, 100);
  delay(5000);
  analogWrite(LEFT_MOTOR_PWM, 0);
  analogWrite(RIGHT_MOTOR_PWM, 0);
}

void loop()
{
  read_counts(RIGHT_MOTOR, &rightMotorCPS);
  read_counts(LEFT_MOTOR, &rightMotorCPS);

  leftMotorSpeed = ((double) leftMotorCPS/TICKS_PER_REV) * M_PI * WHEEL_DIAMETER;
  rightMotorSpeed = ((double) rightMotorCPS/TICKS_PER_REV) * M_PI * WHEEL_DIAMETER;
  
  //leftMotorPID.Compute();
  //rightMotorPID.Compute();

  //analogWrite(LEFT_MOTOR_PWM, leftMotorPWM);
  //analogWrite(RIGHT_MOTOR_PWM, rightMotorPWM);
  Serial.print(leftMotorSpeed);
  Serial.print(" ");
  Serial.println(rightMotorSpeed);
  delay(100);
}

// TODO: move this to comms library
// TODO: Maybe this shouldn't be void? Unsure.
void read_counts(int motor, int32_t* out)
{
  uint8_t reg = motor ? REGISTER_COUNT_RIGHT : REGISTER_COUNT_LEFT;
  Wire.beginTransmission(ENCODER_SLAVE_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ENCODER_SLAVE_ADDRESS, (uint8_t) 4);
  int32_i2c_t response;
  for(int i = 0; i < 4; i++)
  {
    response.buf[i] = Wire.read();
  }
  *out = response.ival;
}

void read_speeds(int motor, int32_t* out)
{
  uint8_t reg = motor ? REGISTER_VELOCITY_RIGHT : REGISTER_VELOCITY_LEFT;
  Wire.beginTransmission(ENCODER_SLAVE_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ENCODER_SLAVE_ADDRESS, (uint8_t) 4);
  int32_i2c_t response;
  for(int i = 0; i < 4; i++)
  {
    response.buf[i] = Wire.read();
  }
  *out = response.ival;
}

void reset_counts(int motor, int32_t reset_value)
{
  int32_i2c_t payload;
  payload.ival = reset_value;
  uint8_t reg = motor ? REGISTER_RESET_RIGHT : REGISTER_RESET_LEFT;
  Wire.beginTransmission(ENCODER_SLAVE_ADDRESS);
  Wire.write(reg);
  for(int i = 0; i < 4; i++)
  {
    Wire.write(payload.buf[i]);
  }
  Wire.endTransmission();
}
