#ifndef MESSAGE_ENCODER_HPP_INCLUDE
#define MESSAGE_ENCODER_HPP_INCLUDE

#include "Crc32.h"
#include "ByteBuffer.h"
#include "Param.h"
#include "SubBytesFinder.h"
#include <stdint.h>

namespace StreamSplitter{
	class MessageEncoder {
	public:
		MessageEncoder();
		bool encode(ByteBuffer& input, ByteBuffer& output);
		size_t getOutputTotalLen(size_t inputLen);

	private:
		bool packData(ByteBuffer& input, ByteBuffer& output);

		SubBytesFinder startBytesFinder;
		uint8_t headerBuf[Param::headerLen];
		uint8_t crcBuf[Param::CRCLen];
	};
}
#endif