#include "DeviceTalker.h"




StreamSplitter::ByteBuffer* DeviceTalker::packData()
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

StreamSplitter::ByteBuffer* DeviceTalker::GetDeviceVerifyMsg()
{
	// Put data
	data_buf_.clear();
	data_buf_.put(cCommandResonse);
	data_buf_.put(mac_addr_, kMacAddrSize);
	data_buf_.flip();

	return packData();
}

StreamSplitter::ByteBuffer* DeviceTalker::onOpenDoor(ByteBuffer& key)
{
	bool open_success = false;
	if (open_door_handler_ != NULL)
	{
		open_success = open_door_handler_(
			(char*)key.GetArray() + key.GetPosition(),
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

StreamSplitter::ByteBuffer* DeviceTalker::GetErrorCmdRepsond()
{
	// Put data
	data_buf_.clear();
	data_buf_.put(cCommandError);
	data_buf_.flip();

	return packData();
}

StreamSplitter::ByteBuffer* DeviceTalker::CommandHandler(ByteBuffer& cmd)
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

void DeviceTalker::PrivateMessageHandler(ByteBuffer& package)
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

void DeviceTalker::MessageHandler(ByteBuffer& package, void* param)
{
	DeviceTalker* this_ = (DeviceTalker*)param;
	this_->PrivateMessageHandler(package);
}

void DeviceTalker::reset()
{
	msg_decoder_.reset();
}

void DeviceTalker::setOutPutHandler(OutPutHandler handler, void* param)
{
	output_handler_ = handler;
	output_handler_param_ = param;
}

void DeviceTalker::setOpenDoorHandler(OpenDoorHandler handler, void* param)
{
	open_door_handler_ = handler;
	open_door_handler_param_ = param;
}

void DeviceTalker::setKey2(uint8_t key)
{
	encrypter_.setKey2(key);
}

void DeviceTalker::setMacAddr(const uint8_t* mac_addr)
{
	mac_addr_ = mac_addr;
	encrypter_.setKey(mac_addr, kMacAddrSize);
}

void* DeviceTalker::onDataInput(const char* input, size_t len)
{
	msg_decoder_.decode(input, len, MessageHandler, this);
	return NULL;
}

DeviceTalker::DeviceTalker() :
data_buf_((uint8_t*)malloc(max_stream_buf_len), max_stream_buf_len),
package_buf_((uint8_t*)malloc(max_stream_buf_len), max_stream_buf_len),
temp_bb_struct_(NULL, 0)
{

}
