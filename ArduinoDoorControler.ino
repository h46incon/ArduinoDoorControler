#include "Main.h"

#include <Servo.h>

Main main_program;

void setup() {
	main_program.setup();
	delay(1000);
}

void loop() {
	main_program.loop();
}
