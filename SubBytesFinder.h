#ifndef SUB_BYTES_FINDER_HPP_INCLUDE
#define SUB_BYTES_FINDER_HPP_INCLUDE

#include "ByteBuffer.h"

namespace StreamSplitter{
	/**
	* Created by h46incon on 2015/2/17.
	* Find pattern using KMP
	* It could find pattern which may dispersed on many string
	*/
	class SubBytesFinder{
	public:
		SubBytesFinder(const unsigned char* pattern, const int pattern_len);

		typedef void(*ByteBufferHandler)(ByteBuffer* byte_buffer, void* param);

		~SubBytesFinder();

		void reset();

		bool findIn(ByteBuffer& buf);

		// skip all data until read a start bytes
		// return true if found, false if not
		bool skipTillPattern(ByteBuffer& buf);

		// Slip buf to buffers by pattern
		// The first buffer is empty if the pattern exist in the front of buf.
		void slip(ByteBuffer& buf, void* param_cb, ByteBufferHandler handler);

	private:
		void BuildNextTable();

		const unsigned char* pattern_;
		const int pattern_len_;
		int* nextTable;
		int patIndex;
	};

}
#endif