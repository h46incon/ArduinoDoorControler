#ifndef MAIN_HPP_INCLUDE
#define MAIN_HPP_INCLUDE

#include "SleepManager.h"
#include "ServoControl.h"
#include "BTManager.h"
#include "DeviceTalker.h"
#include "KeyVerifier.h"

class Main{
public:
	Main();
	void setup();
	void loop();

private:
	void HandlerSimpleCommand();
	void HandlerStreamCommand();
	static bool OpenDoorHandler(const DeviceTalker::KeyInfo& key_info, void* param);
	static void OutPutHandler(const char* data, size_t len, void* param);

	SleepManager sleep_manager_;
	ServoControl servo_control_;
	BTManager bt_manager_;
	DeviceTalker device_talker_;

	KeyVerifier opendoor_keyverifier_;
	static const char * kDefaultOpenDoorKey_;
	static const int kKeyAddr = 0;		// key address in EEPROM

	bool is_in_stream_mode_;
	bool need_open_door_;

	static const int kMacAddrSize = 6;
	unsigned char my_bt_addr_[kMacAddrSize];
	static const size_t input_buf_size_ = 32;
	char input_buffer_[input_buf_size_];

	static const uint8_t cRequireSimpleResponse = 0x38;
	static const uint8_t cDeviceSimpleResponse = 0x83;
	static const uint8_t cEnterStreamCommunicate = 0x76;


};
#endif