#ifndef BYTE_STREAM_BUFFER_HPP_INCLUDE
#define BYTE_STREAM_BUFFER_HPP_INCLUDE


#include "ByteBuffer.h"
#include "Arduino.h"
//#include <malloc.h>
/**
* Created by h46incon on 2015/2/17.
* It could get bytes with given length, but may stored in different input stream
*/
namespace StreamSplitter{
	class ByteStreamBuffer {
	private:
		ByteBuffer* newInBuffer;
		ByteBuffer remainBuffer;

	public:

		ByteStreamBuffer(size_t initBufLen);

		~ByteStreamBuffer();

		void newInputStream(void* inStream, size_t length);

		void newInputStream(ByteBuffer& newInBuffer);

		bool tryGetMsg(void* dst, size_t msgLen, bool needPop);

		void clear();

	private:
		static void getDataFromBuffer(
			ByteBuffer& buf, void* dst, size_t len, bool needPop);

		void storeRemainInStream();

		// clear buffer, and make it into read state
		static void resetBuffer(ByteBuffer& buffer);

	};

}
#endif