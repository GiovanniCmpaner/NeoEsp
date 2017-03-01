#pragma once

#if !defined DEBUG
#define DEBUG(x)
#endif

bool NeoEsp::receiveParameter(char** str) {
	bool isReceiving = false;

	uint32_t timeout = millis();
	while (1) {
		if (stream->available()) {
			char c = stream->read();

			timeout = millis();

			if (isReceiving) {
				if (c == '\"') {
					return true;
				}
				else {
					**str++ = c;
				}
			}
			else if (c == '\"') {
				isReceiving = true;
			}
			else {
				break;
			}

		}
		else if (millis() - timeout > 15)
			break;
	}

	return false;
}

bool NeoEsp::receiveParameter(uint8_t* b, int base = DEC){
	uint32_t t;
	if (receiveParameter(&t,base)) {
		*b = t;
		return true;
	}
	return false;
}

bool NeoEsp::receiveParameter(int8_t* n, int base = DEC){
	int32_t t;
	if (receiveParameter(&t, base)) {
		*n = t;
		return true;
	}
	return false;
}

bool NeoEsp::receiveParameter(uint16_t* n, int base = DEC){
	uint32_t t;
	if (receiveParameter(&t, base)) {
		*n = t;
		return true;
	}
	return false;
}

bool NeoEsp::receiveParameter(int16_t* n, int base = DEC) {
	int32_t t;
	if (receiveParameter(&t, base)) {
		*n = t;
		return true;
	}
	return false;
}

bool NeoEsp::receiveParameter(int32_t* n, int base = DEC){
	bool isReceiving = false;
	bool isNegative = false;
	int32_t value = 0;

	uint32_t timeout = millis();
	while (1) {
		if (stream->available()) {
			char c = stream->read();

			timeout = millis();

			if (c >= '0' && c <= '9') {
				value = value * base + (c - '0');
				isReceiving = true;
			}
			else if (c >= 'A' && c <= 'Z') {
				value = value * base + (c - 'A' + 10);
				isReceiving = true;
			}
			else if (c >= 'a' && c <= 'z') {
				value = value * base + (c - 'a' + 10);
				isReceiving = true;
			}
			else if (!isReceiving && c == '+') {
				isNegative = false;
				isReceiving = true;
			}
			else if (!isReceiving && c == '-') {
				isNegative = true;
				isReceiving = true;
			}
			else if(isReceiving){
				if (isNegative) {
					value = -value;
				}
				*n = value;
				return true;
			}
		}
		else if (millis() - timeout > 15) {
			//Serial.println(" +++ parse timeout +++ ");
			break;
		}
	}

	return false;
}

bool NeoEsp::receiveParameter(uint32_t* n, int base = DEC){
	bool isReceiving = false;
	uint32_t value = 0;

	uint32_t timeout = millis();
	while (1) {
		if (stream->available()) {
			char c = stream->read();

			timeout = millis();

			if (c >= '0' && c <= '9') {
				value = value * base + (c - '0');
				isReceiving = true;
			}
			else if (c >= 'A' && c <= 'Z') {
				value = value * base + (c - 'A' + 10);
				isReceiving = true;
			}
			else if (c >= 'a' && c <= 'z') {
				value = value * base + (c - 'a' + 10);
				isReceiving = true;
			}
			else if (isReceiving) {
				*n = value;
				return true;
			}
		}
		else if (millis() - timeout > 15) {
			//Serial.println(" +++ parse timeout +++ ");
			break;
		}
	}

	return false;
}

template < typename T, typename... Ts >
bool NeoEsp::receiveParameter(T arg, Ts... args) {
	return (receiveParameter(arg) && receiveParameter(args...));
}

template < typename T, typename... Ts >
bool NeoEsp::receiveParameter(T arg, int base, Ts... args) {
	return (receiveParameter(arg, base) && receiveParameter(args...));
}