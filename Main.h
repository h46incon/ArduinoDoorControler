#ifndef MAIN_HPP_INCLUDE
#define MAIN_HPP_INCLUDE

#include "SleepManager.h"
#include "ServoControl.h"
#include "BTManager.h"

class Main{
public:
	Main();
	void setup();
	void loop();

private:
	SleepManager sleep_manager_;
	ServoControl servo_control_;
	BTManager bt_manager_;
};
#endif