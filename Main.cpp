#include "Main.h"

#include "Utility.h"
#include "MD5.h"

#include <Arduino.h>
#include <Servo.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stdlib.h>

int release_anger = 60;
int press_anger = 10;
int servo_pin = 9;
int servo_en_pin = 11;
int led_pin = 13;					// LED is used to stands for running or sleeping state
int alarm_led_pin = 6;				// LED to show some alarm
int factory_reset_detect_pin = 7;	// Detect if user press reset button when power up device

//int bt_enable_pin = 7;
int bt_key_pin = 2;
int bt_state_pin = 3;
int bt_reset_pin = 4;
int bt_IRQn = 1;    // IRQn for pin2
unsigned long bt_baud = 9600UL;

// Bluetooth will keep connected state for 2 minutes. 
// If this connection could not finish work in 2 minutes, then means some errors have happened.
// Then need to restart the bluetooth device.
unsigned long keep_wake_time = 1000UL * 180UL;
unsigned long wake_time = 0;

const char * Main::kDefaultOpenDoorKey_ = "000000";
const char * Main::kDefaultAdminKey_ = "000000";

#ifndef EEPROM_SIZE
#define EEPROM_SIZE		1024
#endif


Main::Main() 
: sleep_manager_(bt_state_pin, bt_IRQn, led_pin)
, servo_control_(servo_pin, servo_en_pin, press_anger, release_anger)
, bt_manager_( bt_key_pin, bt_reset_pin, bt_baud)
, opendoor_keyverifier_(kKeyAddr, kDefaultOpenDoorKey_, strlen(kDefaultOpenDoorKey_))
, admin_keyverifier_(kAdminKeyAddr, kDefaultAdminKey_, strlen(kDefaultAdminKey_))
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

bool Main::ChangeKeyHandler(
	const DeviceTalker::KeyInfo& admin_key, 
	const DeviceTalker::KeyInfo& old_key, 
	const DeviceTalker::KeyInfo& new_key, 
	void* param)
{
	Main* this_ = (Main*)param;
	if ( this_->admin_keyverifier_.VerifyKey(admin_key.key, admin_key.len)
		&& this_->opendoor_keyverifier_.VerifyKey(old_key.key, old_key.len) )
	{
		this_->opendoor_keyverifier_.StoreKey(new_key.key, new_key.len);
		return true;
	}
	else{
		return false;
	}
}


bool Main::ChangeAdminKeyHandler(
	const DeviceTalker::KeyInfo& old_admin_key, 
	const DeviceTalker::KeyInfo& new_admin_key, 
	void* param)
{
	Main* this_ = (Main*)param;
	if ( this_->admin_keyverifier_.VerifyKey(old_admin_key.key, old_admin_key.len))
	{
		this_->admin_keyverifier_.StoreKey(new_admin_key.key, new_admin_key.len);
		return true;
	}
	else{
		return false;
	}
}


void Main::OutPutHandler(const char* data, size_t len, void* param)
{
	Serial.write(data, len);
}

void Main::setup()
{
	wdt_disable();
	InitLedPin();
	if (CheckNeedFactoryReset())
	{
		FactoryReset();
		Reboot();
	}
	else{
		NormalWorkSetup();
	}
}

void Main::NormalWorkSetup()
{
	// LED Setting
	digitalWrite(led_pin, HIGH);

	srand(analogRead(2));
	// Turn off ADC for power consumption
	ADCSRA = 0;

	// NOTE: debug
	// Serial.begin(9600);

	// This init may failed, check it first
	bt_manager_.Init();
	bt_manager_.Reset();
	if (!bt_manager_.GetMac(my_bt_addr_))
	{
		Reboot();
	}

	opendoor_keyverifier_.Init();
	admin_keyverifier_.Init();
	sleep_manager_.Init();
	servo_control_.Init();
	wake_time = millis();


	LOG(F("Init Completed\n"));
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
	device_talker_.setChangeKeyHandler(ChangeKeyHandler, this);
	device_talker_.setChangeAdminKeyHandler(ChangeAdminKeyHandler, this);
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

void Main::FactoryReset()
{
	EraseEEPROM();
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

		// Check need open door
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
		if (CheckIfTimeOut(wake_time, keep_wake_time)){
			LOG(F("Resetting Bluetooth...\n"));
			bt_manager_.Reset();
			wake_time = millis();
			device_talker_.reset();
			is_in_stream_mode_ = false;
		}
	}
	else {
		// Wake from sleep
		device_talker_.reset();
		is_in_stream_mode_ = false;
		wake_time = millis();
		//Serial.println("Wake up from sleeping");
		// hand shake message may missing.
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

void Main::Reboot()
{
	// reset after 4 second
	wdt_enable(WDTO_4S);
	// Blink 
	unsigned long time = 300;
	while (true)
	{
		digitalWrite(alarm_led_pin, LOW);
		delay(time);
		digitalWrite(alarm_led_pin, HIGH);
		delay(time);
	}
}

bool Main::CheckNeedFactoryReset()
{
	// Check if factory reset button is press
	pinMode(factory_reset_detect_pin, INPUT_PULLUP);
	bool need_reset = false;

	// Use do-while to enable break
	do 
	{
		if (digitalRead(factory_reset_detect_pin) == HIGH)
		{
			need_reset = false;
			break;
		}

		// light alarm
		digitalWrite(alarm_led_pin, HIGH);

		// Check if this button have press for a while.
		unsigned long beg_time = millis();
		// Need some filitering?
		while (!CheckIfTimeOut(beg_time, kFactoryResetButtonFiliteTime))
		{
			if (digitalRead(factory_reset_detect_pin) == HIGH)
			{
				// Cancel factory reset
				digitalWrite(alarm_led_pin, LOW);
				need_reset = false;
				break;
			}
			else{
				delay(100);
			}
		}

		need_reset = true;

	} while (false);

	// Power concumption
	pinMode(factory_reset_detect_pin, OUTPUT);
	digitalWrite(factory_reset_detect_pin, LOW);
	return need_reset;
}

bool Main::CheckIfTimeOut(unsigned long begin_time, unsigned long time_out)
{
	// Use subtraction will get correct answer even if millis() is overflowed.
	return (millis() - begin_time > time_out);
}

void Main::InitLedPin()
{
	pinMode(led_pin, OUTPUT);
	pinMode(alarm_led_pin, OUTPUT);
	digitalWrite(led_pin, LOW);
	digitalWrite(alarm_led_pin, LOW);
}

void Main::EraseEEPROM()
{
	uint8_t sreg, i;
	uint16_t addr;
	uint8_t clear[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	uint8_t data[8];

	sreg = SREG;
	cli();

	// Write page by page using the block writing method
	for (addr = 0; addr < EEPROM_SIZE; addr += 8)
	{
		eeprom_read_block((void *)&data[0], (const void *)addr, 8);
		for (i = 0; i < 8; i++){
			if (data[i] != 0xFF)
			{
				eeprom_write_block((void*)&clear[0], (void*)addr, 8);
				break;
			}
		}
	}

	SREG = sreg;
}
