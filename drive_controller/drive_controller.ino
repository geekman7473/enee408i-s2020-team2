#include <PID_v1.h>
#include <Wire.h>
#include <SerialCommands.h>

// TODO: refactor this into an enum
#define LEFT_MOTOR 0
#define RIGHT_MOTOR 1

#define LEFT_MOTOR_PWM 3
#define LEFT_MOTOR_INA 2 
#define LEFT_MOTOR_INB 4
#define RIGHT_MOTOR_PWM 9
#define RIGHT_MOTOR_INA 7
#define RIGHT_MOTOR_INB 8

#define TICKS_PER_REV_RIGHT 3200
#define TICKS_PER_REV_LEFT  4480
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

void proximity_override();

const uint8_t REGISTER_COUNT_LEFT = 0x01;
const uint8_t REGISTER_COUNT_RIGHT = 0x02;
const uint8_t REGISTER_VELOCITY_LEFT = 0x11;
const uint8_t REGISTER_VELOCITY_RIGHT = 0x12;
const uint8_t REGISTER_RESET_LEFT = 0x21;
const uint8_t REGISTER_RESET_RIGHT = 0x22;
const uint8_t REGISTER_RESET_BOTH = 0x23;
const uint8_t REGISTER_DISTANCE_LEFT = 0x01;
const uint8_t REGISTER_DISTANCE_CENTER = 0x02;
const uint8_t REGISTER_DISTANCE_RIGHT = 0x11;

const uint8_t ENCODER_SLAVE_ADDRESS = 0x42;
const uint8_t ULTRASONIC_SLAVE_ADDRESS = 0x43;

double leftMotorSetpoint, rightMotorSetpoint, leftMotorSpeed, rightMotorSpeed;
int32_t leftMotorCount, rightMotorCount;
int32_t leftMotorCPS, rightMotorCPS;
double leftMotorPWM, rightMotorPWM;
double leftKp = 3, rightKp = 3, leftKi = 7, rightKi = 7, leftKd = 0, rightKd = 0;

PID leftMotorPID(&leftMotorSpeed, &leftMotorPWM, &leftMotorSetpoint, leftKp, leftKi, leftKd, DIRECT);
PID rightMotorPID(&rightMotorSpeed, &rightMotorPWM, &rightMotorSetpoint, rightKp, rightKi, rightKd, DIRECT);


// TODO maybe move to comms library
char serial_command_buffer[64];
SerialCommands serial_commands(&Serial, serial_command_buffer, sizeof(serial_command_buffer), "\r\n", " ");
void recv_speed(SerialCommands* sender)
{
  char *arg;

  sender->GetSerial()->println("foo");
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
  analogWrite(pinPWM, abs(speed));
}

void setup()
{
  leftMotorSetpoint = 0;
  rightMotorSetpoint = 0;

  Wire.begin();
  Serial.begin(9600);
  
  leftMotorPID.SetMode(AUTOMATIC);
  rightMotorPID.SetMode(AUTOMATIC);
  leftMotorPID.SetOutputLimits(-255, 255);
  rightMotorPID.SetOutputLimits(-255, 255);
  
  pinMode(RIGHT_MOTOR_INA, OUTPUT);
  pinMode(RIGHT_MOTOR_INB, OUTPUT);
  pinMode(RIGHT_MOTOR_PWM, OUTPUT);
  pinMode(LEFT_MOTOR_INA, OUTPUT);
  pinMode(LEFT_MOTOR_INB, OUTPUT);
  pinMode(LEFT_MOTOR_INB, OUTPUT);

  leftMotorSetpoint = 7;
  rightMotorSetpoint = 7;
  digitalWrite(LEFT_MOTOR_INA, HIGH);
  digitalWrite(LEFT_MOTOR_INB, LOW);
  digitalWrite(RIGHT_MOTOR_INA, HIGH);
  digitalWrite(RIGHT_MOTOR_INB, LOW);

  serial_commands.AddCommand(&cmd_recv_speed);
}

void loop()
{
  //proximity_override();
  
  leftMotorSetpoint = 15;
  rightMotorSetpoint = 15;
  
  serial_commands.ReadSerial();
  read_speeds(RIGHT_MOTOR, &rightMotorCPS);
  read_speeds(LEFT_MOTOR, &leftMotorCPS);

  leftMotorSpeed = ((double) leftMotorCPS/TICKS_PER_REV_LEFT) * M_PI * WHEEL_DIAMETER;
  rightMotorSpeed = ((double) rightMotorCPS/TICKS_PER_REV_RIGHT) * M_PI * WHEEL_DIAMETER;
  
  leftMotorPID.Compute();
  rightMotorPID.Compute();
  
  set_speed(RIGHT_MOTOR, rightMotorPWM);
  set_speed(LEFT_MOTOR, leftMotorPWM);
  
  Serial.print(leftMotorCPS);
  Serial.print(" ");
  Serial.print(rightMotorCPS);
  Serial.print(" ");
  Serial.print(leftMotorSpeed);
  Serial.print(" ");
  Serial.println(rightMotorSpeed);
  delay(100);
}

void proximity_override()
{
  float left_distance, center_distance, right_distance;
  read_distance(1, &left_distance);
  read_distance(2, &center_distance);
  read_distance(3, &right_distance);
  int left_prox = (left_distance < 8.0);
  int center_prox = (center_distance < 8.0);
  int right_prox = (right_distance < 8.0);

  double right_speed_1, left_speed_1, right_speed_2, left_speed_2;
  int delay_length_1, delay_length_2;
  if (left_prox && center_prox && right_prox){
    right_speed_1 = -20;
    left_speed_1 = -20;
    delay_length_1 = 1000;
    right_speed_2 = -20;
    left_speed_2 = 20;
    delay_length_2 = 1000;
  } else if (left_prox && center_prox && !right_prox){
    right_speed_1 = 20;
    left_speed_1 = -20;
    delay_length_1 = 500;
    right_speed_2 = 0;
    left_speed_2 = 0;
    delay_length_2 = 0;
  } else if (left_prox && !center_prox && right_prox){
    right_speed_1 = -20;
    left_speed_1 = -20;
    delay_length_1 = 800;
    right_speed_2 = -20;
    left_speed_2 = 20;
    delay_length_2 = 500;
  } else if (!left_prox && center_prox && right_prox){
    right_speed_1 = -20;
    left_speed_1 = 20;
    delay_length_1 = 500;
    right_speed_2 = 0;
    left_speed_2 = 0;
    delay_length_2 = 0;
  } else if (left_prox && !center_prox && !right_prox){
    right_speed_1 = -20;
    left_speed_1 = -20;
    delay_length_1 = 800;
    right_speed_2 = 20;
    left_speed_2 = -20;
    delay_length_2 = 800;
  } else if (!left_prox && center_prox && !right_prox){
    right_speed_1 = -20;
    left_speed_1 = -20;
    delay_length_1 = 800;
    right_speed_2 = 20;
    left_speed_2 = -20;
    delay_length_2 = 800;
  } else if (!left_prox && !center_prox && right_prox){
    right_speed_1 = -20;
    left_speed_1 = -20;
    delay_length_1 = 800;
    right_speed_2 = 20;
    left_speed_2 = -20;
    delay_length_2 = 800;
  }

  if (left_prox || center_prox || right_prox) {
    leftMotorPID.SetMode(DIRECT);
    rightMotorPID.SetMode(DIRECT);

    Serial.print(right_speed_1);
    Serial.print(" ");
    Serial.print(left_speed_1);
    Serial.print(" ");
    Serial.print(right_speed_2);
    Serial.print(" ");
    Serial.println(left_speed_2);
    set_speed(RIGHT_MOTOR, right_speed_1);
    set_speed(LEFT_MOTOR, left_speed_1);
    delay(delay_length_1);
    set_speed(RIGHT_MOTOR, right_speed_2);
    set_speed(LEFT_MOTOR, left_speed_2);
    delay(delay_length_2);

    leftMotorPID.SetMode(AUTOMATIC);
    rightMotorPID.SetMode(AUTOMATIC);
  }
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

void read_distance(int sensor, float* distance_cm)
{
  uint8_t reg = sensor == 1 ? REGISTER_DISTANCE_LEFT : (sensor == 2 ? REGISTER_DISTANCE_CENTER : REGISTER_DISTANCE_RIGHT);
  Wire.beginTransmission(ULTRASONIC_SLAVE_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(ULTRASONIC_SLAVE_ADDRESS, (uint8_t) 4);
  float_i2c_t response;
  for(int i = 0; i < 4; i++)
  {
    response.buf[i] = Wire.read();
  }
  *distance_cm = response.fval;
}
