/*
 * Config.h
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source :  https://github.com/Protonerd/FX-SaberOS
 */

#include <Arduino.h>
#include "Config_HW.h"
#include "Config_SW.h"

#if not defined CONFIGMENU_H_
#define CONFIGMENU_H_

#include <WS2812.h>

enum SaberStateEnum {S_STANDBY, S_SABERON, S_CONFIG, S_SLEEP, S_JUKEBOX};
enum ActionModeSubStatesEnum {AS_HUM, AS_IGNITION, AS_RETRACTION, AS_BLADELOCKUP, AS_PREBLADELOCKUP, AS_BLASTERDEFLECTMOTION, AS_BLASTERDEFLECTPRESS, AS_CLASH, AS_SWING, AS_SPIN, AS_FORCE, AS_LASTMEMBER}; // never delete AS_LASTMEMBER!!! Needed to calculate number of elements in the enum type!!!

// configure the config menu based on the blade type
#if defined LEDSTRINGS
enum ConfigModeSubStatesEnum {CS_SOUNDFONT, CS_FLICKERTYPE, CS_SLEEPINIT, CS_LASTMEMBER, CS_VOLUME, CS_POWERONOFFTYPE, CS_SWINGSENSITIVITY, CS_MAINCOLOR, CS_CLASHCOLOR, CS_BLASTCOLOR, CS_BATTERYLEVEL, CS_STORAGEACCESS, CS_UARTMODE}; // never delete CS_LASTMEMBER!!! Needed to calculate number of elements in the enum type!!!
#endif
#if defined STAR_LED or defined ADF_PIXIE_BLADE
enum ConfigModeSubStatesEnum {CS_SOUNDFONT, CS_FLICKERTYPE, CS_MAINCOLOR, CS_CLASHCOLOR, CS_BLASTCOLOR, CS_SWINGSENSITIVITY, CS_LASTMEMBER, CS_VOLUME, CS_SLEEPINIT, CS_BATTERYLEVEL, CS_POWERONOFFTYPE, CS_STORAGEACCESS, CS_UARTMODE}; // never delete CS_LASTMEMBER!!! Needed to calculate number of elements in the enum type!!!
#endif
#if defined PIXELBLADE
enum ConfigModeSubStatesEnum {CS_SOUNDFONT, CS_SWINGSENSITIVITY, CS_FLICKERTYPE, CS_MAINCOLOR, CS_CLASHCOLOR, CS_BLASTCOLOR, CS_SLEEPINIT, CS_LASTMEMBER, CS_VOLUME, CS_POWERONOFFTYPE, CS_BATTERYLEVEL, CS_STORAGEACCESS, CS_UARTMODE}; // never delete CS_LASTMEMBER!!! Needed to calculate number of elements in the enum type!!!
#endif

// ====================================================================================
// ===           	  	 			CONFIG MODE FUNCTIONS	                		===
// ====================================================================================

void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
		short int multiplier);

void NextConfigState();

#endif /* CONFIG_H_ */


