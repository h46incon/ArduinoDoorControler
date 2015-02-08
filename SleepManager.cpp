#include "SleepManager.h"

#include <Arduino.h>
#include <avr/sleep.h>

SleepManager::SleepManager(int pin_num, int irq_num, int led_pin /*= -1*/) : pin_num_(pin_num), irq_num_(irq_num)
, led_pin_(led_pin)
{
	switch (irq_num_)
	{
	case 0:
		wakeISR_n = wakeISR_0;
		break;
	case 1:
		wakeISR_n = wakeISR_1;
		break;
	default:
		wakeISR_n = NULL;
		break;
	}
}

void SleepManager::Init()
{
	pinMode(pin_num_, INPUT_PULLUP);
}

bool SleepManager::TryEnterSleep()
{
	// Do not sleep if bt is connected
	if (digitalRead(pin_num_) == HIGH){
		return false;
	}

	if (wakeISR_n == NULL)
	{
		return false;
	}

	if (led_pin_ != -1)
	{
		digitalWrite(led_pin_, LOW);
	}
	sleep_enable();
	attachInterrupt(irq_num_, wakeISR_n, HIGH);
	/* 0, 1, or many lines of code here */
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_bod_disable();
	sei();
	sleep_cpu();

	/* wake up here */
	sleep_disable();
	if (led_pin_ != -1)
	{
		digitalWrite(led_pin_, HIGH);
	}
	return true;
}

void SleepManager::wakeISR_0()
{
	sleep_disable();
	detachInterrupt(0);
}

void SleepManager::wakeISR_1()
{
	sleep_disable();
	detachInterrupt(1);
}
