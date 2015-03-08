#include "Encrypter.h"
#include <stdlib.h>


size_t Encrypter::bitCount(unsigned int i)
{
	size_t count = 0;
	while (i != 0) {
		i &= (i - 1);
		++count;
	}

	return count;
}

void Encrypter::doXOR(ByteBuffer& input, uint8_t key, ByteBuffer& output)
{
	while (input.hasRemaining()) {
		uint8_t d = input.get();
		output.put(d ^ key);
	}
}

uint8_t Encrypter::calcKey(uint8_t random1, uint8_t ramdom2)
{
	size_t bit_count = bitCount(random1);
	uint8_t key1 = macAddr[bit_count % macAddrSize];
	return (key1 ^ ramdom2);
}

bool Encrypter::decrypt(ByteBuffer& input, ByteBuffer& output)
{
	if (input.remaining() < 2) {
		return false;
	}
	if (output.remaining() < input.remaining() - 2) {
		return false;
	}

	uint8_t random1 = input.get();
	uint8_t random2 = input.get();
	uint8_t key = calcKey(random1, random2);

	doXOR(input, key, output);
	return true;
}

bool Encrypter::encrypt(ByteBuffer& input, ByteBuffer& output)
{
	if (output.remaining() < input.remaining() + 2) {
		return false;
	}

	uint8_t random1 = rand();
	uint8_t random2 = rand();
	uint8_t key = calcKey(random1, random2);

	output.put(random1);
	output.put(random2);
	doXOR(input, key, output);

	return true;
}

const uint8_t* Encrypter::getMacAddr()
{
	return macAddr;
}

void Encrypter::setKey(const uint8_t* mac_addr, size_t size)
{
	this->macAddr = mac_addr;
	this->macAddrSize = size;
}
