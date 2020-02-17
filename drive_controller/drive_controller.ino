#include <PID_v1.h>
#include <Wire.h>
#include <SerialCommands.h>
#include "Ultrasound.h"

// TODO: refactor this into an enum
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1

#define RIGHT_MOTOR_PWM 5
#define RIGHT_MOTOR_INA 3 
#define RIGHT_MOTOR_INB 2
#define LEFT_MOTOR_PWM 9
#define LEFT_MOTOR_INA 8
#define LEFT_MOTOR_INB 7

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
double leftMotorPWM, rightMotorPWM;
double leftKp = 2, rightKp = 2, leftKi = 5, rightKi = 5, leftKd = 0, rightKd = 0;

PID leftMotorPID(&leftMotorSpeed, &leftMotorPWM, &leftMotorSetpoint, leftKp, leftKi, leftKd, DIRECT);
PID rightMotorPID(&rightMotorSpeed, &rightMotorPWM, &rightMotorSetpoint, rightKp, rightKi, rightKd, DIRECT);


// TODO maybe move to comms library
char serial_command_buffer[64];
SerialCommands serial_commands(&Serial, serial_command_buffer, sizeof(serial_command_buffer), "\r\n", " ");
void recv_speed(SerialCommands* sender)
{
  char *arg;

  arg = sender->Next();
  if (arg != NULL) {
    leftMotorSetpoint = atof(arg);
  }
  else {// TODO: Add error handling
  }

  arg = sender->Next();
  if (arg != NULL) {
    rightMotorSetpoint = atof(arg);
  }
  else {//TODO: Add error handling
  }
}
SerialCommand cmd_recv_speed("SPEED", recv_speed);

// TODO Move to actuator state library
void set_speed(int motor, double speed)
{
  uint8_t pinA = motor ? RIGHT_MOTOR_INA : LEFT_MOTOR_INA;
  uint8_t pinB = motor ? RIGHT_MOTOR_INB : LEFT_MOTOR_INB;
  uint8_t pinPWM = motor ? RIGHT_MOTOR_PWM : LEFT_MOTOR_PWM;

  if(speed > 0)
  {
    digitalWrite(pinA, HIGH);
    digitalWrite(pinB, LOW);
  }
  else if(speed < 0)
  {
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, HIGH);
  }
  else
  {
    // We idle here instead of braking for the benifit of PW
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  }
  analogWrite(pinPWM, speed);
}

void setup()
{
  leftMotorSetpoint = 0;
  rightMotorSetpoint = 0;

  Wire.begin();
  Serial.begin(9600);
  leftMotorPID.SetMode(AUTOMATIC);
  rightMotorPID.SetMode(AUTOMATIC);
  pinMode(RIGHT_MOTOR_INA, OUTPUT);
  pinMode(RIGHT_MOTOR_INB, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(LEFT_MOTOR_INA, OUTPUT);
  pinMode(LEFT_MOTOR_INB, OUTPUT);
  pinMode(LEFT_MOTOR_INB, OUTPUT);

  leftMotorSetpoint = 10;
  rightMotorSetpoint = 10;
  digitalWrite(LEFT_MOTOR_INA, HIGH);
  digitalWrite(LEFT_MOTOR_INB, LOW);
  digitalWrite(RIGHT_MOTOR_INA, HIGH);
  digitalWrite(RIGHT_MOTOR_INB, LOW);

  serial_commands.AddCommand(&cmd_recv_speed);
}

void loop()
{
  serial_commands.ReadSerial();
  read_speeds(RIGHT_MOTOR, &rightMotorCPS);
  read_speeds(LEFT_MOTOR, &leftMotorCPS);

  leftMotorSpeed = ((double) leftMotorCPS/TICKS_PER_REV) * M_PI * WHEEL_DIAMETER;
  rightMotorSpeed = ((double) rightMotorCPS/TICKS_PER_REV) * M_PI * WHEEL_DIAMETER;
  
  leftMotorPID.Compute();
  rightMotorPID.Compute();
  
  set_speed(RIGHT_MOTOR, rightMotorPWM);
  set_speed(LEFT_MOTOR, leftMotorPWM);
  Serial.print(leftMotorPWM);
  Serial.print(" ");
  Serial.print(rightMotorPWM);
  Serial.print(" ");
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
