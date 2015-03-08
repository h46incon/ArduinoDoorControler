#include "SubBytesFinder.h"
#include "Arduino.h"
//#include <memory>

using namespace StreamSplitter;

void SubBytesFinder::BuildNextTable()
{
	//int[] nextTable = new int[pattern.length];
	int j = 0;
	int t = nextTable[0] = -1;

	while (j < pattern_len_ - 1) {
		if (0 > t || pattern_[j] == pattern_[t]) {
			j++;
			t++;
			nextTable[j] =
				pattern_[j] != pattern_[t] ? t : nextTable[t];
		}
		else {
			// ß≈‰
			t = nextTable[t];
		}
	}
}

void SubBytesFinder::slip(ByteBuffer& buf, void* param_cb, ByteBufferHandler handler)
{
	//ArrayList<ByteBuffer> result = new ArrayList<>();
	ByteBuffer last(NULL, 0);
	while (true) {
		buf.DuplicateTo(last);
		if (skipTillPattern(buf)) {
			// Find a available piece
			if (buf.GetPosition() >= pattern_len_ + last.GetPosition()) {
				last.SetLimit(buf.GetPosition() - pattern_len_);
				//result.add(last);
				handler(&last, param_cb);
			}

			// Add a "null" to indicate a pattern is found
			//result.add(null);
			handler(NULL,param_cb);
		}
		else {
			last.SetLimit(buf.GetPosition());
			// result.add(last);
			handler(&last, param_cb);
			break;
		}
	}

	//return result;
}

bool SubBytesFinder::skipTillPattern(ByteBuffer& buf)
{
	if (!buf.hasRemaining()) {
		return false;
	}

	unsigned char b = buf.get();
	while (true) {
		if (patIndex == -1 || b == pattern_[patIndex]) {
			// match
			++patIndex;
			// test all match
			if (patIndex == pattern_len_) {
				patIndex = 0;
				return true;
			}
			else {
				// try read next byte
				if (buf.hasRemaining()) {
					b = buf.get();
				}
				else {
					return false;
				}
			}
		}
		else {
			// skip
			patIndex = nextTable[patIndex];
		}

	}
	// Unreachable statement...
	// return false;
}

bool SubBytesFinder::findIn(ByteBuffer& buf)
{
	ByteBuffer dup(NULL, 0);
	buf.DuplicateTo(dup);
	return skipTillPattern(dup);
}

void SubBytesFinder::reset()
{
	this->patIndex = 0;
}

SubBytesFinder::~SubBytesFinder()
{
	free(this->nextTable);
}

SubBytesFinder::SubBytesFinder(const unsigned char* pattern, const int pattern_len) :
pattern_(pattern),
pattern_len_(pattern_len),
patIndex(0)

{
	this->nextTable = (int*)malloc(sizeof(nextTable[0]) * pattern_len_);
	BuildNextTable();
}
