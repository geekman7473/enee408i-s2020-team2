#ifndef Ultrasound_h
#define Ultrasound_h

#include "Arduino.h"

// Header file for Ultrasound.cpp

#define DEFAULT_PULSEWIDTH 50
#define DEFAULT_PULSE_SCHEDULE 20

class Ultrasound
{
  public:
    Ultrasound(int trigPin, int echoPin);
    Ultrasound(int trigPin, int echoPin, int pulseWidth, int pulseSchedule);
    long getDistance();
    long microsecondsToInches(long microseconds);
    long microsecondsToCentimeters(long microseconds);
  private:
    int _trigPin;
    int _echoPin;
    int _pulseWidth
    volatile long _echo_start = 0;
    volatile long _echo_end = 0;
    volatile long _echo_duration = 0;                      // Duration - difference between end and start
    volatile int _trigger_time_count = 0;                  // Count down counter to trigger pulse time
    int _trigger_state = 0;
};

#endif