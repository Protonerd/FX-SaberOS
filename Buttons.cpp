/*
 * Buttons.c
 *
 *  Created on: 21 Octber 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source :  https://github.com/Protonerd/FX-SaberOS
://github.com/neskweek/LightSaberOS
 */

#include "Buttons.h"
#include "Config_HW.h"
#include "Config_SW.h"
#include "SoundFont.h"
#include "Light.h"
#include "ConfigMenu.h"


extern SoundFont soundFont;

// global Saber state and Sub State variables
extern SaberStateEnum SaberState;
extern SaberStateEnum PrevSaberState;
extern ActionModeSubStatesEnum ActionModeSubStates;
extern ConfigModeSubStatesEnum ConfigModeSubStates;
extern ActionModeSubStatesEnum PrevActionModeSubStates;
extern ConfigModeSubStatesEnum PrevConfigModeSubStates;
//extern SubStateEnum SubState;


extern unsigned long sndSuppress;
extern bool hum_playing;
extern int8_t modification;
extern bool play;
extern int16_t value;
#ifdef JUKEBOX
extern bool jukebox_play;
extern uint8_t jb_track;
#endif
extern bool lockuponclash;
extern bool changeMenu;
extern uint8_t menu;
extern bool enterMenu;
#if defined LEDSTRINGS
extern uint8_t ledPins[];
#endif
#if defined STAR_LED
extern uint8_t ledPins[];
extern cRGB currentColor;
//extern uint8_t currentColor[4]; //0:Red 1:Green 2:Blue 3:ColorID
#endif
#if defined PIXELBLADE or defined ADF_PIXIE_BLADE
extern uint8_t ledPins[];
extern cRGB color;
extern cRGB currentColor;
#endif
extern void HumRelaunch();
extern void SinglePlay_Sound(uint8_t track);
extern void LoopPlay_Sound(uint8_t track);
extern void Pause_Sound();
extern void Resume_Sound();
extern void Set_Loop_Playback();
extern void Set_Volume(int8_t volumeSet);
extern void Disable_FTDI(bool ftdi_off);
extern void Disable_MP3(bool mp3_off);
extern void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
    short int multiplier);
extern uint8_t GravityVector();

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

extern bool fireblade;
// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

void ConfigMenuButtonEventHandler(bool SaturateColor, ButtonActionEnum ButtonActionType, int8_t incrementSign=1){
    if (ConfigModeSubStates == CS_VOLUME and ButtonActionType==SINGLE_CLICK) {
      confParseValue(storage.volume, 5, 30, 1*incrementSign);
      storage.volume = value;
      BladeMeter(ledPins, value*100/30);
      Set_Volume(storage.volume);
      #if defined LS_INFO
              Serial.println(storage.volume);
      #endif      
    }
    else if (ConfigModeSubStates == CS_SOUNDFONT and ButtonActionType==SINGLE_CLICK) {
      play = false;
      confParseValue(storage.soundFont, 0, SOUNDFONT_QUANTITY - 1, 1*incrementSign);
      storage.soundFont = value;
      soundFont.setID(value);
      SinglePlay_Sound(soundFont.getMenu((storage.soundFont)*NR_FILE_SF));
      //Serial.print("soundfont   "); Serial.print(storage.soundFont); Serial.print("  Offset:   ");Serial.println(soundFont.getMenu((storage.soundFont)*NR_FILE_SF));
      delay(150);    
    }
    #if defined PIXELBLADE or defined STAR_LED or defined ADF_PIXIE_BLADE
    else if (ConfigModeSubStates==CS_MAINCOLOR) {
      #ifdef GRAVITY_COLOR
        ColorMixing(storage.sndProfile[storage.soundFont].mainColor,modification,MAX_BRIGHTNESS, SaturateColor);
      #else if COLOR_PROFILE
        if (ButtonActionType==SINGLE_CLICK){
          confParseValue(modification, 0, 14, incrementSign);
          modification = value;
          getColorFix(modification);
        }
      #endif
      storage.sndProfile[storage.soundFont].mainColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].mainColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].mainColor.b=currentColor.b;
      lightOn(ledPins, -1, currentColor, NUMPIXELS/2, NUMPIXELS-6);
      delay(50);
    
      }
    else if (ConfigModeSubStates==CS_CLASHCOLOR) {
      #ifdef GRAVITY_COLOR
        ColorMixing(storage.sndProfile[storage.soundFont].clashColor,modification,MAX_BRIGHTNESS, SaturateColor);
      #else if COLOR_PROFILE
        if (ButtonActionType==SINGLE_CLICK){
          confParseValue(modification, 0, 14, incrementSign);
          modification = value;
          getColorFix(modification);
        }
      #endif 
      storage.sndProfile[storage.soundFont].clashColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].clashColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].clashColor.b=currentColor.b;
      lightOn(ledPins, -1, currentColor, 1, NUMPIXELS/2-1);
      delay(50);
    }
    else if (ConfigModeSubStates==CS_BLASTCOLOR) {
      #ifdef GRAVITY_COLOR
        ColorMixing(storage.sndProfile[storage.soundFont].blasterboltColor,modification,MAX_BRIGHTNESS, SaturateColor);
      #else if COLOR_PROFILE
        if (ButtonActionType==SINGLE_CLICK){
          confParseValue(modification, 0, 14, incrementSign);
          modification = value;
          getColorFix(modification);
        }
      #endif 
      storage.sndProfile[storage.soundFont].blasterboltColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].blasterboltColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].blasterboltColor.b=currentColor.b;
      lightOn(ledPins, -1, currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
      delay(50);
    }
  #endif // PIXELBLADE or STAR_LED  
    else if (ConfigModeSubStates == CS_FLICKERTYPE and ButtonActionType==SINGLE_CLICK) {
      #ifdef LEDSTRINGS
        confParseValue(storage.sndProfile[storage.soundFont].flickerType, 0, 2, 1*incrementSign); // max number of flicker types for LEDSTRINGS currently 3
      #endif
      #ifdef STAR_LED
        confParseValue(storage.sndProfile[storage.soundFont].flickerType, 0, 2, 1*incrementSign); // max number of flicker types for STAR_LED currently 3
      #endif
      #ifdef ADF_PIXIE_BLADE
        confParseValue(storage.sndProfile[storage.soundFont].flickerType, 0, 0, 1*incrementSign); // max number of flicker types for Adafruit Pixie currently 1
      #endif
      #ifdef PIXELBLADE
        confParseValue(storage.sndProfile[storage.soundFont].flickerType, 0, 4, 1*incrementSign); // max number of flicker types for PIXELBLADE currently 5
      #endif
      storage.sndProfile[storage.soundFont].flickerType = value;
      #if defined PIXELBLADE
        if (value==2 or value==3 or value==4) {fireblade=true;}
        else {fireblade=false;}
      #endif
      #if defined LS_INFO
        Serial.println(storage.sndProfile[storage.soundFont].flickerType);
      #endif      
    }
    else if (ConfigModeSubStates == CS_SWINGSENSITIVITY and ButtonActionType==SINGLE_CLICK) {
      // 2048LSB/g, -32k to +32k, but usable range is ~16384(=1g acceleration), increment with 1/100th of a g
      confParseValue(storage.sndProfile[storage.soundFont].swingSensitivity, 0, 16000, 160*incrementSign);
      storage.sndProfile[storage.soundFont].swingSensitivity = value;
      #if defined LS_INFO
        Serial.println(storage.sndProfile[storage.soundFont].swingSensitivity);
      #endif
      BladeMeter(ledPins, (storage.sndProfile[storage.soundFont].swingSensitivity)/100);     
    }
}

void mainClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button click."));
#endif
	if (SaberState==S_SABERON) {
    /*
    if (lockuponclash) {
      HumRelaunch();
      ActionModeSubStates = AS_HUM;
      lockuponclash=false;
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("End clash triggered lockup (either pre or active phase)"));
      #endif  
    }
    else {
      lockuponclash=true;
#if defined LS_BUTTON_DEBUG
      Serial.println(F("Start clash triggered lockup (either pre or active phase)"));
#endif 
    }*/
    #if defined LS_BUTTON_DEBUG
      Serial.println(F("Start motion triggered blaster bolt deflect"));
    #endif
    if (ActionModeSubStates!=AS_BLASTERDEFLECTMOTION) { // start motion triggered blaster deflect
      ActionModeSubStates=AS_BLASTERDEFLECTMOTION;
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Start motion triggered blaster bolt deflect"));
      #endif
    }
    else { // stop motion triggered blaster deflect
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("End motion triggered blaster bolt deflect"));
      #endif
      HumRelaunch();
      ActionModeSubStates=AS_HUM;
      accentLEDControl(AL_ON);
    } 
  }
	else if (SaberState==S_CONFIG) {
    #ifdef DEEP_SLEEP
    if (ConfigModeSubStates==CS_SLEEPINIT) {
        SaberState=S_SLEEP;
        PrevSaberState=S_CONFIG;
        // play a beep 3 times
          SinglePlay_Sound(1);
          delay(500);
          SinglePlay_Sound(1);
          delay(500);    
          SinglePlay_Sound(1);
          delay(500);
    }
    #endif // DEEP_SLEEP
    if (ConfigModeSubStates != CS_FLICKERTYPE) {
      SinglePlay_Sound(1);
      delay(50);
    }
    ConfigMenuButtonEventHandler(true, SINGLE_CLICK);
	}
	else if (SaberState==S_STANDBY) {
		// LightSaber poweron
   SaberState=S_SABERON;
   PrevSaberState=S_STANDBY;
   ActionModeSubStates=AS_IGNITION;
		//actionMode = true;
	}
#ifdef JUKEBOX 
  else if (SaberState==S_JUKEBOX) {
#ifdef SINGLEBUTTON
    #if defined LS_BUTTON_DEBUG
        Serial.print(F("Play/Pause current song "));Serial.print(jb_track);
    #endif
    if (jukebox_play) {
      // pause the song
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Pause Song"));
      #endif
      jukebox_play=false;
      Pause_Sound();
    } else {
      // resume playing the song
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Resume Song"));
      #endif
      jukebox_play=true;
      Resume_Sound();
    }
#else // two button setup (main + aux)
    #if defined LS_BUTTON_DEBUG
        Serial.print(F("Next JukeBox sound file "));Serial.print(jb_track);
    #endif    // jump to next song and start playing it
    if (jb_track==NR_CONFIGFOLDERFILES+NR_JUKEBOXSONGS) {
      jb_track=NR_CONFIGFOLDERFILES+1;  // fold back to first song in the dir designated for music playback
    }
    else {
      jb_track++;
    }
    SinglePlay_Sound(jb_track);
#endif // SINGLEBUTTON
  }
#endif // JUKEBOX
} // mainClick

void mainDoubleClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button double click."));
#endif
#ifdef SINGLEBUTTON
	if (SaberState==S_SABERON) {
		//ACTION TO DEFINE
    #if defined LS_BUTTON_DEBUG
      Serial.println(F("Start motion triggered blaster bolt deflect"));
    #endif
    if (ActionModeSubStates!=AS_BLASTERDEFLECTMOTION) { // start motion triggered blaster deflect
      ActionModeSubStates=AS_BLASTERDEFLECTMOTION;
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Start motion triggered blaster bolt deflect"));
      #endif
    }
    else { // stop motion triggered blaster deflect
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("End motion triggered blaster bolt deflect"));
      #endif
      HumRelaunch();
      ActionModeSubStates=AS_HUM;
      accentLEDControl(AL_ON);
    }    
} else if (SaberState==S_CONFIG) {
// Change Menu
    NextConfigState();
  }
#ifdef JUKEBOX 
  else if (SaberState==S_STANDBY) {
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Enter JukeBox"));
      #endif
      SaberState=S_JUKEBOX;
      PrevSaberState=S_STANDBY;
    //ACTION TO DEFINE
  } else if (SaberState==S_JUKEBOX) {
//Entering JukeBox mode (MP3 player)
    SaberState=S_STANDBY;
    PrevSaberState=S_JUKEBOX;
// stop/pause track being played
    Pause_Sound();
  }
#endif  // JUKEBOX
#else  // not SINGLEBUTTON
#endif  // SINGLEBUTTON
} // mainDoubleClick

void mainLongPressStart() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress start"));
#endif
	if (SaberState==S_SABERON) {
    // LightSaber switch-off
    ActionModeSubStates=AS_RETRACTION;
    SaberState=S_STANDBY;
    PrevSaberState=S_SABERON;
	} else if (SaberState==S_CONFIG) {
#ifndef SINGLEBUTTON
// Change Menu
    NextConfigState();
#else  // SINGLEBUTTON
//Leaving Config Mode
  if (ConfigModeSubStates!=CS_MAINCOLOR and ConfigModeSubStates!=CS_CLASHCOLOR and ConfigModeSubStates!=CS_BLASTCOLOR) {
    changeMenu = false;
    SaberState=S_STANDBY;
    PrevSaberState=S_CONFIG;
    #ifdef PIXELBLADE
      pixelblade_KillKey_Disable();
    #endif
    Set_Volume(storage.volume);
    delay(200);
  }
#endif
  }
#ifdef SINGLEBUTTON
#ifdef JUKEBOX
  else if (SaberState==S_JUKEBOX) {

#if defined LS_BUTTON_DEBUG
    Serial.print(F("Next JukeBox sound file "));Serial.print(jb_track);
#endif    // jump to next song and start playing it
    if (jb_track==NR_CONFIGFOLDERFILES+NR_JUKEBOXSONGS) {
      jb_track=NR_CONFIGFOLDERFILES+1;  // fold back to first song in the dir designated for music playback
    }
    else {
      jb_track++;
    }
    SinglePlay_Sound(jb_track);
  }
#endif // JUKEBOX 
#endif // SINGLEBUTTON


#ifdef SINGLEBUTTON
  else if (SaberState==S_STANDBY) {
    //Entering Config Mode
    SaberState=S_CONFIG;
    PrevSaberState=S_STANDBY; 
    #ifdef PIXELBLADE 
      pixelblade_KillKey_Enable();     
    #endif
	}
#endif
} // mainLongPressStart

void mainLongPress() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress..."));
#endif
	if (SaberState==S_SABERON) {
	} else if (SaberState==S_CONFIG) {
    ConfigMenuButtonEventHandler(false, LONGPRESS);
	}
	else if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // mainLongPress

void mainLongPressStop() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button longPress stop"));
#endif
} // mainLongPressStop

#ifndef SINGLEBUTTON
void lockupClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button click."));
#endif
	if (SaberState==S_SABERON) {
    // Blaster bolt deflect
    #if defined LS_BUTTON_DEBUG
      Serial.println(F("Start button activated blaster bolt deflect"));
    #endif
    ActionModeSubStates=AS_BLASTERDEFLECTPRESS;
	} else if (SaberState==S_CONFIG) {
    if (ConfigModeSubStates != CS_FLICKERTYPE) {
      SinglePlay_Sound(2);
      delay(50);
    }
      ConfigMenuButtonEventHandler(true, SINGLE_CLICK,-1);

	} 
#ifdef JUKEBOX
  else if (SaberState==S_JUKEBOX) {
    if (jukebox_play) {
      // pause the song
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Pause Song"));
      #endif
      jukebox_play=false;
      Pause_Sound();
    } else {
      // resume playing the song
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Resume Song"));
      #endif
      jukebox_play=true;
      Resume_Sound();
    }
  }
#endif  // JUKEBOX
} // lockupClick

void lockupDoubleClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button double click."));
#endif
	if (SaberState==S_SABERON) {
    #if defined LS_BUTTON_DEBUG
      Serial.println(F("Start motion triggered blaster bolt deflect"));
    #endif
    if (ActionModeSubStates!=AS_BLASTERDEFLECTMOTION) { // start motion triggered blaster deflect
      ActionModeSubStates=AS_BLASTERDEFLECTMOTION;
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Start motion triggered blaster bolt deflect"));
      #endif
    }
    else { // stop motion triggered blaster deflect
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("End motion triggered blaster bolt deflect"));
      #endif
      HumRelaunch();
      ActionModeSubStates=AS_HUM;
    }
	} 
#ifdef JUKEBOX 
	else if (SaberState==S_STANDBY) {
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("Enter JukeBox"));
      #endif
      SaberState=S_JUKEBOX;
      PrevSaberState=S_STANDBY;
		//ACTION TO DEFINE
	} else if (SaberState==S_JUKEBOX) {
//Entering JukeBox mode (MP3 player)
    SaberState=S_STANDBY;
    PrevSaberState=S_JUKEBOX;
// stop/pause track being played
    Pause_Sound();
  }
#endif  // JUKEBOX
} // lockupDoubleClick

void lockupLongPressStart() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress start"));
#endif
	if (SaberState==S_SABERON) {
    //Lockup Start
    ActionModeSubStates=AS_BLADELOCKUP;
		if (soundFont.getLockup((storage.soundFont)*NR_FILE_SF)) {
			LoopPlay_Sound(soundFont.getLockup((storage.soundFont)*NR_FILE_SF));
		}
	} else if (SaberState==S_CONFIG) {
      if (ConfigModeSubStates!=CS_MAINCOLOR and ConfigModeSubStates!=CS_CLASHCOLOR and ConfigModeSubStates!=CS_BLASTCOLOR) {
        //Leaving Config Mode
    		changeMenu = false;
    		//	repeat = true;
        SaberState=S_STANDBY;
        PrevSaberState=S_CONFIG;
        #ifdef PIXELBLADE
          pixelblade_KillKey_Disable();
        #endif
        Set_Volume(storage.volume);
        delay(200);      }
	} else if (SaberState==S_STANDBY) {
//Entering Config Mode
    SaberState=S_CONFIG;
    PrevSaberState=S_STANDBY;

	}
} // lockupLongPressStart

void lockupLongPress() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress..."));
#endif
	if (SaberState==S_SABERON) {
		/*
		 * ACTION TO DEFINE
		 */
    ActionModeSubStates=AS_BLADELOCKUP; // needed, otherwise the FSM will change to AS_HUM and the lockup will end prematurely when the hum is relaunched
    sndSuppress = millis();  // trick the hum relaunch by starting the stopper all over again otherwise the hum relaunch will interrupt the lockup
	} 
	else if (SaberState==S_CONFIG) {
    ConfigMenuButtonEventHandler(false, LONGPRESS);
	} else if (SaberState==S_STANDBY) {
		/*
		 * ACTION TO DEFINE
		 */
	}
} // lockupLongPress

void lockupLongPressStop() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Lockup button longPress stop"));
#endif
	if (SaberState==S_SABERON) {
    HumRelaunch();
    ActionModeSubStates=AS_HUM;
	}
} // lockupLongPressStop
#endif // SINGLEBUTTON not defined



