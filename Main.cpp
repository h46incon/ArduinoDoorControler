#include "Main.h"

#include <Arduino.h>
#include <Servo.h>
#include <avr/sleep.h>

int release_anger = 60;
int press_anger = 10;
int servo_pin = 9;
int led_pin = 13;    // LED is used to stands for running or sleeping state
int bt_enable_pin = 7;
int bt_key_pin = 6;
int bt_state_pin = 2;
int bt_IRQn = 0;    // IRQn for pin2
unsigned long bt_baud = 9600;

// Bluetooth will keep connected state for 3 minutes. 
// If this connection could not finish work in 3 minutes, then means some errors have happened.
// Then need to restart the bluetooth device.
unsigned long keep_wake_time = 1000UL * 180UL;
unsigned long wake_time = 0;


Main::Main() 
: sleep_manager_(bt_state_pin, bt_IRQn, led_pin)
, servo_control_(servo_pin, press_anger, release_anger)
, bt_manager_(bt_enable_pin, bt_key_pin)
{

}

void Main::setup()
{
	// LED Setting
	pinMode(led_pin, OUTPUT);
	digitalWrite(led_pin, HIGH);

	sleep_manager_.Init();
	servo_control_.Init();
	bt_manager_.Init(bt_baud);

	wake_time = millis();
	sleep_manager_.TryEnterSleep();
}

void Main::loop()
{
	while (Serial.available() > 0)
	{
		int c = Serial.read();
		switch (c){
			// Shake hand
		case 0x69:
			Serial.write(0x96);
			break;
			// Opendoor
		case 0x38:
			Serial.write(0x83);
			servo_control_.OpenDoor();
			break;
		}
	}

	// try enter sleep
	if (sleep_manager_.TryEnterSleep() == false){
		// Fail to enter sleep means bluetooth is still in contected state
		// Check if it is reach the timeout
		if (millis() - wake_time > keep_wake_time){    // Use subtraction will get correct answer even if millis() is overflowed.
			bt_manager_.Reset();
			wake_time = millis();
		}
	}
	else {
		wake_time = millis();
		//Serial.println("Wake up from sleeping");
		// hand shake message may missing.
		// openDoor();
	}

}
