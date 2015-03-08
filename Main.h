#ifndef MAIN_HPP_INCLUDE
#define MAIN_HPP_INCLUDE

#include "SleepManager.h"
#include "ServoControl.h"
#include "BTManager.h"
#include "DeviceTalker.h"

class Main{
public:
	Main();
	void setup();
	void loop();

private:
	SleepManager sleep_manager_;
	ServoControl servo_control_;
	BTManager bt_manager_;
	DeviceTalker device_talker_;

	bool is_in_stream_mode_;

	static const int kMacAddrSize = 6;
	unsigned char my_bt_addr_[kMacAddrSize];

	static const uint8_t cRequireSimpleResponse = 0x38;
	static const uint8_t cDeviceSimpleResponse = 0x83;
	static const uint8_t cEnterStreamCommunicate = 0x76;

	static const uint8_t cCommandResonse = 0xFF;
	static const uint8_t cCommandFailed = 0xFD;
	static const uint8_t cRequireVerify = 0xbc;
	static const uint8_t cOpenDoor = 0x69;

};
#endif