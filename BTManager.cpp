#include "BTManager.h"

#include <Arduino.h>
BTManager::BTManager(int enable_pin, int key_pin)
: enable_pin_(enable_pin), key_pin_(key_pin)
{

}

void BTManager::Init(unsigned long baud)
{
	// BlueTooth Setting
	pinMode(enable_pin_, OUTPUT);
	// Enable uart
	Serial.begin(baud);

	digitalWrite(enable_pin_, HIGH);
}

void BTManager::Reset()
{
	// TODO: use AT command to restart Bluetooth
	// Re enable Blue tooth, and sleep
	digitalWrite(enable_pin_, LOW);
	delay(2000);
	digitalWrite(enable_pin_, HIGH);
	// recode wake_time
}
