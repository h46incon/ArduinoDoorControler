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
	void NormalWorkSetup();
	void FactoryReset();
	void Reboot();
	bool CheckNeedFactoryReset();

	static bool CheckIfTimeOut(unsigned long begin_time, unsigned long time_out);

	void HandlerSimpleCommand();
	void HandlerStreamCommand();

	static bool OpenDoorHandler(const DeviceTalker::KeyInfo& key_info, void* param);
	static bool ChangeKeyHandler(
		const DeviceTalker::KeyInfo& admin_key,
		const DeviceTalker::KeyInfo& old_key,
		const DeviceTalker::KeyInfo& new_key,
		void* param);
	static bool ChangeAdminKeyHandler(
		const DeviceTalker::KeyInfo& old_admin_key,
		const DeviceTalker::KeyInfo& new_admin_key,
		void* param);
	static void OutPutHandler(const char* data, size_t len, void* param);

	SleepManager sleep_manager_;
	ServoControl servo_control_;
	BTManager bt_manager_;
	DeviceTalker device_talker_;

	struct KeyID{
		enum KeyIDEnum {
			OPEN_DOOR = 0,
			ADMIN,
			_TOTAL
		};

	};

	KeyVerifier opendoor_keyverifier_;
	KeyVerifier admin_keyverifier_;
	static const char * kDefaultOpenDoorKey_;
	static const char * kDefaultAdminKey_;
	// key address in EEPROM
	static const int kKeyAddr = KeyID::OPEN_DOOR * KeyVerifier::kKeyNeedEepromSize;	
	static const int kAdminKeyAddr = KeyID::ADMIN * KeyVerifier::kKeyNeedEepromSize;	

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