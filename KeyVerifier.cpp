#include "KeyVerifier.h"

//* Note: this MD5 class will use 30% of flash in ATmega328.....
#include "MD5.h"
#include <avr/eeprom.h>

bool KeyVerifier::IsKeyUnSet()
{
	for (int i = 0; i < kMD5Size; ++i)
	{
		// Read a unwritten location in EEPROM will return 0xFF;
		if (hashed_key_[i] != 0xFF)
		{
			return false;
		}
	}
	return true;
}

void KeyVerifier::GetHashedKey(const char* key, size_t key_len, uint8_t* output)
{
	static const char* salt_key = "nocnincon";
	const size_t salt_key_len = strlen(salt_key);

	// Will free quickly
	char* key_buf = (char*)malloc(key_len);
	if (key_buf == NULL)
	{
		return;
	}
	memcpy(key_buf, key, key_len);

	for (size_t i = 0; i < key_len; ++i)
	{
		key_buf[i] ^= salt_key[i%salt_key_len];
	}
	MD5::make_hash(key_buf, key_len, (unsigned char*)output);
	free(key_buf);
}

bool KeyVerifier::StoreKey(const char* key, size_t key_size)
{
	GetHashedKey(key, key_size, md5_buf_);
	eeprom_write_block(md5_buf_, (void*)eeprom_addr_, kMD5Size);

	// read back to hashed_key_, and check
	eeprom_read_block((void*)eeprom_addr_, hashed_key_, kMD5Size);
	return VerifyKey(key, key_size);
}

bool KeyVerifier::VerifyKey(const char* key, size_t key_size)
{
	GetHashedKey(key, key_size, md5_buf_);
	return memcmp(hashed_key_, md5_buf_, kMD5Size) == 0;
}

KeyVerifier::KeyVerifier(size_t eeprom_addr) 
	: eeprom_addr_(eeprom_addr)
{
}

void KeyVerifier::Init(const char* default_key, size_t default_key_size)
{
	eeprom_read_block(hashed_key_, (void*)eeprom_addr_, kMD5Size);
	if (IsKeyUnSet())
	{
		// Use default key instead
		GetHashedKey(default_key, default_key_size, hashed_key_);
	}
}
