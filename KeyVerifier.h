#ifndef KEY_VERIFIER_HPP_INCLUDE
#define KEY_VERIFIER_HPP_INCLUDE

#include <stdint.h>
#include <stddef.h>

/***********************************************************************
	This Key use MD5 as the hashed algorithm
	So it need 16 bytes begin from eeprom_addr
***********************************************************************/
class KeyVerifier{
public:
	KeyVerifier(size_t eeprom_addr, const char* default_key, size_t default_key_size);

	void Init();
	bool VerifyKey(const char* key, size_t key_size);
	bool StoreKey(const char* key, size_t key_size);
	bool ResetKey();
	
private:
	static void GetHashedKey(const char* key, size_t key_len, uint8_t* output);

	bool IsKeyUnSet();

	const size_t eeprom_addr_;
	static const int kMD5Size = 16;
	uint8_t hashed_key_[kMD5Size];
	uint8_t md5_buf_[kMD5Size];
	const char* default_key_;
	const size_t default_key_size_;



};

#endif // !KEY_VERIFIER_HPP_INCLUDE
