#include "Main.h"

#include <Arduino.h>
#include <Servo.h>
#include <avr/sleep.h>
#include "Utility.h"

int release_anger = 60;
int press_anger = 10;
int servo_pin = 9;
int led_pin = 13;    // LED is used to stands for running or sleeping state
//int bt_enable_pin = 7;
int bt_key_pin = 6;
int bt_state_pin = 2;
int bt_IRQn = 0;    // IRQn for pin2
unsigned long bt_baud = 9600UL;

// Bluetooth will keep connected state for 3 minutes. 
// If this connection could not finish work in 3 minutes, then means some errors have happened.
// Then need to restart the bluetooth device.
unsigned long keep_wake_time = 1000UL * 180UL;
unsigned long wake_time = 0;


Main::Main() 
: sleep_manager_(bt_state_pin, bt_IRQn, led_pin)
, servo_control_(servo_pin, press_anger, release_anger)
, bt_manager_( bt_key_pin, bt_baud)
{

}

void Main::setup()
{
	// LED Setting
	pinMode(led_pin, OUTPUT);
	digitalWrite(led_pin, HIGH);

	sleep_manager_.Init();
	servo_control_.Init();
	bt_manager_.Init();

	wake_time = millis();

	LOG(F("Init Completed\n"));
	bt_manager_.GetMac(my_bt_addr_);

	device_talker_.setMacAddr((uint8_t*)my_bt_addr_);
	// TODO: set key
	is_in_stream_mode_ = false;

	sleep_manager_.TryEnterSleep();
}

void Main::loop()
{
	const size_t buf_size = 32;
	char input_buffer[buf_size];

	while (Serial.available() > 0)
	{
		if (is_in_stream_mode_)
		{
			size_t input_len = Serial.available();
			if (input_len > buf_size)
			{
				input_len = buf_size;
			}
			Serial.readBytes(input_buffer, input_len);

			//ByteBuffer* byte_buf = device_talker_.onDataInput(input_buffer, input_len);
			//if (byte_buf != NULL)
			//{
			//	Serial.write(byte_buf->GetArray() + byte_buf->GetPosition(), byte_buf->remaining());
			//}
		}
		else{
			int c = Serial.read();
			switch ((uint8_t)c){
				// Shake hand
			case cRequireSimpleResponse:
				Serial.write(cDeviceSimpleResponse);
				break;
			case cEnterStreamCommunicate:
				// TODO:
				is_in_stream_mode_ = true;
				break;
			}
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
