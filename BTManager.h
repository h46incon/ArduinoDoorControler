#ifndef BT_MANAGER_HPP_INCLUDE
#define BT_MANAGER_HPP_INCLUDE

/*BT device manager, but not handler UART communication*/
class BTManager{
public:
	static const int kMacAddrSize = 6;
	BTManager(int key_pin, int reset_pin, unsigned long baud);
	void Init();
	bool Reset();
	bool GetMac(unsigned char buffer[kMacAddrSize]);


private:
	enum ATRetrunState
	{
		OK,
		RETURN_ERROR,
		ERROR,
		UNCOMPLETE
	};
	bool EnterATMode();
	void EnterCommunicationMode();
	ATRetrunState ReadATCmdReturn();
	void ParseMacAddr(unsigned char * buffer);
	bool AppendATReturnToBuffer();
	void EmptySerialInput();

	static ATRetrunState CheckATReturnState(const char* buffer);
	static unsigned long HexToInt(const char* str);
	static unsigned char HexToInt(char c);
	static void UlToByte(unsigned long num, unsigned char* bytes, int byte_size);

	static const int kBufSize = 64;
	const int key_pin_;
	const int reset_pin_;
	const unsigned long communicate_baud_;

	char buffer_[kBufSize];
	int avaliable_buffer_size_;
};

#endif