#include "KeyVerifier.h"

//* Note: this MD5 class will use 30% of flash in ATmega328.....
#include "MD5.h"
#include <avr/eeprom.h>

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
	eeprom_write_byte((uint8_t*)eeprom_addr_, kValidKeyFlag);
	WriteHashedKey(md5_buf_);

	// read back to hashed_key_, and check
	Init();
	return VerifyKey(key, key_size);
}

bool KeyVerifier::ResetKey()
{
	// I don's know how to erase eeprom
	// Just write 0xFF to eeprom
	// If may set some checksum bit in eeprom block

	// Write flag
	eeprom_write_byte((uint8_t*)eeprom_addr_, 0xFF);

	// clean data
	for (int i = 0; i < kMD5Size; ++i)
	{
		md5_buf_[i] = 0xFF;
	}
	WriteHashedKey(md5_buf_);

	// Reread key
	Init();
	return VerifyKey(default_key_, default_key_size_);
}

bool KeyVerifier::VerifyKey(const char* key, size_t key_size)
{
	GetHashedKey(key, key_size, md5_buf_);
	return memcmp(hashed_key_, md5_buf_, kMD5Size) == 0;
}

KeyVerifier::KeyVerifier(size_t eeprom_addr, const char* default_key, size_t default_key_size) 
	: eeprom_addr_(eeprom_addr), default_key_(default_key), default_key_size_(default_key_size)
{
}

void KeyVerifier::Init()
{
	// check flag
	uint8_t header = eeprom_read_byte((const uint8_t *)eeprom_addr_);
	if (header == kValidKeyFlag)
	{
		// Read key from EEPROM
		ReadHashedKey(hashed_key_);
	}
	else{
		// Use default key instead
		GetHashedKey(default_key_, default_key_size_, hashed_key_);
	}
}

void KeyVerifier::ReadHashedKey(uint8_t* target_addr)
{
	eeprom_read_block(target_addr, (void*)(eeprom_addr_ + kHeaderSize), kMD5Size);
}

void KeyVerifier::WriteHashedKey(uint8_t* src_addr)
{
	eeprom_write_block(src_addr, (void*)(eeprom_addr_ + kHeaderSize), kMD5Size);
}
