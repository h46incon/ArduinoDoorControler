#include "ByteStreamBuffer.h"


using namespace StreamSplitter;

void ByteStreamBuffer::resetBuffer(ByteBuffer& buffer)
{
	buffer.clear();
	// Now buffer is read for write
	// Flip to read
	buffer.flip();
}

void ByteStreamBuffer::storeRemainInStream()
{
	if (newInBuffer == 0 || !newInBuffer->hasRemaining()) {
		return;
	}
	// Clean remainBuffer, to make most usage of it
	if (!remainBuffer.hasRemaining()){
		resetBuffer(remainBuffer);
	}
	size_t totalLen = newInBuffer->remaining() + remainBuffer.remaining();

	// Enable remainBuffer to WRITE
	size_t lastRemainBufPos = remainBuffer.GetPosition();
	remainBuffer.SetPosition(remainBuffer.GetLimit());
	remainBuffer.SetLimit(remainBuffer.capacity());

	// when could not append directly
	if (newInBuffer->remaining() > remainBuffer.remaining()) {
		if (remainBuffer.capacity() >= totalLen) {
			// Just compact
			remainBuffer.compact();
			lastRemainBufPos = 0;
		}
		else {
			// need new a buffer
			void* new_buf = malloc(totalLen);
			ByteBuffer newBuffer = ByteBuffer((uint8_t*)new_buf, totalLen);
			// Make remainBuffer to READ
			remainBuffer.SetLimit(remainBuffer.GetPosition());
			remainBuffer.SetPosition(lastRemainBufPos);

			// Move to new buffer
			newBuffer.put(remainBuffer);
			free(remainBuffer.GetArray());
			newBuffer.DuplicateTo(remainBuffer);

			// reset pos to zero
			lastRemainBufPos = 0;
		}

	}
	// append remain data
	remainBuffer.put(*newInBuffer);

	// Make remainBuffer to Read
	// Do not use flip() function, because position may not be 0 after flip
	remainBuffer.SetLimit(remainBuffer.GetPosition());
	remainBuffer.SetPosition(lastRemainBufPos);

	newInBuffer = 0;
}

void ByteStreamBuffer::getDataFromBuffer(ByteBuffer& buf, void* dst, size_t len, bool needPop)
{
	if (needPop) {
		buf.get(dst, len);
	}
	else {
		buf.mark();
		buf.get(dst, len);
		buf.reset();
	}
}

void ByteStreamBuffer::Clear()
{
	resetBuffer(remainBuffer);
	newInBuffer = 0;
}

bool ByteStreamBuffer::tryGetMsg(void* dst, size_t msgLen, bool needPop)
{
	size_t dataRemain = remainBuffer.remaining();
	if (newInBuffer != 0) {
		dataRemain += newInBuffer->remaining();
	}
	// test if data is enough
	if (dataRemain < msgLen) {
		// Not enough bytes
		storeRemainInStream();
		return false;
	}

	// byte[] msgReturn = new byte[msgLen];
	size_t remainBufPopLen = msgLen;
	if (remainBufPopLen > remainBuffer.remaining())
	{
		remainBufPopLen = remainBuffer.remaining();
	}
	if (remainBufPopLen > 0) {
		getDataFromBuffer(remainBuffer, dst, remainBufPopLen, needPop);
	}

	const size_t newBufPopLen = msgLen - remainBufPopLen;
	if (newBufPopLen > 0) {
		getDataFromBuffer(*newInBuffer, (unsigned char*)dst + remainBufPopLen, newBufPopLen, needPop);
	}
	return true;
}

void ByteStreamBuffer::newInputStream(ByteBuffer& newInBuffer)
{
	if (this->newInBuffer != 0){
		storeRemainInStream();
	}
	this->newInBuffer = &newInBuffer;
}

void ByteStreamBuffer::newInputStream(void* inStream, size_t length)
{
	ByteBuffer byteBuffer = ByteBuffer((uint8_t*)inStream, length);
	this->newInputStream(byteBuffer);
}

ByteStreamBuffer::~ByteStreamBuffer()
{
	free(remainBuffer.GetArray());
}

ByteStreamBuffer::ByteStreamBuffer(size_t initBufLen) :
remainBuffer((uint8_t*)malloc(initBufLen), initBufLen)
{
	resetBuffer(remainBuffer);
}
