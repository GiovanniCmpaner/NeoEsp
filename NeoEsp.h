#pragma once

#include <Arduino.h>

#include "NeoEspConstants.h"

class NeoEsp : public Print {
public:

#include "NeoEspEnums.h"
#include "utility/RingBuffer.h"

	//--------------------------------------Object--------------------------------------//

	NeoEsp(Stream* stream) : stream(stream) {}

	bool begin();
	bool reset();

	//--------------------------------------Commands--------------------------------------//
	
	bool join(const char ssid[], const char password[], const char bssid[] = nullptr);
	bool join(const __FlashStringHelper* ssid, const __FlashStringHelper* password, const __FlashStringHelper* bssid = nullptr);
	bool leave();

	bool serverOn(ServerType type, uint16_t port);
	bool serverOff(ServerType type);
	
	bool close(uint8_t n);
	bool closeAll();

	bool connect(ConnectionType type, int8_t id, uint8_t ip[4], uint16_t port);
	bool connect(ConnectionType type, int8_t id, const char address[], uint16_t port);
	bool connect(ConnectionType type, int8_t id, const __FlashStringHelper* address, uint16_t port);
	bool connect(ConnectionType type, int8_t id, const String& address, uint16_t port);

	bool connected();

	//--------------------------------------AP Commands--------------------------------------//
	
	bool acessPointOn(const char ssid[], const char password[]);
	bool acessPointOff();
	
	//--------------------------------------Events--------------------------------------//
	

	void onReceive(ServerType type, void(*onReceiveCallback)(uint8_t, char));
	void onConnect(ServerType type, void(*onConnectCallback)(uint8_t));
	void onDisconnect(ServerType type, void(*onDisconnectCallback)(uint8_t));

	void poll();

	//--------------------------------------Print--------------------------------------//
	size_t write(uint8_t b) override;
	size_t write(const uint8_t *buffer, size_t size) override;
	uint32_t send(int8_t id, void(*callback)());

	//--------------------------------------Infos--------------------------------------//
	bool getIP(uint8_t* ip, uint8_t* gateway = nullptr, uint8_t* netmask = nullptr);
	bool getMAC(uint8_t* mac);

private:

#include "utility/SendCommand.h"
#include "utility/ReceiveParameter.h"

	//--------------------------------------Tasks--------------------------------------//

	RingBuffer<24> ring;

	TaskResult wait(uint16_t timeout = 1000);
	int8_t wait(PGM_P arg1, PGM_P arg2 = FAIL, PGM_P arg3 = ERROR, PGM_P arg4 = BUSY);
	int8_t wait(uint16_t timeout, PGM_P arg1, PGM_P arg2 = FAIL, PGM_P arg3 = ERROR, PGM_P arg4 = BUSY);

	struct {
		TaskState state = TaskState::IDLE;
		int8_t result;

		PGM_P keys[4] = { nullptr };

		uint32_t timer = 0;
		uint16_t timeout = 1000;
	} task;

	//--------------------------------------Events--------------------------------------//

	void handleConnectEvent(int8_t id);
	void handleDisconnectEvent(int8_t id);
	void handleReceiveEvent(int8_t id, char c);

	//--------------------------------------Control Variables--------------------------------------//
	Stream* const stream;

	State state = State::IDLE;

	uint32_t pollTimer = 0;
	uint32_t available = 0, prepared = 0, sent = 0;
	uint8_t readingId = 0, sendingId = 0;

	uint16_t udpPort = 0;
	int8_t udpId = -1;

	void(*onTcpReceiveCallback)(uint8_t, char) = nullptr;
	void(*onTcpConnectCallback)(uint8_t) = nullptr;
	void(*onTcpDisconnectCallback)(uint8_t) = nullptr;

	void(*onUdpReceiveCallback)(uint8_t, char) = nullptr;
	void(*onUdpConnectCallback)(uint8_t) = nullptr;
	void(*onUdpDisconnectCallback)(uint8_t) = nullptr;
	//--------------------------------------End--------------------------------------//
};
