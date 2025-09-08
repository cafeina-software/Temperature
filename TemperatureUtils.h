/*
 * Copyright 2025, cafeina, <cafeina@world>. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef __TEMPERATURE_UTILS__
#define __TEMPERATURE_UTILS__

#include <String.h>
#include <SupportDefs.h>

enum {
	SCALE_CELSIUS = 'c',
	SCALE_FAHRENHEIT = 'f',
	SCALE_KELVIN = 'k',

	SCALE_COUNT = 3
};

inline bool IsValidScale(char opcode) {
	switch(opcode) {
		case SCALE_CELSIUS:
		case SCALE_FAHRENHEIT:
		case SCALE_KELVIN:
			return true;
		default:
			return false;
	}
}

inline BString SymbolForScale(char opcode, int32 addSpacing = 0) {
	static auto symbol = [](char opcode) {
		switch(opcode) {
			case SCALE_CELSIUS: return "°C";
			case SCALE_FAHRENHEIT: return "°F";
			case SCALE_KELVIN: return "K";
			default: return "";
		}
	};

	BString output;
	for(int32 i = 0; i < addSpacing; i++)
		output << " ";
	output << symbol(opcode);

	return output;
}

inline float ConvertToScale(float initial, char initialScale, char finalScale) {
	float converted = 0.0f;
	switch(initialScale) {
		case SCALE_CELSIUS: {
			switch(finalScale) {
				case SCALE_CELSIUS:
					converted = initial; break;
				case SCALE_FAHRENHEIT:
					converted = initial * (9.0f / 5.0f) + 32.0f; break;
				case SCALE_KELVIN:
					converted = initial + 273.15f; break;
			}
			break;
		}
		case SCALE_FAHRENHEIT: {
			switch(finalScale) {
				case SCALE_CELSIUS:
					converted = (initial - 32.0f) * (5.0f / 9.0f); break;
				case SCALE_FAHRENHEIT:
					converted = initial; break;
				case SCALE_KELVIN:
					converted = (initial + 459.67f) * (5.0f / 9.0f); break;
			}
			break;
		}
		case SCALE_KELVIN: {
			switch(finalScale) {
				case SCALE_CELSIUS:
					converted = initial - 273.15f; break;
				case SCALE_FAHRENHEIT:
					converted = initial * (9.0f / 5.0f) - 459.67f; break;
				case SCALE_KELVIN:
					converted = initial; break;
			}
			break;
		}
	}

	return converted;
}

#endif /* __TEMPERATURE_UTILS__ */
