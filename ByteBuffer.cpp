#include "ByteBuffer.h"
#include "Arduino.h"

using namespace StreamSplitter;

ByteBuffer::ByteBuffer(uint8_t* buffer, size_t size)
	: buffer_(buffer), buf_size_(size),
	position_(0), limit_(size),
	mark_(0)
{

}

size_t ByteBuffer::get(void* dst, size_t len)
{
	size_t len_get = len;
	if (len_get > this->remaining())
	{
		len_get = this->remaining();
	}

	memcpy(dst, buffer_ + position_, len_get);
	position_ += len_get;
	return len_get;
}

uint8_t ByteBuffer::get()
{
	unsigned char r = buffer_[position_];
	++position_;
	return r;
}

size_t ByteBuffer::put(ByteBuffer& src)
{
	size_t len_put = src.get(buffer_ + position_, this->remaining());
	position_ += len_put;
	return len_put;
}

size_t ByteBuffer::put(const uint8_t* buffer, size_t size)
{
	ByteBuffer byte_buffer((uint8_t*)buffer, size);
	return put(byte_buffer);
}

size_t ByteBuffer::put(uint8_t data)
{
	return put(&data, 1);
}

uint8_t* ByteBuffer::GetArray() const
{
	return buffer_;
}

void ByteBuffer::clear()
{
	this->position_ = 0;
	this->mark_ = 0;
	this->limit_ = buf_size_;
}

void ByteBuffer::compact()
{
	size_t data_size = this->remaining();
	memcpy(buffer_, buffer_ + position_, data_size);
	position_ = data_size;
	limit_ = buf_size_;
	mark_ = 0;
}

void ByteBuffer::flip()
{
	limit_ = position_;
	position_ = 0;
	mark_ = 0;
}

bool ByteBuffer::hasRemaining() const
{
	return remaining() > 0;
}

size_t ByteBuffer::remaining() const
{
	return limit_ - position_;
}

size_t ByteBuffer::capacity() const
{
	return buf_size_;
}

void ByteBuffer::mark()
{
	mark_ = position_;
}

void ByteBuffer::reset()
{
	position_ = mark_;
}

size_t ByteBuffer::GetLimit() const
{
	return limit_;
}

void ByteBuffer::SetLimit(size_t val)
{
	limit_ = val;
}

size_t ByteBuffer::GetPosition() const
{
	return position_;
}

void ByteBuffer::SetPosition(size_t val)
{
	position_ = val;
}

void ByteBuffer::DuplicateTo(ByteBuffer& dst)
{
	dst.buffer_ = buffer_;
	dst.buf_size_ = buf_size_;

	dst.position_ = position_;
	dst.limit_ = limit_;
	dst.mark_ = mark_;
}
