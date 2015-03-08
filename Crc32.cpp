#include "Crc32.h"


using namespace StreamSplitter;


const PROGMEM uint32_t CRC32::crc_table[16] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
		0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
		0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
		0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

uint32_t CRC32::crc_bytes(uint8_t *data, size_t len)
{
	uint32_t crc = ~0L;
	for (size_t i = 0; i < len; ++i)
	{
		crc = crc_update(crc, data[i]);
	}
	crc = ~crc;
	return crc;
}

uint32_t CRC32::crc_update(uint32_t crc, uint8_t data)
{
	uint8_t tbl_idx;
	tbl_idx = crc ^ (data >> (0 * 4));
	crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
	//crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
	tbl_idx = crc ^ (data >> (1 * 4));
	crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
	//crc = crc_table[tbl_idx & 0x0f] ^ (crc >> 4);
	return crc;
}


uint32_t CRC32::bigendian_to_int(uint8_t* data, size_t len)
{
	uint32_t reslut = 0;
	for (size_t i = 0; i < len; ++i)
	{
		reslut <<= 8;
		reslut |= data[i];
	}

	return reslut;
}

bool CRC32::int_to_bigendian(uint32_t val, uint8_t* out_put, size_t out_len)
{
	for (long i = out_len - 1; i >= 0; --i) {
		out_put[i] = (uint8_t)(val & 0xFF);
		val >>= 8;
	}

	return val == 0;
}

