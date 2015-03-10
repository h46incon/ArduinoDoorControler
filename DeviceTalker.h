#ifndef DEVICE_TALKER_HPP_INCLUDE
#define DEVICE_TALKER_HPP_INCLUDE

#include "Encrypter.h"
#include "MessageEncoder.h"
#include "MessageDecoder.h"
#include "ByteBuffer.h"
#include "Param.h"

#include <stdint.h>


class DeviceTalker{
public:
	typedef void(*OutPutHandler)(const char* output, size_t len, void* param);
	typedef bool(*OpenDoorHandler)(const char* key_buf, size_t len, void* param);

	DeviceTalker();

	void* onDataInput(const char* input, size_t len);

	void setMacAddr(const uint8_t* mac_addr);

	void setKey2(uint8_t key);

	void setOpenDoorHandler(OpenDoorHandler handler, void* param);

	void setOutPutHandler(OutPutHandler handler, void* param);

	void reset();

private:

	static void MessageHandler(ByteBuffer& package, void* param);

	void PrivateMessageHandler(ByteBuffer& package);

	StreamSplitter::ByteBuffer* CommandHandler(ByteBuffer& cmd);

	StreamSplitter::ByteBuffer* GetErrorCmdRepsond();

	StreamSplitter::ByteBuffer* onOpenDoor(ByteBuffer& key);

	StreamSplitter::ByteBuffer* GetDeviceVerifyMsg();

	/*
	* Warnning: this function will modify data_buf_!
	*/
	StreamSplitter::ByteBuffer* packData();

	Encrypter encrypter_;
	StreamSplitter::MessageDecoder msg_decoder_;
	StreamSplitter::MessageEncoder msg_encoder_;

	StreamSplitter::ByteBuffer data_buf_;
	StreamSplitter::ByteBuffer package_buf_;
	StreamSplitter::ByteBuffer temp_bb_struct_; /* this is a ugly filed, but I could not use malloc to new a object*/
	static const size_t max_load_len_ = 128;
	static const size_t max_stream_buf_len =
		StreamSplitter::Param::packageMinLen + max_load_len_;

	static const size_t kMacAddrSize = 6;
	const uint8_t* mac_addr_;
	OpenDoorHandler open_door_handler_;
	void* open_door_handler_param_;
	OutPutHandler output_handler_;
	void* output_handler_param_;

	static const uint8_t cCommandResonse = 0xFF;
	static const uint8_t cCommandError = 0xFD;
	static const uint8_t cRequireVerify = 0xBC;
	static const uint8_t cOpenDoor = 0x69;
	static const uint8_t cOpenDoorSuccess = 0x96;
	static const uint8_t cOpenDoorKeyError = 0x99;
};
#endif