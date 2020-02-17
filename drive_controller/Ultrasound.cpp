#include "Ultrasound.h"
#include <TimerOne.h>

// Class Constructor
Ultrasound::Ultrasound(int trigPin, int echoPin) {
    Ultrasound(trigPin, echoPin, DEFAULT_PULSEWIDTH, DEFAULT_PULSE_SCHEDULE);
}

Ultrasound::Ultrasound(int trigPin, int echoPin, int pulseWidth, int pulseSchedule) {
    _trigPin = trigPin;
    _echoPin = echoPin;
    Timer1.initialize(pulseWidth);              // Initialise timer 1
    Timer1.attachInterrupt(trigger_pulse);      // Attach interrupt to the timer service routine
    attachInterrupt(0, echo_interrupt, CHANGE); // Attach interrupt to the sensor echo input
}

// --------------------------
// trigger_pulse() called every 50 uS to schedule trigger pulses.
// Generates a pulse one timer tick long.
// Minimum trigger pulse width for the HC-SR04 is 10 us. This system
// delivers a 50 uS pulse.
// --------------------------
static void Ultrasound::trigger_pulse(){
    if (!(--_trigger_time_count))  {                                 
        // Time out - Initiate trigger pulse
        _trigger_time_count = _pulse_schedule; // Reload
        _trigger_state = 1;                   // Changing to state 1 initiates a pulse
    }

    switch (_trigger_state) {
    case 0: // Normal state does nothing
        break;

    case 1:                          // Initiate pulse
        digitalWrite(trigPin, HIGH);
        _trigger_state = 2;
        break;

    case 2: // Complete the pulse
    default:
        digitalWrite(trigPin, LOW);
        _trigger_state = 0;                  // and return state to normal 0
        break;
    }
}

// --------------------------
// echo_interrupt() External interrupt from HC-SR04 echo signal. 
// Called every time the echo signal changes state.
//
// Note: this routine does not handle the case where the timer
//       counter overflows which will result in the occassional error.
// --------------------------
static void Ultrasound::echo_interrupt()
{
  switch (digitalRead(echoPin))                     // Test to see if the signal is high or low
  {
    case HIGH:                                      // High so must be the start of the echo pulse
      _echo_end = 0;                                 // Clear the end time
      _echo_start = micros();                        // Save the start time
      break;
      
    case LOW:                                       // Low so must be the end of hte echo pulse
      _echo_end = micros();                          // Save the end time
      _echo_duration = echo_end - echo_start;        // Calculate the pulse duration
      break;
  }
}


long Ultrasound::getDistance() {
    cm = microsecondsToCentimeters(_echo_duration);
    return cm;
}

private double Ultrasound::microsecondsToInches(long microseconds) {
    // According to Parallax's datasheet for the PING))), there are 73.746
    // microseconds per inch (i.e. sound travels at 1130 feet per second).
    // This gives the distance travelled by the ping, outbound and return,
    // so we divide by 2 to get the distance of the obstacle.
    // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
    return ((double) microseconds) / 74.647 / 2.0;
}

private double Ultrasound::microsecondsToCentimeters(long microseconds) {
    // The speed of sound is 340 m/s or 29 microseconds per centimeter.
    // The ping travels out and back, so to find the distance of the object we
    // take half of the distance travelled.
    return ((double) microseconds) / 29.388 / 2.0;
}
