#ifndef UTILITY_HPP_INCLUDE
#define UTILITY_HPP_INCLUDE

#include <Arduino.h>

#define SERIAL_LOG 0


#if SERIAL_LOG

#define LOG(format, ...) \
    do{\
        Serial.print(format, ##__VA_ARGS__); \
        Serial.flush(); \
	} while (0)
#else
#define LOG(format, ...) 
#endif

#endif