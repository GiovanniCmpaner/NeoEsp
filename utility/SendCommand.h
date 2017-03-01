#pragma once

#include <Arduino.h>

#include <Print.h>
#include <Stream.h>

#if !defined DEBUG
#define DEBUG(x)
#endif

template < typename T >
size_t NeoEsp::sendCommand(T command) {
	DEBUG(Serial.print(F("SEND: ")); Serial.print(command); Serial.println(F("[13][10]"));)
		return stream->print(command) + stream->print(F("\r\n"));
}

template < typename T, typename... Ts >
size_t NeoEsp::sendCommand(T command, Ts... parameters) {
	DEBUG(Serial.print(F("SEND: ")); Serial.print(command); Serial.print('=');)
		return stream->print(command) + stream->print('=') + sendParameter(parameters...);
}

template < typename T >
size_t NeoEsp::sendParameter(T parameter) {
	DEBUG(Serial.print(parameter); Serial.println(F("[13][10]"));)
		return stream->print(parameter) + stream->print(F("\r\n"));
}

size_t NeoEsp::sendParameter(const String& parameter) {
	DEBUG(Serial.print(parameter); Serial.println(F("\"[13][10]"));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\"\r\n"));
}

size_t NeoEsp::sendParameter(const __FlashStringHelper* parameter) {
	DEBUG(Serial.print('\"');  Serial.print(parameter); Serial.println(F("\"[13][10]"));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\"\r\n"));
}

size_t NeoEsp::sendParameter(const char parameter[]) {
	DEBUG(Serial.print('\"'); Serial.print(parameter); Serial.println(F("\"[13][10]"));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\"\r\n"));
}

size_t NeoEsp::sendParameter(char parameter[]) {
	DEBUG(Serial.print(parameter); Serial.println(F("\"[13][10]"));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\"\r\n"));
}

template < typename T, typename... Ts >
size_t NeoEsp::sendParameter(T parameter, Ts... parameters) {
	DEBUG(Serial.print(parameter); Serial.print(',');)
		return stream->print(parameter) + stream->print(',') + sendParameter(parameters...);
}

template < typename... Ts >
size_t NeoEsp::sendParameter(const String& parameter, Ts... parameters) {
	DEBUG(Serial.print('\"'); Serial.print(parameter); Serial.print(F("\","));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\",")) + sendParameter(parameters...);
}

template < typename... Ts >
size_t NeoEsp::sendParameter(const __FlashStringHelper* parameter, Ts... parameters) {
	DEBUG(Serial.print('\"'); Serial.print(parameter); Serial.print(F("\","));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\",")) + sendParameter(parameters...);
}

template < typename... Ts >
size_t NeoEsp::sendParameter(const char parameter[], Ts... parameters) {
	DEBUG(Serial.print('\"'); Serial.print(parameter); Serial.print(F("\","));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\",")) + sendParameter(parameters...);
}

template < typename... Ts >
size_t NeoEsp::sendParameter(char parameter[], Ts... parameters) {
	DEBUG(Serial.print('\"'); Serial.print(parameter); Serial.print(F("\","));)
		return stream->print('\"') + stream->print(parameter) + stream->print(F("\",")) + sendParameter(parameters...);
}