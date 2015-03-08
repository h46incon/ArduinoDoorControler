#ifndef MESSAGE_DECODER_HPP_INCLUDE
#define MESSAGE_DECODER_HPP_INCLUDE

#include "ByteBuffer.h"
#include "Splitter.h"
#include "Param.h"

namespace StreamSplitter{
	/**
	* Created by h46incon on 2015/2/23.
	* This class is used to decode message receive from device
	*/
	class MessageDecoder {


	public:
		typedef void(*MessageHandler)(void* param, ByteBuffer& message);
		MessageDecoder();

		void decode(void* msg, int length, MessageHandler handler, void* param);

		void decode(ByteBuffer& byteBuffer, MessageHandler handler, void* param);
		void reset();

	private:
		struct PackageHandlerParam{
			MessageDecoder* _this;
			MessageHandler mes_handler;
			void* mes_handler_param;
		};

		static void PackageHandler(void* param, ByteBuffer& package);

		static size_t MsgLenGetter(void* param, const void* header);

		ByteBuffer* getLoad(ByteBuffer& package);

		Splitter streamSplitter;

		ByteBuffer load_buffer_;
		ByteBuffer byte_buf_temp_;
		static const size_t max_load_len = 128;
		static const size_t max_stream_buf_len =
			Param::packageMinLen + max_load_len;
		uint8_t load_buffer_m_[max_load_len];
	};
}
#endif