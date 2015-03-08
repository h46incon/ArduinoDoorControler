#ifndef ENCODE_PARM_HPP_INCLUDE
#define ENCODE_PARM_HPP_INCLUDE

#include <stdint.h>
namespace StreamSplitter{
	struct Param {
		const static int startBytesLen = 4;
		const static int endBytesLen = 4;
		const static uint8_t startBytes[startBytesLen];
		const static uint8_t endBytes[endBytesLen];

		const static int headerLen = 2;
		const static int CRCLen = 4;
		const static int packageMinLen = endBytesLen + headerLen + CRCLen;
		const static int randomLenInLoad = 1;
	};

}
#endif // !ENCODE_PARM_HPP_INCLUDE
