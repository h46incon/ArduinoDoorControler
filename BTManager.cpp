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
	pinMode(key_pin_, OUTPUT);

	digitalWrite(enable_pin_, LOW);
	EnterCommunicationMode();	
	digitalWrite(enable_pin_, HIGH);

	// Enable uart
	Serial.begin(baud);
}

void BTManager::Reset()
{
	if (EnterATMode()){
		Serial.print("AT+RESET\r\n");
	} else{
		// Hard reset
		digitalWrite(enable_pin_, LOW);
		delay(2000);
		digitalWrite(enable_pin_, HIGH);
	}

	EnterCommunicationMode();
}

bool BTManager::GetMac(unsigned char buffer[kMacAddrSize])
{
	return false;
}

bool BTManager::GetRemoteMac(unsigned char buffer[kMacAddrSize])
{
	return false;
}

bool BTManager::EnterATMode()
{
	digitalWrite(key_pin_, HIGH);
	Serial.print("AT\r\n");
	// TODO:
	if (true) {
		return true;
	}
	else{
		digitalWrite(key_pin_, LOW);
		return false;
	}
}

void BTManager::EnterCommunicationMode()
{
	digitalWrite(key_pin_, LOW);
}
