#ifndef BYTE_BUFFER_HPP_INCLUDE
#define BYTE_BUFFER_HPP_INCLUDE

#include <stdint.h>
#include <stddef.h>

namespace StreamSplitter{
	class ByteBuffer{
		// typedef unsigned int size_t;
	public:
		ByteBuffer(uint8_t* buffer, size_t size);

		size_t get(void* dst, size_t len);
		unsigned char get();
		size_t put(ByteBuffer& src);
		size_t put(const uint8_t* buffer, size_t size);
		size_t put(uint8_t data);

		uint8_t* GetArray() const;

		void clear();
		void compact();
		void flip();

		bool hasRemaining() const;
		size_t remaining() const;
		size_t capacity() const;

		void mark();
		void reset();

		size_t GetLimit() const;
		void SetLimit(size_t val);
		size_t GetPosition() const;
		void SetPosition(size_t val);

		void DuplicateTo(ByteBuffer& dst);

	private:
		uint8_t* buffer_;
		size_t buf_size_;
		size_t position_;
		size_t limit_;
		size_t mark_;

	};

}
#endif