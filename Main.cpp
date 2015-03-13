#include "Main.h"

#include "Utility.h"
#include "MD5.h"

#include <Arduino.h>
#include <Servo.h>
#include <avr/sleep.h>
#include <stdlib.h>

int release_anger = 60;
int press_anger = 10;
int servo_pin = 9;
int servo_en_pin = 11;
int led_pin = 13;    // LED is used to stands for running or sleeping state
//int bt_enable_pin = 7;
int bt_key_pin = 2;
int bt_state_pin = 3;
int bt_reset_pin = 4;
int bt_IRQn = 1;    // IRQn for pin2
unsigned long bt_baud = 9600UL;

// Bluetooth will keep connected state for 2 minutes. 
// If this connection could not finish work in 2 minutes, then means some errors have happened.
// Then need to restart the bluetooth device.
unsigned long keep_wake_time = 1000UL * 120UL;
unsigned long wake_time = 0;

const char * Main::kDefaultOpenDoorKey_ =  "367429";

Main::Main() 
: sleep_manager_(bt_state_pin, bt_IRQn, led_pin)
, servo_control_(servo_pin, servo_en_pin, press_anger, release_anger)
, bt_manager_( bt_key_pin, bt_reset_pin, bt_baud)
, opendoor_keyverifier_(kKeyAddr)
, need_open_door_(false)
{

}


bool Main::OpenDoorHandler(const DeviceTalker::KeyInfo& key_info, void* param)
{
	Main* this_ = (Main*)param;
	if (this_->opendoor_keyverifier_.VerifyKey(key_info.key, key_info.len))
	{
		this_->need_open_door_ = true;
		//this_->servo_control_.OpenDoor();
		return true;
	}
	else
	{
		return false;
	}
}


void Main::OutPutHandler(const char* data, size_t len, void* param)
{
	Serial.write(data, len);
}

void Main::setup()
{
	// LED Setting
	pinMode(led_pin, OUTPUT);
	digitalWrite(led_pin, HIGH);

	srand(analogRead(2));

	// NOTE: debug
	// Serial.begin(9600);

	opendoor_keyverifier_.Init(kDefaultOpenDoorKey_, strlen(kDefaultOpenDoorKey_));
	sleep_manager_.Init();
	servo_control_.Init();
	bt_manager_.Init();
	wake_time = millis();


	LOG(F("Init Completed\n"));
	bt_manager_.GetMac(my_bt_addr_);
	// Note: debug
	//my_bt_addr_[0] = 0x20;
	//my_bt_addr_[1] = 0x13;
	//my_bt_addr_[2] = 0x05;
	//my_bt_addr_[3] = 0x07;
	//my_bt_addr_[4] = 0x08;
	//my_bt_addr_[5] = 0x29;

	device_talker_.setMacAddr((uint8_t*)my_bt_addr_);
	device_talker_.setOpenDoorHandler(OpenDoorHandler, this);
	device_talker_.setOutPutHandler(OutPutHandler, this);
	is_in_stream_mode_ = false;
	need_open_door_ = false;

	// NOTE: debug
	//Serial.println("input data");
	//uint8_t data[] = { 0xFD, 0xB1, 0x85, 0x40, 0x00, 0x0B, 0x96, 0x97, 0x23, 0x9A, 0xB8, 0x70, 0xFF, 0x40, 0x85, 0xB1, 0xFD };
	//device_talker_.onDataInput((const char*)data, 17);
	// delay(2000);

	// wait for init
	delay(1000);

	sleep_manager_.TryEnterSleep();
}

void Main::loop()
{
	while (Serial.available() > 0)
	{
		if (is_in_stream_mode_)
		{
			HandlerStreamCommand();
		}
		else{
			HandlerSimpleCommand();
		}

		// Check nned open door
		if (need_open_door_)
		{
			need_open_door_ = false;
			servo_control_.OpenDoor();
		}
	}

	// try enter sleep
	if (sleep_manager_.TryEnterSleep() == false){
		// Fail to enter sleep means bluetooth is still in contected state
		// Check if it is reach the timeout
		if (millis() - wake_time > keep_wake_time){    // Use subtraction will get correct answer even if millis() is overflowed.
			LOG(F("Resetting Bluetooth...\n"));
			bt_manager_.Reset();
			wake_time = millis();
			device_talker_.reset();
			is_in_stream_mode_ = false;
		}
	}
	else {
		device_talker_.reset();
		is_in_stream_mode_ = false;
		wake_time = millis();
		//Serial.println("Wake up from sleeping");
		// hand shake message may missing.
		// openDoor();
	}

}

void Main::HandlerSimpleCommand()
{
	int c = Serial.read();
	switch ((uint8_t)c){
	case cRequireSimpleResponse:
		// Shake hand
		Serial.write(cDeviceSimpleResponse);
		break;

	case cEnterStreamCommunicate:
		uint8_t random_key = rand();
		//Note: debug
		//uint8_t random_key = 0;
		device_talker_.reset();
		device_talker_.setKey2(random_key);
		// send key back twice
		Serial.write(random_key);
		Serial.write(random_key);

		is_in_stream_mode_ = true;
		break;
	}
}

void Main::HandlerStreamCommand()
{
	size_t input_len = Serial.available();
	if (input_len > input_buf_size_)
	{
		input_len = input_buf_size_;
	}
	Serial.readBytes(input_buffer_, input_len);

	device_talker_.onDataInput(input_buffer_, input_len);
			// The out put will be send in OutPutHandler
}

