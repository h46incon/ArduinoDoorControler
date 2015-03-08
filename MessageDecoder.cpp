#include "MessageDecoder.h"

#include "Param.h"
#include "Crc32.h"

#ifndef NULL
#define NULL 0
#endif

using namespace StreamSplitter;


ByteBuffer* MessageDecoder::getLoad(ByteBuffer& package)
{
	const size_t len = package.remaining();
	uint8_t* pack_data = package.GetArray() + package.GetPosition();
	// Check length
	if (len < Param::packageMinLen) {
		return NULL;
	}

	// Check end bytes
	for (size_t i = Param::endBytesLen; i > 0; --i) {
		if (pack_data[len - i] != Param::endBytes[Param::endBytesLen - i]) {
			return NULL;
		}
	}


	// Check CRC
	const size_t loadLen = len - Param::headerLen - Param::CRCLen - Param::endBytesLen;
	uint32_t crcTarget = CRC32::crc_bytes(pack_data + Param::headerLen, loadLen);
	// Get CRC value in package
	uint32_t crcExpect = CRC32::bigendian_to_int(pack_data + Param::headerLen + loadLen, Param::CRCLen);

	if (crcTarget != crcExpect) {
		return NULL;
	}

	ByteBuffer result(pack_data + Param::headerLen, loadLen);
	result.DuplicateTo(byte_buf_temp_);
	return &byte_buf_temp_;
}


void MessageDecoder::decode(ByteBuffer& input, MessageHandler handler, void* param)
{
	PackageHandlerParam p_param;
	p_param._this = this;
	p_param.mes_handler = handler;
	p_param.mes_handler_param = param;

	streamSplitter.join(input, PackageHandler,(void*) &p_param);

}

void StreamSplitter::MessageDecoder::PackageHandler(ByteBuffer& package, void* param)
{
	PackageHandlerParam* r_param = (PackageHandlerParam*)param;
	MessageDecoder* _this = r_param->_this;

	//List<byte[]> packages = streamSplitter.join(input);

		ByteBuffer* data = _this->getLoad(package);
		if (data == NULL) {
			// Note: NACK
		}
		else {
			// TODO: Decrypt using load_buffer_
			// TODO: And run callback
			r_param->mes_handler(*data, r_param->mes_handler_param);

			//_this->load_buffer_.clear();
			//cipher.doFinal(data, load_buffer_);

			//load_buffer_.flip();
			//// Skip random byte
			//if (load_buffer_.remaining() < Param.randomLenInLoad) {
			//	return null;
			//}
			//load_buffer_.position(load_buffer_.position() + Param.randomLenInLoad);

			//// copy result
			//byte[] r = new byte[load_buffer_.remaining()];
			//load_buffer_.get(r);
			//results.add(r);
			// Note: ACK
		}

}
void MessageDecoder::decode(const void* msg, size_t length, MessageHandler handler, void* param)
{
	ByteBuffer byteBuffer((uint8_t*)msg, length);
	decode(byteBuffer, handler, param);
}

size_t StreamSplitter::MessageDecoder::MsgLenGetter(const void* header, void* param)
{
	const unsigned char* r_header = (const unsigned char*)header;
	size_t len = r_header[0] & ~(1 << 7);
	len <<= 8;
	len |= r_header[1];
	if (len > max_stream_buf_len)
	{
		len = max_stream_buf_len;
	}
	return len;
}

void StreamSplitter::MessageDecoder::reset()
{
	streamSplitter.reset();
}


MessageDecoder::MessageDecoder():
	streamSplitter(
		Splitter::PackageFormat{ 
			Param::startBytes, Param::startBytesLen, Param::headerLen, MsgLenGetter, NULL
		}
		,max_stream_buf_len)
	, load_buffer_(load_buffer_m_, max_load_len)
	, byte_buf_temp_(NULL, 0)
{
}

