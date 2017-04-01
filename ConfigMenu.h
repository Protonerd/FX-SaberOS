/*
 * ConfigMenu.h
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 */

#include <Arduino.h>
#if not defined CONFIGMENU_H_
#define CONFIGMENU_H_

#include <WS2812.h>

// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier);

/*
#ifdef COLORS
void confMenuStart(uint16_t variable, uint16_t sound, uint8_t menu);
#else
void confMenuStart(cRGB variable, uint16_t sound, uint8_t menu);
#endif
*/
#endif /* CONFIG_H_ */


