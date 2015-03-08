#include "Splitter.h"

using namespace StreamSplitter;
#ifndef NULL 
	#if defined(__cplusplus)
		#define NULL 0
	#else
		#define NULL ((void *)0)
	#endif
#endif

void Splitter::enterNextReceivingState()
{
	switch (receivingState)
	{
	case START_BYTES:
		receivingState = HEADER;
		break;
	case HEADER:
		receivingState = MSG;
		break;
	case MSG:
		receivingState = START_BYTES;
		break;
	}
}

ByteBuffer* Splitter::joinNormalData(ByteBuffer& inStream)
{
	msgBuf.newInputStream(inStream);

	if (receivingState == HEADER) {
		// do not pop header from buffer
		if (msgBuf.tryGetMsg(package_buf_, msgHeaderLen, false)){
			// package's length include header
			nextPackLen = lenGetter(len_getter_param_, package_buf_) + msgHeaderLen;
			enterNextReceivingState();
		}
		else {
			return NULL;
		}
	}

	if (receivingState == MSG) {
		if (nextPackLen > package_buf_len_)
		{
			free(package_buf_);
			package_buf_ = (uint8_t*)malloc(nextPackLen);
		}

		if (msgBuf.tryGetMsg(package_buf_, nextPackLen, true)){
			ByteBuffer buf(package_buf_, nextPackLen);
			buf.DuplicateTo(bytebuff_temp_);
			enterNextReceivingState();
			return &bytebuff_temp_;
		}
		else {
			return NULL;
		}
	}

	return NULL;
}

ByteBuffer* Splitter::joinWithBufferPiece(ByteBuffer* inStream)
{
	if (inStream == NULL) {
		// is a start byte
		this->msgBuf.clear();
		// enter next state of START_BYTES
		this->receivingState = START_BYTES;
		enterNextReceivingState();
		return NULL;
	}
	else {
		// normal data
		if (receivingState == START_BYTES) {
			return NULL;
		}
		else {
			return joinNormalData(*inStream);
		}
	}
}

void Splitter::BufPieceHandler(void *param, ByteBuffer* buffer)
{
	//List<byte[]> result = new LinkedList<>();

	//for (ByteBuffer buf : buffers) {
	//	byte[] pack = joinWithBufferPiece(buf);
	//	if (pack != null) {
	//		result.add(pack);
	//	}
	//}

	//return result;
	BufPieceHandlerParam* r_param = (BufPieceHandlerParam*)param;
	ByteBuffer* package = r_param->splitter->joinWithBufferPiece(buffer);
	if (package != NULL)
	{
		r_param->package_handler(r_param->package_handler_param, *package);
	}
}

void Splitter::join(ByteBuffer& inStream, PackageHandler package_handler, void* param)
{
	BufPieceHandlerParam bparam;
	bparam.splitter = this;
	bparam.package_handler = package_handler;
	bparam.package_handler_param = param;

	startBytesFinder.slip(inStream, &bparam, BufPieceHandler);
}

void Splitter::join(void* inStream, size_t length, PackageHandler package_handler, void* param)
{
	ByteBuffer byteBuffer = ByteBuffer((uint8_t*)inStream, length);
	this->join(byteBuffer, package_handler, param);
}

Splitter::~Splitter()
{
	free(package_buf_);
}

Splitter::Splitter(const PackageFormat& format, size_t init_buf_size) :
msgBuf(init_buf_size),
bytebuff_temp_(NULL, 0),
msgHeaderLen(format.headerLen),
lenGetter(format.lenGetter),
len_getter_param_(format.lenGetterParam),
startBytesFinder(format.startBytes, format.start_bytes_len)
{
	package_buf_len_ = init_buf_size;
	nextPackLen = 0;
	receivingState = START_BYTES;
	package_buf_ = (uint8_t*)malloc(package_buf_len_);
}


Splitter::Splitter(const PackageFormat& format) : Splitter(format, kDefaultInitBufSize)
{

}

void StreamSplitter::Splitter::reset()
{
	msgBuf.clear();
	receivingState = START_BYTES;
	nextPackLen = 0;
	startBytesFinder.reset();
}

