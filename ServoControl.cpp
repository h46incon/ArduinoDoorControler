#include "ServoControl.h"

#include <Arduino.h>

const int ServoControl::kDelayForServo = 1000;

ServoControl::ServoControl(int pwm_pin, int enable_pin, int press_anger, int release_anger)
: pwm_pin_(pwm_pin)
, enable_pin_(enable_pin)
, press_anger_(press_anger)
, release_anger_(release_anger)
{
}

void ServoControl::Enable()
{
	pinMode(enable_pin_, OUTPUT);
	digitalWrite(enable_pin_, HIGH);
}

void ServoControl::Disable()
{
	pinMode(enable_pin_, OUTPUT);
	digitalWrite(enable_pin_, LOW);
}

void ServoControl::Init()
{
	pinMode(enable_pin_, OUTPUT);
	// Servo Init
	// Let servo in the release anger
	// Init PWM signal first
	servo_.attach(pwm_pin_);
	servo_.write(release_anger_);

	// Then Enable
	Enable();
	delay(kDelayForServo);
	Disable();
	servo_.detach();
}

void ServoControl::OpenDoor()
{
	servo_.attach(pwm_pin_);
	servo_.write(press_anger_);

	Enable();

	delay(kDelayForServo);
	servo_.write(release_anger_);
	delay(kDelayForServo);

	Disable();
	servo_.detach();
}
