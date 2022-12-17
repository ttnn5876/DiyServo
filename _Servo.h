/*
 DIY Servo library for arduino Uno.
 Works on my arduino, probably won't work on yours.

 https://microcontrollerslab.com/arduino-pwm-tutorial-generate-fix-and-variable-frequency-signal/
 https://github.com/arduino-libraries/Servo/blob/master/src/Servo.h
 https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf

------------------------------------------

  1 - 2 MS
   Pulse
 __________
|          |                            |
|          |                            |
|          |                            |
|          |                            |
|          |                            |
|          |                            |
|          |____________________________|

                18 - 19 MS Downtime

[________________________________________]
                  20 MS Period

------------------------------------------

 Servo angle to freq - 

 1 / 19     MS - Left
 1.5 / 18.5 MS - Middle
 2 / 18     MS - Right

------------------------------------------

*/
#define SREG_IO 0x5F

#define MIN_PULSE_WIDTH 544
#define MAX_PULSE_WIDTH 2400
#define DEFAULT_PULSE_WIDTH 1500

bool timer_active = false;
short timer_indicator = 0;

/************* Servo API Definition *************/
class _Servo {
public:
  _Servo();
  void attach(int pin);
  void attach(int pin, int min, int max);
  void detach();
  void write(int angle);
  void writeMicroseconds(int us);
  int read();
  int readMicroseconds();
  bool attached;

private:
  int current_input_ms;
  int attached_pin;
  int min_pulse_width;
  int max_pulse_width;
};

/************* Implementations *************/

boolean isTimerActive() {
  return timer_active;
}

void activateTimer() {
  // I only work with _timer1 because I have no intention of using more than 2 motors
  TCCR1A = _BV(COM1A1) | _BV(COM1B1);
  TCCR1B = _BV(WGM13) | _BV(CS11); // Phase and frequency correct, 8 prescaler
  ICR1 = 20000;  
  timer_active = true;
}

void disableTimer() {
  // Stop the interrupts
  TIMSK1 &= _BV(OCIE1A);
  timer_active = false;
}

_Servo::_Servo() {
  current_input_ms = DEFAULT_PULSE_WIDTH;
  attached_pin = 0;
  min_pulse_width = MIN_PULSE_WIDTH;
  max_pulse_width = MAX_PULSE_WIDTH;
}

void _Servo::write(int angle) {
  // Validate values
  if (angle < 0) {
    angle = 0;
  } else if (angle > 180) {
    angle = 180;
  }

  return writeMicroseconds(map(angle, 0, 180, min_pulse_width, max_pulse_width));
}

void _Servo::writeMicroseconds(int us) {

  // Validate values
  if (us < min_pulse_width) {
    us = min_pulse_width;
  } else if (us > max_pulse_width) {
    us = max_pulse_width;
  }
  current_input_ms = us;

  // Stop interrupting while we change the setpoint
  cli();

  // Change the PWM compare value
  if (attached_pin == 9) {
    OCR1A = us;
  } else if (attached_pin == 10) {
    OCR1B = us;
  }

  // Resume interrupts
  sei();
}

int _Servo::read() {
  return map(current_input_ms, min_pulse_width, max_pulse_width, 0, 180);
}

int _Servo::readMicroseconds() {
  return current_input_ms;
}

void _Servo::attach(int pin) {
  attach(pin, min_pulse_width, max_pulse_width);
}

void _Servo::attach(int pin, int min, int max) {
  // Validate pin - only 9 and 10 work on the shield
  if (pin != 9 && pin != 10) {
    // The servo won't work :(
    return;
  }

  attached_pin = pin;
  min_pulse_width = min;
  max_pulse_width = max;

  pinMode(attached_pin, OUTPUT);
  attached = true;

  if (timer_active == false) {
    activateTimer();
  }
  // Mark that we use the timer
  timer_indicator |= attached_pin;

  writeMicroseconds(current_input_ms);
}

void _Servo::detach() {
  attached = false;
  
  // Release the timer
  timer_indicator &= attached_pin;

  // 2DO - Figure out how to shut both servos
  if (timer_indicator == 0) {
    disableTimer();
  }

}
