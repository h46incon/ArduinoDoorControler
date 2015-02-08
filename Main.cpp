#include "Main.h"

#include <Arduino.h>
#include <Servo.h>
#include <avr/sleep.h>

Servo servo;
int release_anger = 60;
int press_anger = 10;
int servo_pin = 9;
int led_pin = 13;    // LED is used to stands for running or sleeping state
int bt_enable_pin = 7;
int bt_state_pin = 2;
int bt_IRQn = 0;    // IRQn for pin2

// Bluetooth will keep connected state for 3 minutes. 
// If this connection could not finish work in 3 minutes, then means some errors have happened.
// Then need to restart the bluetooth device.
unsigned long keep_wake_time = 1000UL * 10UL;
unsigned long wake_time = 0;


Main::Main() : sleep_manager_(bt_state_pin, bt_IRQn, led_pin)
{

}

void detachServo()
{
	servo.detach();
	//VS-2 servo will stop working if PWM is low.
	digitalWrite(servo_pin, LOW);
}

void btSetup()
{
	// BlueTooth Setting
	pinMode(bt_enable_pin, OUTPUT);
	pinMode(bt_state_pin, INPUT_PULLUP);
	// Enable uart
	Serial.begin(9600);

	digitalWrite(bt_enable_pin, HIGH);
}

void servoSetup()
{
	// Servo Init
	// Let servo in the release anger
	servo.attach(servo_pin);
	servo.write(release_anger);
	delay(2000);
	// detach for power saving
	// VS-2 servo will stop working if PWM is low.
	detachServo();
}

void openDoor()
{
	servo.attach(servo_pin);
	servo.write(press_anger);
	delay(2000);
	servo.write(release_anger);
	delay(2000);
	detachServo();
}


void Main::setup()
{
	// LED Setting
	pinMode(led_pin, OUTPUT);
	digitalWrite(led_pin, HIGH);

	btSetup();
	servoSetup();

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
			openDoor();
			break;
		}
	}

	// try enter sleep
	if (sleep_manager_.TryEnterSleep() == false){
		// Fail to enter sleep means bluetooth is still in contected state
		// Check if it is reach the timeout
		if (millis() - wake_time > keep_wake_time){    // Use subtraction will get correct answer even if millis() is overflowed.
			// TODO: use AT command to restart Bluetooth
			// Re enable Blue tooth, and sleep
			digitalWrite(bt_enable_pin, LOW);
			delay(2000);
			digitalWrite(bt_enable_pin, HIGH);
			// recode wake_time
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
