#ifndef BT_MANAGER_HPP_INCLUDE
#define BT_MANAGER_HPP_INCLUDE

/*BT device manager, but not handler UART communication*/
class BTManager{
public:
	BTManager(int enable_pin, int key_pin);
	void Init(unsigned long baud);
	void Reset();

private:
	int enable_pin_;
	int key_pin_;
};
#endif