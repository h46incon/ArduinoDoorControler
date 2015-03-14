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

bool DeviceTalker::TryGetKeyInfo(ByteBuffer& message, KeyInfo& output)
{
	if (!message.hasRemaining())
	{
		return false;
	}
	size_t key_len = message.get();
	if (key_len > message.remaining())
	{
		return false;
	}
	const size_t pos = message.GetPosition();
	output.len = key_len;
	output.key = (char*)message.GetArray() + pos;
	message.SetPosition(pos + key_len);
	return true;
}


StreamSplitter::ByteBuffer* DeviceTalker::GetDeviceVerifyMsg()
{
	// Put data
	data_buf_.clear();
	data_buf_.put(cCmdSuccess);
	data_buf_.put(mac_addr_, kMacAddrSize);
	data_buf_.flip();

	return packData();
}

StreamSplitter::ByteBuffer* DeviceTalker::onOpenDoor(ByteBuffer& cmd)
{
	bool is_success = false;

	if (open_door_handler_ != NULL)
	{
		KeyInfo key;
		if (TryGetKeyInfo(cmd, key))
		{
			is_success = open_door_handler_(key, open_door_handler_param_);
		}
	}

	// generate output
	return MakeKeyFeedBackMessage(is_success);
}

StreamSplitter::ByteBuffer* DeviceTalker::MakeKeyFeedBackMessage(bool is_success)
{

	data_buf_.clear();
	data_buf_.put(
		is_success ? cCmdSuccess : cKeyError);
	data_buf_.flip();

	return packData();
}


StreamSplitter::ByteBuffer* DeviceTalker::onChangeKey(ByteBuffer& cmd)
{
	bool is_success = false;
	// Use While loop so I can break in the inner code
	while (changekey_handler_ != NULL)
	{
		KeyInfo admin_key;
		KeyInfo old_key;
		KeyInfo new_key;
		if (!TryGetKeyInfo(cmd, admin_key))
		{
			break;
		}
		if (!TryGetKeyInfo(cmd, old_key))
		{
			break;
		}
		if (!TryGetKeyInfo(cmd, new_key))
		{
			break;
		}
		
		is_success = changekey_handler_(admin_key, old_key, new_key, changekey_handler_param_);
		break;
	}

	return MakeKeyFeedBackMessage(is_success);
}

StreamSplitter::ByteBuffer* DeviceTalker::onChangeAdminKey(ByteBuffer& cmd)
{
	bool is_success = false;
	// Use While loop so I can break in the inner code
	while (changeadminkey_handler_ != NULL)
	{
		KeyInfo old_admin_key;
		KeyInfo new_admin_key;
		if (!TryGetKeyInfo(cmd, old_admin_key))
		{
			break;
		}
		if (!TryGetKeyInfo(cmd, new_admin_key))
		{
			break;
		}
		
		is_success = changeadminkey_handler_(old_admin_key, new_admin_key, changeadminkey_handler_param_);
		break;
	}

	return MakeKeyFeedBackMessage(is_success);
}

StreamSplitter::ByteBuffer* DeviceTalker::GetErrorCmdRepsond()
{
	// Put data
	data_buf_.clear();
	data_buf_.put(cNotACmd);
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
		return onOpenDoor(cmd);
	case cChangeKey:
		return onChangeKey(cmd);
	case cChangeAdminKey:
		return onChangeAdminKey(cmd);
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

void DeviceTalker::setChangeKeyHandler(ChangeKeyHandler handler, void* param)
{
	this->changekey_handler_ = handler;
	this->changekey_handler_param_ = param;
}

void DeviceTalker::setChangeAdminKeyHandler(ChangeAdminKeyHandler handler, void* param)
{
	this->changeadminkey_handler_ = handler;
	this->changeadminkey_handler_param_ = param;
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
temp_bb_struct_(NULL, 0),
open_door_handler_(NULL),
output_handler_(NULL),
changekey_handler_(NULL),
changeadminkey_handler_(NULL)
{

}
