#include "Param.h"

using namespace StreamSplitter;
const uint8_t Param::startBytes[startBytesLen] =
{
	0xFD, 0xB1, 0x85, 0x40
};

const uint8_t Param::endBytes[endBytesLen] = 
{ 
	0x40, 0x85, 0xB1, 0xFD
};