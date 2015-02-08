#ifndef SLEEP_MANAGER_HPP_INCLUDE
#define SLEEP_MANAGER_HPP_INCLUDE

class SleepManager{
public:
	SleepManager(int pin_num, int irq_num, int led_pin = -1);

	void Init();

	bool TryEnterSleep();

private:
	void static wakeISR_0();

	void static wakeISR_1();

	void (*wakeISR_n)(void);

	int pin_num_;
	int irq_num_;
	int led_pin_;
};
#endif