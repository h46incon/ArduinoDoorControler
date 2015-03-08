#ifndef DEVICE_TALKER_HPP_INCLUDE
#define DEVICE_TALKER_HPP_INCLUDE

#include "Encrypter.h"
#include "MessageEncoder.h"
#include "MessageDecoder.h"
#include "ByteBuffer.h"
#include "Param.h"

#include <stdint.h>
#include <Arduino.h>


class DeviceTalker{
public:
	typedef void(*OutPutHandler)(const char* output, size_t len, void* param);
	typedef bool(*OpenDoorHandler)(const char* key, size_t len, void* param);

	DeviceTalker():
		data_buf_((uint8_t*)malloc(max_stream_buf_len), max_stream_buf_len),
		package_buf_((uint8_t*)malloc(max_stream_buf_len), max_stream_buf_len),
		temp_bb_struct_(NULL, 0)
	{
	}

	void* onDataInput(const char* input, size_t len)
	{
		msg_decoder_.decode(input, len, MessageHandler, this);
		return NULL;
	}

	void setMacAddr(const uint8_t* mac_addr)
	{
		mac_addr_ = mac_addr;
		encrypter_.setKey(mac_addr, kMacAddrSize);
	}

	void setKey2(uint8_t key)
	{
		encrypter_.setKey2(key);
	}

	void setOpenDoorHandler(OpenDoorHandler handler, void* param)
	{
		open_door_handler_ = handler;
		open_door_handler_param_ = param;
	}

	void setOutPutHandler(OutPutHandler handler, void* param)
	{
		output_handler_ = handler;
		output_handler_param_ = param;
	}

	void reset()
	{
		msg_decoder_.reset();
	}

private:

	static void MessageHandler(ByteBuffer& package, void* param)
	{
		DeviceTalker* this_ = (DeviceTalker*) param;
		this_->PrivateMessageHandler(package );
	}

	void PrivateMessageHandler(ByteBuffer& package)
	{
		data_buf_.clear();
		encrypter_.decrypt(package, data_buf_);
		data_buf_.flip();

		ByteBuffer* output = CommandHandler(data_buf_);

		if (output != NULL)
		{
			if (output_handler_ != NULL)
			{
				output_handler_(
					(const char*)output->GetArray() + output->GetPosition(),
					output->remaining(),
					output_handler_param_);
			}
		}
	}

	StreamSplitter::ByteBuffer* CommandHandler(ByteBuffer& cmd)
	{
		if (!cmd.hasRemaining())
		{
			return NULL;
		}

		uint8_t c = cmd.get();
		switch (c)
		{
		case cRequireVerify:
			return GetDeviceVerifyMsg();
		case cOpenDoor:
			// remain in cmd is key
			return onOpenDoor(cmd);
		default:
			return GetErrorCmdRepsond();
		}
	}

	StreamSplitter::ByteBuffer* GetErrorCmdRepsond()
	{
		// Put data
		data_buf_.clear();
		data_buf_.put(cCommandError);
		data_buf_.flip();

		return packData();
	}

	StreamSplitter::ByteBuffer* onOpenDoor(ByteBuffer& key)
	{
		bool open_success = false;
		if (open_door_handler_ != NULL)
		{
			open_success = open_door_handler_(
				(const char*)key.GetArray() + key.GetPosition(),
				key.remaining(),
				open_door_handler_param_);
		}

		// generate output
		data_buf_.clear();
		data_buf_.put(
			open_success ? cOpenDoorSuccess : cOpenDoorKeyError);
		data_buf_.flip();

		return packData();
	}

	StreamSplitter::ByteBuffer* GetDeviceVerifyMsg()
	{
		// Put data
		data_buf_.clear();
		data_buf_.put(cCommandResonse);
		data_buf_.put(mac_addr_, kMacAddrSize);
		data_buf_.flip();

		return packData();
	}

	/*
	* Warnning: this function will modify data_buf_!
	*/
	StreamSplitter::ByteBuffer* packData()
	{
		// encrypted
		package_buf_.clear();
		if (!encrypter_.encrypt(data_buf_, package_buf_)){
			return NULL;
		}
		package_buf_.flip();

		// Pack data
		data_buf_.clear();
		//if (!msg_encoder_.encode(package_buf_, data_buf_)){
		//	return NULL;
		//}
		// Send it even if is has a start bytes in context
		msg_encoder_.encode(package_buf_, data_buf_);
		data_buf_.flip();

		data_buf_.DuplicateTo(temp_bb_struct_);
		return &temp_bb_struct_;
	}

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