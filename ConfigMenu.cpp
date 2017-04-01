/*
 * Config.cpp
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 */
#include "ConfigMenu.h"
#include "Config.h"
#include "Light.h"

#if defined PIXELBLADE
#include <WS2812.h>
#endif

extern int8_t modification;
extern bool play;
extern int16_t value;
extern void SinglePlay_Sound(uint8_t track);
extern void LoopPlay_Sound(uint8_t track);
// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

// this function ensures that config menu items which have values between a min and a max value
// wrap back to min/max upon reaching max/min. It also plays a sound notifying the user if either min or max value has beeb reached.
// This function is also in charge of changing the actual value of a setting via the value global variable.
void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier) {

	value = variable + (multiplier * 1);

	if (value < (int) min) {
		value = max;
	} else if (value > (int) max) {
		value = min;
	} else if (value == (int) min and play) {
		play = false;
		SinglePlay_Sound(10);
		delay(150);
	} else if (value == (int) max and play) {
		play = false;
		SinglePlay_Sound(9);
		delay(150);
	}
} //confParseValue

// this functions parses in the value of the config variable and based on it plays sounds or activates LEDs
/*#ifdef COLORS
void confMenuStart(uint16_t variable, uint16_t sound, uint8_t menu) {
#else
void confMenuStart(cRGB variable, uint16_t sound, uint8_t menu) {
#endif
  extern uint8_t ledPins[];
#if defined STAR_LED
	extern uint8_t currentColor[];
#endif
#if defined PIXELBLADE
	extern cRGB currentColor;
#endif
	extern bool enterMenu;
	if (enterMenu) {
		SinglePlay_Sound(sound);
		delay(500);

		switch (menu) {
    case 0:
#if defined LS_INFO
      Serial.print(F("SNDFT\nCur:"));
#endif
#if defined LEDSTRINGS
      lightOff();
      lightOn(ledPins, 1);
#endif
      break;
    case 1:
#if defined LS_INFO
			Serial.print(F("VOL\nCur:"));
#endif
#if defined LEDSTRINGS
			lightOff();
			lightOn(ledPins, 0);
#endif
			break;
#if defined STAR_LED
    case 2:
			lightOff(ledPins);
#if defined LS_INFO
			Serial.print(F("COLOR1\nCur:"));
#endif
			getColor(currentColor, variable);
			lightOn(ledPins, currentColor);
			break;
		case 3:
			lightOff(ledPins);
#if defined LS_INFO
			Serial.print(F("COLOR2\nCur:"));
#endif
			getColor(currentColor, variable);
			lightOn(ledPins, currentColor);
			break;
    case 4:
      lightOff(ledPins);
#if defined LS_INFO
      Serial.print(F("COLOR3\nCur:"));
#endif
      getColor(currentColor, variable);
      lightOn(ledPins, currentColor);
      break;
#endif

#if defined PIXELBLADE
    case 2:
      lightOff();

#if defined LS_INFO
      Serial.print(F("COLOR1\nCur:"));
#endif
      getColor(variable);
      for (uint8_t i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      lightOn(currentColor);
      break;
    case 3:
			lightOff();

#if defined LS_INFO
			Serial.print(F("COLOR2\nCur:"));
#endif
			getColor(variable);
			for (uint8_t i = 0; i < 6; i++) {
				digitalWrite(ledPins[i], HIGH);
			}
			lightOn(currentColor);
			break;
		case 4:
			lightOff();
#if defined LS_INFO
			Serial.print(F("COLOR3\nCur:"));
#endif
			getColor(variable);
      for (uint8_t i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      lightOn(currentColor);
      break;
#endif
		}

#if defined LS_INFO
		Serial.println(variable);
#endif
		value = variable;
		enterMenu = false;
		delay(100);
	}
} //confMenuStart
*/                            

