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
	DeviceTalker():
		data_buf_((uint8_t*)malloc(max_stream_buf_len), max_stream_buf_len),
		package_buf_((uint8_t*)malloc(max_stream_buf_len), max_stream_buf_len),
		temp_bb_struct_(NULL, 0)
	{
	}

	ByteBuffer* onDataInput(const char* input, size_t len)
	{
		return NULL;
	}

	void setMacAddr(const uint8_t* mac_addr)
	{
		mac_addr_ = mac_addr;
		encrypter_.setKey(mac_addr, kMacAddrSize);
	}

	void setOpenDoorKey(const uint8_t* key, size_t size)
	{
		open_door_key_len = size;
		open_door_key = key;
	}

	void reset()
	{
		msg_decoder_.reset();
	}

private:
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
		encrypter_.encrypt(data_buf_, package_buf_);
		package_buf_.flip();

		// Pack data
		data_buf_.clear();
		msg_encoder_.encode(package_buf_, data_buf_);
		data_buf_.flip();

		data_buf_.DuplicateTo(temp_bb_struct_);
		return &temp_bb_struct_;
	}

	Encrypter encrypter_;
	StreamSplitter::MessageDecoder msg_decoder_;
	StreamSplitter::MessageEncoder msg_encoder_;

	static const size_t buf_size = 64;
	StreamSplitter::ByteBuffer data_buf_;
	StreamSplitter::ByteBuffer package_buf_;

	StreamSplitter::ByteBuffer temp_bb_struct_; /* this is a ugly filed, but I could not use malloc to new a object*/

	static const size_t kMacAddrSize = 6;
	const uint8_t* mac_addr_;
	size_t open_door_key_len;
	const uint8_t* open_door_key;

	static const size_t max_load_len_ = 128;
	static const size_t max_stream_buf_len =
		StreamSplitter::Param::packageMinLen + max_load_len_;

	static const uint8_t cCommandResonse = 0xFF;
	static const uint8_t cCommandFailed = 0xFD;

	static const uint8_t cRequireVerify = 0xbc;
	static const uint8_t cOpenDoor = 0x69;
};
#endif