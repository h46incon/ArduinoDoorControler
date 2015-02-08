#include "ServoControl.h"

#include <Arduino.h>

const int ServoControl::kDelayForServo = 1000;

ServoControl::ServoControl(int pwm_pin, int press_anger, int release_anger)
: pwm_pin_(pwm_pin), press_anger_(press_anger), release_anger_(release_anger)
{
}

void ServoControl::Disable()
{
	// TODO:

}

void ServoControl::Init()
{
	// Servo Init
	// Let servo in the release anger
	servo_.attach(pwm_pin_);
	servo_.write(release_anger_);
	delay(kDelayForServo);
	// detach for power saving
	// VS-2 servo will stop working if PWM is low.
	Disable();
}

void ServoControl::OpenDoor()
{
	servo_.attach(pwm_pin_);
	servo_.write(press_anger_);
	delay(kDelayForServo);
	servo_.write(release_anger_);
	delay(kDelayForServo);

	Disable();
}

