#include "NeoEsp.h"

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------

#define clearState(s) (state &= ~State::s)
#define setState(s) (state |= State::s)
#define inState(s) (state & State::s)

//---------------------------------------------------------------------------------------------

bool NeoEsp::begin() {
	if (inState(HANDLING_EVENT))
		return false;

	uint8_t retry = 0;
	do {
		if (sendCommand(F("AT")), wait() == TaskResult::OK)
			return reset();

		delay(2000);
	} while (++retry < 5);

	return false;
}

//---------------------------------------------------------------------------------------------------

bool NeoEsp::reset() {
	if (inState(HANDLING_EVENT))
		return false;

	return  ((sendCommand(F("AT+RST")), wait(3000, READY) == 0) &&
		(sendCommand(F("ATE0")), wait() == TaskResult::OK) &&
			 (sendCommand(F("AT+CWMODE_CUR"), 1), wait() == TaskResult::OK) &&
			 (sendCommand(F("AT+CIPMUX"), 1), wait() == TaskResult::OK) &&
			 (sendCommand(F("AT+CIPDINFO"), 0), wait() == TaskResult::OK) &&
			 (sendCommand(F("AT+CWAUTOCONN"), 0), wait() == TaskResult::OK) &&
			 (sendCommand(F("AT+CWDHCP_CUR"), 1, 1), wait() == TaskResult::OK));
}

//---------------------------------------------------------------------------------------------------

bool NeoEsp::join(const char ssid[], const char password[], const char bssid[]) {
	if (inState(HANDLING_EVENT))
		return false;

	char escapedSsid[2 * strlen(ssid) + 1];
	memset(escapedSsid, 0, sizeof(escapedSsid));
	char* ssidPtr = escapedSsid;

	char escapedPassword[2 * strlen(password) + 1];
	memset(escapedPassword, 0, sizeof(escapedPassword));
	char* passwordPtr = escapedPassword;

	while (1) {
		char  c = *ssid++;
		if (c == 0) {
			break;
		}
		if (c == '\'' || c == '\"' || c == '\\') {
			*ssidPtr++ = '\\';
		}
		*ssidPtr++ = c;
	}

	while (1) {
		char  c = *password++;
		if (c == 0) {
			break;
		}
		if (c == '\'' || c == '\"' || c == '\\') {
			*passwordPtr++ = '\\';
		}
		*passwordPtr++ = c;
	}

	if (bssid == nullptr)
		sendCommand(F("AT+CWJAP_CUR"), escapedSsid, escapedPassword);
	else
		sendCommand(F("AT+CWJAP_CUR"), escapedSsid, escapedPassword, bssid);

	return (wait(15000) == TaskResult::OK);
}


//---------------------------------------------------------------------------------------------------

bool NeoEsp::join(const __FlashStringHelper* ssid, const __FlashStringHelper* password, const __FlashStringHelper* bssid) {
	if (inState(HANDLING_EVENT))
		return false;

	PGM_P p1 = reinterpret_cast<PGM_P>(ssid);
	PGM_P p2 = reinterpret_cast<PGM_P>(password);

	char escapedSsid[2 * strlen_P(p1) + 1];
	memset(escapedSsid, 0, sizeof(escapedSsid));
	char* ssidPtr = escapedSsid;

	char escapedPassword[2 * strlen_P(p2) + 1];
	memset(escapedPassword, 0, sizeof(escapedPassword));
	char* passwordPtr = escapedPassword;

	while (1) {
		char c = pgm_read_byte(p1++);
		if (c == 0) {
			break;
		}
		if (c == '\'' || c == '\"' || c == '\\') {
			*ssidPtr++ = '\\';
		}
		*ssidPtr++ = c;
	}

	while (1) {
		char c = pgm_read_byte(p2++);
		if (c == 0) {
			break;
		}
		if (c == '\'' || c == '\"' || c == '\\') {
			*passwordPtr++ = '\\';
		}
		*passwordPtr++ = c;
	}

	if (bssid == nullptr)
		sendCommand(F("AT+CWJAP_CUR"), escapedSsid, escapedPassword);
	else
		sendCommand(F("AT+CWJAP_CUR"), escapedSsid, escapedPassword, bssid);

	return (wait(15000) == TaskResult::OK);
}

//---------------------------------------------------------------------------------------------------

bool NeoEsp::leave() {
	if (inState(HANDLING_EVENT))
		return false;

	return (sendCommand(F("AT+CWQAP")), wait() == TaskResult::OK);
}

//---------------------------------------------------------------------------------------------------

void NeoEsp::poll() {
	if (inState(HANDLING_EVENT))
		return;

	if (stream->available()) {
		char c = stream->read();

		pollTimer = millis();
		//DEBUG(//Serial.print("r: "); //Serial.println(c);)

		if (inState(READING_DATA)) {
			////Serial.println("---READING_DATA---");

			handleReceiveEvent(readingId, c);

			if (!--available)
				clearState(READING_DATA);
		}
		else {

			/*
			//Serial.print(" c: ");
			if (c < 30)
				//Serial.print((int)c, DEC);
			else
				//Serial.print(c);
			//Serial.println();*/

			ring.push(c);

			//-----------------------------------
			if (task.state == TaskState::EXECUTING) {

				task.timer = millis();

				for (uint8_t k = 0; k < sizeof(task.keys) / sizeof(*task.keys); k++) {
					if (task.keys[k] != nullptr && ring.endsWith(reinterpret_cast<const __FlashStringHelper*>(task.keys[k]))) {
						task.result = k;
						task.state = TaskState::DONE;
						break;
					}
				}
			}
			//-----------------------------------

			if (inState(RECEVING_ID) && (c >= '0' && c <= '3')) {
				readingId = c - '0';

				clearState(RECEVING_ID);
				setState(RECEVING_LENGTH);
			}
			else if (inState(RECEVING_LENGTH)) {
				if (c >= '0' && c <= '9') {
					available = available * 10 + (c - '0');
				}
				else if (c == ':') {
					clearState(RECEVING_LENGTH);

					if (available)
						setState(READING_DATA);
				}
			}
			else if (ring.endsWith(F(",CONNECT\r\n"))) {
				int8_t id = ring.getAtRev(10) - '0';
				handleConnectEvent(id);
			}
			else if (ring.endsWith(F(",CLOSED\r\n"))) {
				int8_t id = ring.getAtRev(9) - '0';
				handleDisconnectEvent(id);
			}
			else if (ring.endsWith(F(",CONNECT FAIL\r\n"))) {
				int8_t id = ring.getAtRev(15) - '0';
				handleDisconnectEvent(id);
			}
			else if (ring.endsWith(F("+IPD,"))) {
				setState(RECEVING_ID);

			}
		}

	}
	else {
		if (millis() - pollTimer > 30) {
			if (available || inState(READING_DATA) || inState(RECEVING_ID) || inState(RECEVING_LENGTH)) {
				//Serial.println(" !!! READING TIMEOUT !!! ");
				available = 0;
				clearState(READING_DATA);
				clearState(RECEVING_ID);
				clearState(RECEVING_LENGTH);
			}
		}
		if (millis() - task.timer > task.timeout) {
			if (task.state == TaskState::EXECUTING) {
				//Serial.println(" !!!  WAITING TIMEOUT !!! ");

				task.state = TaskState::DONE;
				task.result = static_cast<uint8_t>(TaskResult::TIMEOUT);
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------

size_t NeoEsp::write(uint8_t b) {
	return write(&b, 1);
}

//---------------------------------------------------------------------------------------------------


size_t NeoEsp::write(const uint8_t *buffer, size_t size) {

	if (inState(HANDLING_EVENT) || inState(CANCEL_SEND)) {
		return 0;
	}
	else if (inState(GETTING_LENGTH)) {
		prepared += size;
		return size;
	}

	while (inState(READING_DATA) || inState(RECEVING_ID) || inState(RECEVING_LENGTH)) {
		yield();
		poll();
	}

	if (prepared == sent) {
		prepared = size;
		sent = 0;
	}
	else if (size > prepared - sent) {
		size = prepared - sent;
	}

	size_t n = 0;

	while (size - n) {

		if (sent % 2048 == 0 && prepared > sent) {
			if (sendCommand(F("AT+CIPSEND"), sendingId, min(2048, prepared - sent)), wait(2000) != TaskResult::OK) {
				setState(CANCEL_SEND);
				prepared = sent;
				break;
			}
		}

		size_t packetSent = stream->write(buffer + n, min(2048, min(size - n, 2048 - sent % 2048)));
		n += packetSent;
		sent += packetSent;

		if (sent % 2048 == 0 || prepared == sent) {
			if (wait(2000, PSTR("SEND OK\r\n"), ERROR) != 0) {
				setState(CANCEL_SEND);
				prepared = sent;
				break;
			}
		}
	}

	return n;
}

//---------------------------------------------------------------------------------------------------

bool NeoEsp::serverOn(ServerType type, uint16_t port) {
	if (inState(HANDLING_EVENT))
		return false;

	if (type == ServerType::TCP) {
		serverOff(ServerType::TCP);

		return (sendCommand(F("AT+CIPSERVER"), 1, port), wait() == TaskResult::OK);

	}
	else if (type == ServerType::UDP) {
		serverOff(ServerType::UDP);

		if (port > 0) {
			udpPort = port;
			udpId = -1;

			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------------------------------

bool NeoEsp::serverOff(ServerType type) {
	if (inState(HANDLING_EVENT))
		return false;

	if (type == ServerType::TCP) {
		return ((sendCommand(F("AT+CIPSERVER"), 0), wait() == TaskResult::OK));
	}
	else if (type == ServerType::UDP) {
		if (udpId >= 0 && udpPort > 0)
			return (sendCommand(F("AT+CIPCLOSE"), udpId), wait() == TaskResult::OK);
	}
	return false;
}

//---------------------------------------------------------------------------------------------------

bool NeoEsp::close(uint8_t n) {
	if (inState(HANDLING_EVENT))
		return false;

	if (n >= 0 && n <= 4) {
		sendCommand(F("AT+CIPCLOSE"), n);
		return (wait() == TaskResult::OK);
	}
	return false;
}

//---------------------------------------------------------------------------------------------------


bool NeoEsp::closeAll() {
	if (inState(HANDLING_EVENT))
		return false;

	return ((sendCommand(F("AT+CIPCLOSE"), 5), wait() == TaskResult::OK));
}

//---------------------------------------------------------------------------------------------------


void NeoEsp::onReceive(ServerType type, void(*onReceiveCallback)(uint8_t, char)) {
	if (type == ServerType::TCP)
		this->onTcpReceiveCallback = onReceiveCallback;
	else if (type == ServerType::UDP)
		this->onUdpReceiveCallback = onReceiveCallback;
}

//---------------------------------------------------------------------------------------------------


void NeoEsp::onConnect(ServerType type, void(*onConnectCallback)(uint8_t)) {
	if (type == ServerType::TCP)
		this->onTcpConnectCallback = onConnectCallback;
	else if (type == ServerType::UDP)
		this->onUdpConnectCallback = onConnectCallback;
}

//---------------------------------------------------------------------------------------------------


void NeoEsp::onDisconnect(ServerType type, void(*onDisconnectCallback)(uint8_t)) {
	if (type == ServerType::TCP)
		this->onTcpDisconnectCallback = onDisconnectCallback;
	else if (type == ServerType::UDP)
		this->onUdpDisconnectCallback = onDisconnectCallback;
}

//---------------------------------------------------------------------------------------------------

uint32_t NeoEsp::send(int8_t id, void(*callback)()) {
	if (inState(HANDLING_EVENT) || inState(SENDING_DATA))
		return 0;

	setState(SENDING_DATA);
	clearState(CANCEL_SEND);

	prepared = 0;
	sent = 0;
	sendingId = id;

	setState(GETTING_LENGTH);
	callback();
	clearState(GETTING_LENGTH);
	callback();

	clearState(SENDING_DATA);
	clearState(CANCEL_SEND);
	return sent;
}

//---------------------------------------------------------------------------------------------------


void NeoEsp::handleConnectEvent(int8_t id) {
	if (udpId >= 0 && udpId == id) {
		if (onUdpConnectCallback) {
			setState(HANDLING_EVENT);
			onUdpConnectCallback(id);
			clearState(HANDLING_EVENT);
		}
	}
	else {
		if (onTcpConnectCallback) {
			setState(HANDLING_EVENT);
			onTcpConnectCallback(id);
			clearState(HANDLING_EVENT);
		}
	}
}

//---------------------------------------------------------------------------------------------------


void NeoEsp::handleDisconnectEvent(int8_t id) {

	if (udpId >= 0 && udpId == id) {
		if (onUdpDisconnectCallback) {
			setState(HANDLING_EVENT);
			onUdpDisconnectCallback(id);
			clearState(HANDLING_EVENT);
		}
	}
	else {
		if (onTcpDisconnectCallback) {
			setState(HANDLING_EVENT);
			onTcpDisconnectCallback(id);
			clearState(HANDLING_EVENT);
		}
	}

	if (id == sendingId) {
		setState(CANCEL_SEND);
		prepared = sent;
	}

	if (id == udpId) {
		udpId = -1;
	}
}

//---------------------------------------------------------------------------------------------------


void NeoEsp::handleReceiveEvent(int8_t id, char c) {
	if (udpId >= 0 && udpId == id) {
		if (onUdpReceiveCallback) {
			setState(HANDLING_EVENT);
			onUdpReceiveCallback(id, c);
			clearState(HANDLING_EVENT);
		}
	}
	else {
		if (onTcpReceiveCallback) {
			setState(HANDLING_EVENT);
			onTcpReceiveCallback(id, c);
			clearState(HANDLING_EVENT);
		}
	}
}

//---------------------------------------------------------------------------------------------------
/*
template < typename... Ts >
int16_t NeoEsp::wait(uint16_t timeout, const __FlashStringHelper* ifsh, Ts... args) {

	uint16_t valuesLength = getMaxLength(F(",CONNECT\r\n"), F(",CLOSED\r\n"), F("+IPD,"), ifsh, args...);

	char receiveBuffer[valuesLength + 4];
	memset(receiveBuffer, 0, sizeof(receiveBuffer));



	DEBUG(//Serial.print(F("RECV: "));)

		uint32_t waitTimer = millis();

	int16_t ret = -1;

	while (1) {
		if (stream->available()) {
			waitTimer = millis();

			if (strlen(receiveBuffer) >= sizeof(receiveBuffer) - 1) {
				strcpy(receiveBuffer, &receiveBuffer[1]);
				receiveBuffer[strlen(receiveBuffer)] = 0;
			}

			char c = stream->read();
			////Serial.print(" r3 = "); //Serial.println(c);
			DEBUG(//Serial.print(c);)
				receiveBuffer[strlen(receiveBuffer)] = c;
			////Serial.print(" r1 = "); //Serial.println(c);

			int n = findString(0, receiveBuffer, F(",CONNECT\r\n"), F(",CLOSED\r\n"), F("+IPD,"), ifsh, args...);
			if (n >= 3)
				return n - 3;
			else if (n >= 0) {
				if (n == 0) {
					//Serial.print(" T id = "); //Serial.println(receiveBuffer);
					//Serial.print(" ## EVENT ID = "); //Serial.println(*(strrchr(receiveBuffer, ',') - 1));
					int8_t id = *(strrchr(receiveBuffer, ',') - 1) - '0';

					handleConnectEvent(id);

					memset(receiveBuffer, 0, sizeof(receiveBuffer));
				}
				else if (n == 1) {
					//Serial.print(" T id = "); //Serial.println(receiveBuffer);
					//Serial.print(" ## EVENT ID = "); //Serial.println(*(strrchr(receiveBuffer, ',') - 1));
					int8_t id = *(strrchr(receiveBuffer, ',') - 1) - '0';

					handleDisconnectEvent(id);

					memset(receiveBuffer, 0, sizeof(receiveBuffer));
				}
				else if (n == 2) {
					int8_t id = stream->parseInt();
					uint32_t length = stream->parseInt();

					handleReceiveEvent(id, length);

					memset(receiveBuffer, 0, sizeof(receiveBuffer));
				}
			}
		}
		else if (millis() - waitTimer > timeout) {
			break;
		}
	}

	return ret;
}*/

//---------------------------------------------------------------------------------------------------


//template < typename... Ts >
//int16_t NeoEsp::wait(uint16_t timeout, Ts... args) {
//
//	uint16_t valuesLength = getMaxLength(F(",CONNECT\r\n"), F(",CLOSED\r\n"), F(",CONNECT FAIL\r\n"), F("+IPD,"), args...);
//
//	char receiveBuffer[valuesLength + 4];
//	memset(receiveBuffer, 0, sizeof(receiveBuffer));
//
//	DEBUG(//Serial.print(F("RECV: "));)
//
//		uint32_t waitTimer = millis();
//
//	int16_t ret = -1;
//
//	while (1) {
//		if (stream->available()) {
//			waitTimer = millis();
//
//			if (strlen(receiveBuffer) >= sizeof(receiveBuffer) - 1) {
//				strcpy(receiveBuffer, &receiveBuffer[1]);
//				receiveBuffer[strlen(receiveBuffer)] = 0;
//			}
//
//			char c = stream->read();
//			////Serial.print(" r3 = "); //Serial.println(c);
//			DEBUG(//Serial.print(c);)
//				receiveBuffer[strlen(receiveBuffer)] = c;
//			////Serial.print(" r1 = "); //Serial.println(c);
//
//
//			char* ptr = nullptr;
//
//			int n = locateSubstring(receiveBuffer, &ptr, F(",CONNECT\r\n"), F(",CLOSED\r\n"), F(",CONNECT FAIL\r\n"), F("+IPD,"), args...);
//
//			if (n >= 5)
//				return n - 5;
//			else if (n >= 1) {
//				switch (n) {
//					case 1: {
//						int8_t id = *(ptr - 1) - '0';
//
//						handleConnectEvent(id);
//						break;
//					}  case 2: case 3: {
//						int8_t id = *(ptr - 1) - '0';
//
//						handleDisconnectEvent(id);
//						break;
//					} case 4: {
//						int8_t id = stream->parseInt();
//						uint32_t length = stream->parseInt();
//
//						handleReceiveEvent(id, length);
//						break;
//					}
//				}
//				memset(receiveBuffer, 0, sizeof(receiveBuffer));
//			}
//		}
//		else if (millis() - waitTimer > timeout) {
//			break;
//		}
//	}
//
//	return ret;
//}


//---------------------------------------------------------------------------------------------------


bool NeoEsp::connect(ConnectionType type, int8_t id, uint8_t ip[4], uint16_t port) {
	char temp[16];
	sprintf_P(temp, PSTR("%d.%d.%d.%d"), ip[3], ip[2], ip[1], ip[0]);

	return connect(type, id, temp, port);
}

//---------------------------------------------------------------------------------------------------


bool NeoEsp::connect(ConnectionType type, int8_t id, const __FlashStringHelper* address, uint16_t port) {
	char temp[strlen_P(reinterpret_cast<PGM_P>(address)) + 1];
	strcpy_P(temp, reinterpret_cast<PGM_P>(address));

	return connect(type, id, temp, port);
}

//---------------------------------------------------------------------------------------------------


bool NeoEsp::connect(ConnectionType type, int8_t id, const String& address, uint16_t port) {
	return connect(type, id, address.c_str(), port);
}

//---------------------------------------------------------------------------------------------------


bool NeoEsp::connect(ConnectionType type, int8_t id, const char address[], uint16_t port) {
	if (inState(HANDLING_EVENT))
		return false;

	if (type == ConnectionType::TCP)
		return (sendCommand(F("AT+CIPSTART"), id, F("TCP"), address, port, 10), wait(10000) == TaskResult::OK);
	else if (type == ConnectionType::SSL)
		return (sendCommand(F("AT+CIPSTART"), id, F("SSL"), address, port, 10), wait(10000) == TaskResult::OK);
	else if (type == ConnectionType::UDP) {
		if (udpPort > 0) {
			if (id >= 0 && id <= 3) {
				sendCommand(F("AT+CIPSTART"), id, F("UDP"), address, port, udpPort, 0);

				if (wait(10000) == TaskResult::OK) {
					udpId = id;
					return true;
				}
			}
		}
		else
			return (sendCommand(F("AT+CIPSTART"), id, F("UDP"), address, port), wait(10000) == TaskResult::OK);
	}
	return false;
}

//---------------------------------------------------------------------------------------------------


NeoEsp::TaskResult NeoEsp::wait(uint16_t timeout) {
	return static_cast<TaskResult>(wait(timeout, OK, ERROR, FAIL, BUSY));
}

//---------------------------------------------------------------------------------------------------

int8_t NeoEsp::wait(PGM_P arg1, PGM_P arg2, PGM_P arg3, PGM_P arg4) {
	return wait(1000, arg1, arg2, arg3, arg4);
}

//---------------------------------------------------------------------------------------------------

int8_t NeoEsp::wait(uint16_t timeout, PGM_P arg1, PGM_P arg2, PGM_P arg3, PGM_P arg4) {

	task.state = TaskState::EXECUTING;

	task.keys[0] = arg1;
	task.keys[1] = arg2;
	task.keys[2] = arg3;
	task.keys[3] = arg4;

	task.timeout = timeout;
	task.timer = millis();

	while (task.state == TaskState::EXECUTING) {
		yield();
		poll();
	}

	return task.result;
}


//---------------------------------------------------------------------------------------------------

bool NeoEsp::getIP(uint8_t* ip, uint8_t* gateway, uint8_t* netmask) {
	if (inState(HANDLING_EVENT))
		return false;

	sendCommand(F("AT+CIPSTA_CUR?"));
	if (wait(5000, PSTR("+CIPSTA_CUR:ip:")) != 0)
		return false;
	if (ip != nullptr && !receiveParameter(&ip[3], &ip[2], &ip[1], &ip[0]))
		return false;
	if (wait(2000, PSTR("+CIPSTA_CUR:gateway:")) != 0)
		return false;
	if (gateway != nullptr && !receiveParameter(&gateway[3], &gateway[2], &gateway[1], &gateway[0]))
		return false;
	if (wait(2000, PSTR("+CIPSTA_CUR:netmask:")) != 0)
		return false;
	if (netmask != nullptr && !receiveParameter(&netmask[3], &netmask[2], &netmask[1], &netmask[0]))
		return false;

	wait(2000);
	return true;
}

//---------------------------------------------------------------------------------------------------
bool NeoEsp::getMAC(uint8_t* mac) {
	if (inState(HANDLING_EVENT))
		return false;

	sendCommand(F("AT+CIPSTAMAC_CUR?"));
	if (wait(5000, PSTR("+CIPSTAMAC_CUR:")) != 0)
		return false;
	if (!receiveParameter(&mac[5], HEX, &mac[4], HEX, &mac[3], HEX, &mac[2], HEX, &mac[1], HEX, &mac[0], HEX))
		return false;

	wait(2000);
	return true;
}
//---------------------------------------------------------------------------------------------------
bool NeoEsp::connected() {
	if (inState(HANDLING_EVENT))
		return false;

	sendCommand(F("AT+CIPSTATUS"));
	if (wait(5000, PSTR("STATUS:")) != 0)
		return false;

	uint8_t status;
	if (!receiveParameter(&status))
		return false;

	return (status == 2 || status == 3);
}
//---------------------------------------------------------------------------------------------------

#undef clearState
#undef setState
#undef inState

//---------------------------------------------------------------------------------------------------