#include "MessageEncoder.h"

using namespace StreamSplitter;

bool MessageEncoder::packData(ByteBuffer& input, ByteBuffer& output)
{
	const int inputLen = input.remaining();
	// put start bytes
	output.put(Param::startBytes, Param::startBytesLen);

	// Skip header
	const int loadPos = output.GetPosition() + Param::headerLen;
	output.SetPosition(loadPos);

	// put data
	output.put(input);

	// Put crc32 value
	uint32_t crcVal = CRC32::crc_bytes(output.GetArray() + loadPos, inputLen);
	CRC32::int_to_bigendian(crcVal, crcBuf, Param::CRCLen);
	output.put(crcBuf, Param::CRCLen);

	// Put end bytes
	output.put(Param::endBytes, Param::endBytesLen);

	// backup end pos
	const int endPos = output.GetPosition();

	// Put header
	output.SetPosition(loadPos - Param::headerLen);
	const int loadLen = inputLen + Param::CRCLen + Param::endBytesLen;
	CRC32::int_to_bigendian(loadLen, headerBuf, Param::headerLen);

	output.put(headerBuf, Param::headerLen);

	output.SetPosition(endPos);

	return true;
}

size_t MessageEncoder::getOutputTotalLen(size_t inputLen)
{
	const size_t totalLen =
		Param::startBytesLen + inputLen + Param::CRCLen + Param::endBytesLen;
	return totalLen;
}

bool MessageEncoder::encode(ByteBuffer& input, ByteBuffer& output)
{
	if (output.remaining() < getOutputTotalLen(input.remaining())) {
		return false;
	}

	int begPos = output.GetPosition();
	// Write random bytes
	if (packData(input, output)) {
		int endPos = output.GetPosition();
		// Check if it contain start bytes
		output.SetPosition(begPos + 1);
		startBytesFinder.reset();
		if (!startBytesFinder.findIn(output)) {
			output.SetPosition(endPos);
			return true;
		}
		else {
			return false;
		}
	}

	return false;
}

MessageEncoder::MessageEncoder() : 
	startBytesFinder(Param::startBytes, Param::startBytesLen)
{

}
