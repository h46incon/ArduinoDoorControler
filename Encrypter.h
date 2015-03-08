#ifndef ENCRYPTER_HPP_INCLUDE
#define ENCRYPTER_HPP_INCLUDE

#include "ByteBuffer.h"
#include <stdint.h>
using StreamSplitter::ByteBuffer;
/**
* Created by h46incon on 2015/2/27.
* This class is used to encrypt data
* it will encrypt data with device MAC address
* First it will put 2 random bytes.
*  The number of bit 1 in first bytes indicate the position of MAC address
*      The target will act as KEY1
*  And the second bytes will act as KEY2
* The message's encode key is KEY = KEY1 ^ KEY2
* And the message will by calc by ^KEY
*/
class Encrypter {
public:
	void setKey(const uint8_t* mac_addr, size_t size);

	const uint8_t* getMacAddr();

	bool encrypt(ByteBuffer& input, ByteBuffer& output);

	bool decrypt(ByteBuffer& input, ByteBuffer& output);

private:
	uint8_t calcKey(uint8_t random1, uint8_t ramdom2);

	void doXOR(ByteBuffer& input, uint8_t key, ByteBuffer& output);

	size_t bitCount(unsigned int i);

	const uint8_t * macAddr;
	size_t macAddrSize;
};

#endif