#include "BTManager.h"
#include "Utility.h"

#include <Arduino.h>

BTManager::BTManager(int key_pin, int reset_pin, unsigned long baud)
	: key_pin_(key_pin), reset_pin_(reset_pin), communicate_baud_(baud)
{

}

void BTManager::Init( )
{
	// BlueTooth Setting
	pinMode(key_pin_, OUTPUT);
	pinMode(reset_pin_, OUTPUT);

	digitalWrite(reset_pin_, HIGH);
	EnterCommunicationMode();	

	// Enable uart
	Serial.begin(communicate_baud_);
}

bool BTManager::Reset()
{
	digitalWrite(reset_pin_, LOW);
	delay(100);
	digitalWrite(reset_pin_, HIGH);
	return true;
	//if (EnterATMode()){
	//	Serial.print(F("AT+RESET\r\n"));
	//	Serial.flush();
	//	// Make sure command have been run
	//	ReadATCmdReturn();
	//	// TODO: HC-05 may enter AT mode when reset
	//	EnterCommunicationMode();

	//	return true;
	//} else{
	//	EnterCommunicationMode();
	//	return false;
	//}

}

bool BTManager::GetMac(unsigned char buffer[kMacAddrSize])
{
	bool result = false;
	if (EnterATMode())
	{
		EmptySerialInput();
		Serial.print(F("AT+ADDR?\r\n"));
		Serial.flush();
		if (ReadATCmdReturn() == OK){
			ParseMacAddr(buffer);

			result = true;
		}
		else{
			result = false;
		}
	}
	else {
		result = false;
	}

	EnterCommunicationMode();
	return result;
}

bool BTManager::EnterATMode()
{
	digitalWrite(key_pin_, HIGH);
	EmptySerialInput();
	Serial.print(F("AT\r\n"));
	Serial.flush();
	if (ReadATCmdReturn() == OK) {
		return true;
	}
	else{
		digitalWrite(key_pin_, LOW);
		return false;
	}
}

void BTManager::EnterCommunicationMode()
{
	digitalWrite(key_pin_, LOW);
}

BTManager::ATRetrunState BTManager::ReadATCmdReturn()
{
	avaliable_buffer_size_ = 0;
	if ( AppendATReturnToBuffer()  == false)
	{
		return ERROR;
	}
	// Append \0 in the last of buffer
	// Because many function of string need last \0
	// Replace last \r 
	if (buffer_[avaliable_buffer_size_ - 1] == '\r')
	{
		buffer_[avaliable_buffer_size_ - 1] = '\0';
	}
	// or append \0
	else{
		buffer_[avaliable_buffer_size_] = '\0';
		++avaliable_buffer_size_;
	}
	const int first_return_size = avaliable_buffer_size_;

	ATRetrunState state = CheckATReturnState(buffer_);
	if (state != UNCOMPLETE)
	{
		return state;
	}

	if (AppendATReturnToBuffer() == false)
	{
		return ERROR;
	}
	else{
		buffer_[avaliable_buffer_size_] = '\0';
		return CheckATReturnState(buffer_ + first_return_size);
	}

}

void BTManager::EmptySerialInput()
{
	while (Serial.available() > 0)
	{
		Serial.read();
	}
}

BTManager::ATRetrunState BTManager::CheckATReturnState(const char* buffer)
{
	const char* OK_Return = "OK";
	const char* ERROR_Return = "ERROR";
	if (strncmp(buffer, OK_Return, strlen(OK_Return)) == 0){
		return OK;
	}
	else if (strncmp(buffer, ERROR_Return, strlen(ERROR_Return)) == 0){
		// Consider return error code
		return RETURN_ERROR;
	}
	else {
		return UNCOMPLETE;
	}
}

bool BTManager::AppendATReturnToBuffer()
{
	int read_num = 
		Serial.readBytesUntil('\n', 
			buffer_ + avaliable_buffer_size_, 
			kBufSize - avaliable_buffer_size_);

	// NOTE: the last '\n' will not be read

	avaliable_buffer_size_ += read_num;
	// kBufSize is 64, it is enough for all legal return, including return error 
	// So if read 64 byte, it must be some errors
	if (read_num == 0 || avaliable_buffer_size_ >= kBufSize)
	{
		return false;
	}
	else{
		return true;
	}

}

unsigned long BTManager::HexToInt(const char* str)
{
	const char* ptr = str;
	unsigned long result = 0;
	while ((*ptr) != '\0' )
	{
		result <<= 4;
		result |= HexToInt(*ptr);
		++ptr;
	}

	return result;
}

unsigned char BTManager::HexToInt(char c)
{
	if (c >= '0' && c <= '9')
	{
		return (unsigned char)(c - '0');
	}
	else if (c >= 'A' && c <= 'Z')
	{
		return (unsigned char)(c - 'A' + 10);
	}
	else if (c >= 'a' && c <= 'z')
	{
		return (unsigned char)(c - 'a' + 10);
	}
	else
	{
		return 0;
	}
}

void BTManager::UlToByte(unsigned long num, unsigned char* bytes, int byte_size)
{
	int i = byte_size - 1;
	while (i >= 0)
	{
		bytes[i] = num & 0xFFUL;
		num >>= 8;
		--i;
	}
}

void BTManager::ParseMacAddr(unsigned char * output)
{

	// The return is return in format: "+ADDR:1234:56:789ABC"
	// But each section in address will delete all "0" in prefix

	char* p = strtok(buffer_, ":");
	unsigned long addr_section;

	// Skip "+ADDR"

	p = strtok(NULL, ":");
	addr_section = HexToInt(p);
	UlToByte(addr_section, output, 2);

	p = strtok(NULL, ":");
	addr_section = HexToInt(p);
	UlToByte(addr_section, output + 2, 1);

	p = strtok(NULL, ":");
	addr_section = HexToInt(p);
	UlToByte(addr_section, output + 3, 3);
}
