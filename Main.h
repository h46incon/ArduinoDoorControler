#ifndef MAIN_HPP_INCLUDE
#define MAIN_HPP_INCLUDE
#include "SleepManager.h"

class Main{
public:
	Main();
	void setup();
	void loop();

private:
	SleepManager sleep_manager_;
};
#endif