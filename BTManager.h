#ifndef BT_MANAGER_HPP_INCLUDE
#define BT_MANAGER_HPP_INCLUDE

/*BT device manager, but not handler UART communication*/
class BTManager{
public:
	static const int kMacAddrSize = 6;
	BTManager(int enable_pin, int key_pin);
	void Init(unsigned long baud);
	void Reset();
	bool GetMac(unsigned char buffer[kMacAddrSize]);
	bool GetRemoteMac(unsigned char buffer[kMacAddrSize]);

private:
	bool EnterATMode();
	void EnterCommunicationMode();
	int enable_pin_;
	int key_pin_;
};

#endif