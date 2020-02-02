#include <PID_v1.h>

#define LEFT_MOTOR_PWM 6
#define RIGHT_MOTOR_PWM 7

double leftMotorSetpoint, rightMotorSetpoint, leftMotorSpeed, rightMotorSpeed;
uint8_t leftMotorPWM, rghtMotorPWM;
int leftKp = 2, rightKp = 2, leftKi = 5, rightKi = 5, leftKd = 0, rightKd = 0;

PID leftMotorPID(&leftMotorSpeed, &leftMotorPWM, &leftMotorSetpoint, leftKp, leftKi, leftKd, DIRECT);
PID rightMotorPID(&rightMotorSpeed, &rightMotorPWM, &rightMotorSetpoint, rightKp, rightKi, rightKd, DIRECT);

void setup() {
  leftMotorSetpoint = 0;
  rightMotorSetpoint = 0;

  leftMotorPID.SetMode(AUTOMATIC);
  rightMotorPID.SetMode(AUTOMATIC);
}

void loop() {
  leftMotorSpeed = 0; //TODO: Read from I2C
  rightMotorSpeed = 0; //TODO: Read from I2C
  
  leftMotorPID.Compute();
  rightMotorPID.Compute();

  analogWrite(LEFT_MOTOR_PWM, leftMotorPWM);
  analogWrite(RIGHT_MOTOR_PWM, rightMotorPWM);
}
