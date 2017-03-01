#pragma once

template < uint16_t BUFFER_SIZE >
class RingBuffer {
public:
	RingBuffer() {}

	void push(char c) {

		*bufferPtr = c;

		if (++bufferPtr >= (buffer + sizeof(buffer)))
			bufferPtr = buffer;
	}

	bool endsWith(const __FlashStringHelper* str) {

		PGM_P p1 = reinterpret_cast<PGM_P>(str) + strlen_P(reinterpret_cast<PGM_P>(str)) - 1;
		char* p2 = bufferPtr;

		while (1) {

			if (--p2 < buffer)
				p2 = buffer + sizeof(buffer) - 1;

			if (pgm_read_byte(p1) != *p2)
				return false;

			if (--p1 < reinterpret_cast<PGM_P>(str))
				return true;
			else if (p2 == bufferPtr)
				return false;
		}

		return false;
	}

	bool endsWith(const String& str) {
		return endsWith(str.c_str());
	}

	bool endsWith(const char str[]) {

		const char* p1 = str + strlen(str) - 1;
		char* p2 = bufferPtr;

		while (1) {

			if (--p2 < buffer)
				p2 = buffer + sizeof(buffer) - 1;

			if (*p1 != *p2)
				return false;

			if (--p1 < str)
				return true;
			else if (p2 == bufferPtr)
				return false;
		}

		return false;
	}

	char getAtRev(uint16_t index) {
		char* p = bufferPtr;
		do {
			if (--p < buffer) {
				p = buffer + sizeof(buffer) - 1;
			}
		} while (index--);

		return *p;
	}

private:
	char buffer[BUFFER_SIZE] = { 0 };
	char* bufferPtr = buffer;
};
