/*
 * Config.cpp
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source :  https://github.com/Protonerd/FX-SaberOS
 *      Author: neskw
 */
#include "ConfigMenu.h"
#include "Config_HW.h"
#include "Config_SW.h"
#include "Light.h"
#include "SoundFont.h"


// global Saber state and Sub State variables
SaberStateEnum SaberState;
SaberStateEnum PrevSaberState;
ActionModeSubStatesEnum ActionModeSubStates;
ConfigModeSubStatesEnum ConfigModeSubStates;
ActionModeSubStatesEnum PrevActionModeSubStates;
ConfigModeSubStatesEnum PrevConfigModeSubStates;
//SubStateEnum SubState;

extern int8_t modification;
extern bool play;
extern int16_t value;


extern uint8_t ledPins[];
extern cRGB currentColor;
#if defined PIXELBLADE
extern cRGB color;
#endif
extern void HumRelaunch();
extern void SinglePlay_Sound(uint8_t track);
extern void LoopPlay_Sound(uint8_t track);
extern void Pause_Sound();
extern void Resume_Sound();
extern void Set_Loop_Playback();
extern void Set_Volume();
extern void Disable_FTDI(bool ftdi_off);
extern void Disable_MP3(bool mp3_off);
extern void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
    short int multiplier);
extern uint8_t GravityVector();
extern void BatLevel_ConfigEnter();

extern struct StoreStruct {
  // This is for mere detection if they are our settings
  char version[5];
  // The settings
  uint8_t volume;// 0 to 31
  uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
  struct Profile {
    cRGB mainColor;
    cRGB clashColor;
    cRGB blasterboltColor;
    uint16_t swingSensitivity;
    uint8_t flickerType;
    uint8_t poweronoffType;
  }sndProfile[SOUNDFONT_QUANTITY];
}storage;
extern SoundFont soundFont;
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

void NextConfigState(){
  if (ConfigModeSubStates!=CS_MAINCOLOR and ConfigModeSubStates!=CS_CLASHCOLOR and ConfigModeSubStates!=CS_BLASTCOLOR) {  
    lightOff();
  }
  ConfigModeSubStates=ConfigModeSubStates+1; // change to next config state in the ordered list
  if (ConfigModeSubStates== CS_LASTMEMBER) {
    ConfigModeSubStates=0; // after the last config menu item go back to the first
  }

  Serial.println(ConfigModeSubStates);
    
  if (ConfigModeSubStates== (CS_STORAGEACCESS+1)) {
    Disable_FTDI(false);
    delay(200);
    Disable_MP3(false);
    delay(200);
  }
  switch(ConfigModeSubStates) {
      case CS_VOLUME: 
        #if defined LS_FSM
          Serial.print(F("Volume"));
        #endif  
        ConfigModeSubStates=CS_VOLUME;
        BladeMeter(ledPins, storage.volume*100/30);
        SinglePlay_Sound(4);
        delay(500);
        break;
      case CS_SOUNDFONT: 
        #if defined LS_FSM
          Serial.print(F("Sound font"));
        #endif        
        lightOff();
        SinglePlay_Sound(5);
        delay(600);
        SinglePlay_Sound(soundFont.getMenu((storage.soundFont)*NR_FILE_SF));
        delay(500);  
        break;
      case CS_MAINCOLOR: 
        #if defined LS_FSM
          Serial.print(F("Main color"));
        #endif        
        SinglePlay_Sound(6);
        delay(500); 
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        pixelblade_KillKey_Disable();
        lightOn(ledPins, -1, currentColor, NUMPIXELS/2, NUMPIXELS-6);
        break;  
      case CS_CLASHCOLOR: 
        #if defined LS_FSM
          Serial.print(F("Clash color"));
        #endif        
        SinglePlay_Sound(7);
        delay(500); 
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        pixelblade_KillKey_Disable();
        lightOn(ledPins, -1, storage.sndProfile[storage.soundFont].clashColor, 1, NUMPIXELS/2-1); 
        break;
      case CS_BLASTCOLOR: 
        #if defined LS_FSM
          Serial.print(F("Blaster deflect color"));
        #endif        
        SinglePlay_Sound(8);
        delay(500); 
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        pixelblade_KillKey_Disable();
        lightOn(ledPins, -1, storage.sndProfile[storage.soundFont].blasterboltColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4); 
        break;         
      case CS_FLICKERTYPE: 
        #if defined LS_FSM
          Serial.print(F("Flicker type"));
        #endif  
        SinglePlay_Sound(25);
        delay(700);
        LoopPlay_Sound(soundFont.getHum((storage.soundFont)*NR_FILE_SF));
        break;           
      case CS_POWERONOFFTYPE: 
        #if defined LS_FSM
          Serial.print(F("Power on/off type"));
        #endif  
        ConfigModeSubStates=CS_POWERONOFFTYPE;
        SinglePlay_Sound(24);
        delay(500); 
        break; 
      case CS_SWINGSENSITIVITY:
        #if defined LS_FSM
          Serial.print(F("Swing Sensitivity"));
        #endif  
        BladeMeter(ledPins, (storage.sndProfile[storage.soundFont].swingSensitivity)/100);
        SinglePlay_Sound(26);
        delay(500); 
        break;  
      case CS_SLEEPINIT: 
        #if defined LS_FSM
          Serial.print(F("Initialize sleep mode"));
        #endif        
        SinglePlay_Sound(29);
        delay(500);
        break;   
      case CS_BATTERYLEVEL: 
        #if defined LS_FSM
          Serial.print(F("Display battery level"));
        #endif        
        BatLevel_ConfigEnter();
        break;   
      case CS_STORAGEACCESS: 
        #if defined LS_FSM
          Serial.print(F("USB Media storage access"));
        #endif        
        SinglePlay_Sound(28);
        delay(500);
        Disable_FTDI(true); // disable FTDI to be able to manipulate storage media on board via USB
        break;   
      case CS_UARTMODE: 
        #if defined LS_FSM
          Serial.print(F("USB board programming access"));
        #endif        
        SinglePlay_Sound(27);
        delay(500);
        Disable_MP3(true);
        //delay(1000);
        Disable_FTDI(false); // enable FTDI again
        break;   
        }  
}

                        

