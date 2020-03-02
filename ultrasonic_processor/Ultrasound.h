#ifndef Ultrasound_h
#define Ultrasound_h

#include "Arduino.h"

// Header file for Ultrasound.cpp

#define DEFAULT_PULSEWIDTH 50
#define DEFAULT_PULSE_SCHEDULE 100        

void timer_setup();

class Ultrasound
{
  public:
    Ultrasound(int trigPin1, int echoPin1, int trigPin2, int echoPin2, int trigPin3, int echoPin3);
    Ultrasound(int trigPin1, int echoPin1, int trigPin2, int echoPin2, int trigPin3, int echoPin3, int pulseWidth, int pulseSchedule);
    double getDistance(int ultrasonicNum);
    double microsecondsToInches(long microseconds);
    double microsecondsToCentimeters(long microseconds);
};

#endif
