#ifndef STREAM_SPLITTER_HPP_INCLUDE
#define STREAM_SPLITTER_HPP_INCLUDE

#include "ByteBuffer.h"
#include "ByteStreamBuffer.h"
#include "SubBytesFinder.h"

namespace StreamSplitter{
	/**
	* Created by h46incon on 2015/1/16.
	* Split stream into package
	* The attributes of package is store in StreamSplitter.PackageFormat,
	* contain start bytes, header, message
	* The result package contain header and message body, but not start bytes.
	*/
	class Splitter {
		static const size_t kDefaultInitBufSize = 256;
	public:
		// A callback when need get package's length from header
		// LENGTH means loaded data's length, NOT including header
		typedef size_t(*MsgLenGetter)(void* param, const void* header);
		typedef void(*PackageHandler)(void* param, ByteBuffer& package);

		struct PackageFormat{
			const uint8_t* startBytes;
			size_t start_bytes_len;
			size_t headerLen;

			MsgLenGetter lenGetter;
			void* lenGetterParam;
		};

		Splitter(const PackageFormat& format);

		Splitter(const PackageFormat& format, size_t init_buf_size);

		~Splitter();

		void join(void* inStream, size_t length, PackageHandler package_handler, void* param);

		void join(ByteBuffer& inStream, PackageHandler package_handler, void* param);

		void reset();

	private:
		struct BufPieceHandlerParam
		{
			Splitter* splitter;
			PackageHandler package_handler;
			void* package_handler_param;
		};

		static void BufPieceHandler(void *param, ByteBuffer* buffer);

		ByteBuffer* joinWithBufferPiece(ByteBuffer* inStream);

		// Join normal data (expect start bytes)
		// When current state is not waiting for start bytes
		ByteBuffer* joinNormalData(ByteBuffer& inStream);

		void enterNextReceivingState();

		enum ReceivingState{
			START_BYTES = 0,
			HEADER,
			MSG,
		} receivingState;

		ByteStreamBuffer msgBuf;
		ByteBuffer bytebuff_temp_;
		//ReceivingState receivingState;
		size_t nextPackLen;
		const size_t msgHeaderLen;
		const MsgLenGetter lenGetter;
		void* len_getter_param_;

		uint8_t* package_buf_;
		size_t package_buf_len_;
		SubBytesFinder startBytesFinder;
	};
}
#endif