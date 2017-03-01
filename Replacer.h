#pragma once

#include <Arduino.h>
#include <Print.h>

/*
  #define CAT( A, B ) A ## B
  #define SELECT( NAME, NUM ) CAT( NAME ## _, NUM )
  #define GET_COUNT( _1, _2, _3, _4, _5, _6 , COUNT, ... ) COUNT
  #define VA_SIZE( ... ) GET_COUNT( __VA_ARGS__, 6, 5, 4, 3, 2, 1 )
  #define VA_SELECT( NAME, ... ) SELECT( NAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)

  #define KeyValuePair( ... ) VA_SELECT( KeyValuePair, __VA_ARGS__ )
  #define KeyValuePair_1( X ) #X, X
  #define KeyValuePair_2( X, Y ) #X, X, Y
*/

  /*
#if defined(__AVR__)
	
#elif defined(__arm__)

#endif
*/

//---------------------------------------------------------------------------------------------------

class Replacer {
    public:

        template < typename... Ts >
        static void print(Print* print, PGM_P text, char delimiter, Ts... args);

    private:

        static void comparePrint(Print* print, PGM_P text, size_t length);
		
        template < typename T1, typename T2, typename... Ts >
        static void comparePrint(Print* print, PGM_P text, size_t length, T1 key, T2 value, Ts... args);

        template < typename T1, typename T2, typename... Ts >
        static void comparePrint(Print* print, PGM_P text, size_t length, T1 key, T2 value, int format, Ts... args);

        static bool equals(PGM_P str1, const char str2[], size_t length);

        static bool equals(PGM_P str1, const __FlashStringHelper* str2, size_t length);

        static bool equals(PGM_P str1, const String& str2, size_t length);
};

//---------------------------------------------------------------------------------------------------

template < typename... Ts >
void Replacer::print(Print* print, PGM_P text, char delimiter, Ts... args) {

	PGM_P ptr1 = text;
	PGM_P ptr2 = text;

	while (1) {
		
		while(1) {
			char c = pgm_read_byte(ptr2++);
			if(c == 0)
				return;
			else if (c != delimiter) 
				print->write(c);
			else
				break;
		}
		
		ptr1 = ptr2;

		while(1) {
			char c = pgm_read_byte(ptr2++);
			if(c == 0)
				return;
			else if(c == delimiter){
				comparePrint(print, ptr1, ptr2 - ptr1 - 1, args...);
				break;
			}	
		}
	}
}

//---------------------------------------------------------------------------------------------------

void Replacer::comparePrint(Print* print, PGM_P text, size_t length) {
	(void) print; (void) text; (void) length;
}

//---------------------------------------------------------------------------------------------------

template < typename T1, typename T2, typename... Ts >
void Replacer::comparePrint(Print* print, PGM_P text, size_t length, T1 key, T2 value, Ts... args) {
	if (equals(text, key, length)) {
		print->print(value);
	} else {
		comparePrint(print, text, length, args...);
	}
}

//---------------------------------------------------------------------------------------------------

template < typename T1, typename T2, typename... Ts >
void Replacer::comparePrint(Print* print, PGM_P text, size_t length, T1 key, T2 value, int format, Ts... args) {
	if (equals(text, key, length)) {
		print->print(value, format);
	} else {
		comparePrint(print, text, length, args...);
	}
}

//---------------------------------------------------------------------------------------------------

bool Replacer::equals(PGM_P str1, const char str2[], size_t length) {
	while (length--) {
		char c1 = pgm_read_byte(str1++);
		char c2 = *str2++;
		if (c1 == 0 && c2 == 0) {
			return true;
		} else if (c1 == 0 || c2 == 0 || c1 != c2) {
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------------

bool Replacer::equals(PGM_P str1, const __FlashStringHelper* str2, size_t length) {
	PGM_P p = (PGM_P) str2;
	while (length--) {
		char c1 = pgm_read_byte(str1++);
		char c2 = pgm_read_byte(p++);
		if (c1 == 0 && c2 == 0) {
			return true;
		} else if (c1 == 0 || c2 == 0 || c1 != c2) {
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------------

bool Replacer::equals(PGM_P str1, const String& str2, size_t length) {
	const char* p = str2.c_str();
	while (length--) {
		char c1 = pgm_read_byte(str1++);
		char c2 = *p++;
		if (c1 == 0 && c2 == 0) {
			return true;
		} else if (c1 == 0 || c2 == 0 || c1 != c2) {
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------------
