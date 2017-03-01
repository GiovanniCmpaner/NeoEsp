#pragma once

enum State {
	IDLE = 0,

	HANDLING_EVENT = (1 << 0),
	GETTING_LENGTH = (1 << 1),
	
	READING_DATA = (1 << 2),
	RECEVING_ID = (1 << 3),
	RECEVING_LENGTH = (1 << 4),

	SENDING_DATA = (1 << 5),
	CANCEL_SEND = (1 << 6)
}  __attribute__((__packed__));

enum ServerType {
	TCP = 0,
	UDP = 1,
}  __attribute__((__packed__));

enum class ConnectionType {
	TCP = 0,
	UDP = 1,
	SSL = 2
}  __attribute__((__packed__));

enum class TaskState {
	IDLE = 0,
	/*PENDING = 1,*/
	EXECUTING = 2,
	DONE = 3
}  __attribute__((__packed__));

enum class TaskResult {
	TIMEOUT = -1,
	OK = 0,
	ERROR = 1,
	FAIL = 2,
	BUSY = 3,
}  __attribute__((__packed__));