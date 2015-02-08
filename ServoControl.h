#ifndef SERVO_CONTROL_HPP_INCLUDE
#define SERVO_CONTROL_HPP_INCLUDE

#include <Servo.h>

class ServoControl{
public:
	ServoControl(int pwm_pin, int press_anger, int release_anger);
	void Init();
	void OpenDoor( );

private:
	void Disable();
	Servo servo_;
	int pwm_pin_;
	int press_anger_;
	int release_anger_;
	static const int kDelayForServo;
};
#endif