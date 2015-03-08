#ifndef CRC_32_HPP_INCLUDE
#define CRC_32_HPP_INCLUDE

#include <stdint.h>
#include <stddef.h>
#include <avr/pgmspace.h>

namespace StreamSplitter{
	class CRC32{
		static const PROGMEM uint32_t crc_table[16];
		// static const uint32_t crc_table[16];

		static uint32_t crc_update(uint32_t crc, uint8_t data);

	public:
		static uint32_t crc_bytes(uint8_t *data, size_t len);

		static uint32_t bigendian_to_int(uint8_t* data, size_t len);
		static bool int_to_bigendian(uint32_t val, uint8_t* out_put, size_t out_len);

	};
}
#endif