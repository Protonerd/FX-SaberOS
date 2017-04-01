/*
 * Buttons.c
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 */

#include "Buttons.h"
#include "Config.h"
#include "SoundFont.h"
#include "Light.h"


extern SoundFont soundFont;
enum SaberStateEnum {S_STANDBY, S_SABERON, S_CONFIG, S_SLEEP, S_JUKEBOX};
enum ActionModeSubStatesEnum {AS_HUM, AS_IGNITION, AS_RETRACTION, AS_BLADELOCKUP, AS_PREBLADELOCKUP, AS_BLASTERDEFLECTMOTION, AS_BLASTERDEFLECTPRESS, AS_CLASH, AS_SWING, AS_SPIN, AS_FORCE};
enum ConfigModeSubStatesEnum {CS_VOLUME, CS_SOUNDFONT, CS_MAINCOLOR, CS_CLASHCOLOR, CS_BLASTCOLOR, CS_FLICKERTYPE, CS_IGNITIONTYPE, CS_RETRACTTYPE, CS_SLEEPINIT, CS_BATTERYLEVEL};
extern SaberStateEnum SaberState;
extern SaberStateEnum PrevSaberState;
extern ActionModeSubStatesEnum ActionModeSubStates;
extern ConfigModeSubStatesEnum ConfigModeSubStates;
extern unsigned long sndSuppress;
extern bool hum_playing;
extern int8_t modification;
extern bool play;
extern int16_t value;
#ifdef JUKEBOX
extern bool jukebox_play;
extern uint8_t jb_track;
#endif
//extern bool blasterBlocks;
extern bool lockuponclash;
extern int8_t blink;
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
#if defined PIXELBLADE
extern uint8_t ledPins[];
extern cRGB color;
extern cRGB currentColor;
#endif
extern uint8_t blaster;
extern void HumRelaunch();
extern void SinglePlay_Sound(uint8_t track);
extern void LoopPlay_Sound(uint8_t track);
extern void Pause_Sound();
extern void Resume_Sound();
extern void Set_Loop_Playback();
extern void Set_Volume();
extern void confParseValue(uint16_t variable, uint16_t min, uint16_t max,
    short int multiplier);
#ifndef COLORS
extern uint8_t GravityVector();
#endif
#if defined LEDSTRINGS
extern struct StoreStruct {
  // This is for mere detection if they are our settings
  char version[5];
  // The settings
  uint8_t volume;     // 0 to 31
  uint8_t soundFont; // as many Sound font you have defined in Soundfont.h Max:253
} storage;
#endif
#if defined STAR_LED
extern struct StoreStruct {
  // This is for mere detection if they are our settings
  char version[5];
  // The settings
  uint8_t volume;// 0 to 31
  uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
  struct Profile {
  #ifdef COLORS
    uint8_t mainColor;  //colorID
    uint8_t clashColor;//colorID
    uint8_t blasterboltColor;//colorID
  #else
    cRGB mainColor;
    cRGB clashColor;
    cRGB blasterboltColor;
  #endif
  }sndProfile[SOUNDFONT_QUANTITY + 2];
}storage;
#endif  // STAR_LED

#if defined PIXELBLADE
extern struct StoreStruct {
  // This is for mere detection if they are our settings
  char version[5];
  // The settings
  uint8_t volume;// 0 to 31
  uint8_t soundFont;// as many as Sound font you have defined in Soundfont.h Max:253
  struct Profile {
  #ifdef COLORS
    uint8_t mainColor;  //colorID
    uint8_t clashColor;//colorID
    uint8_t blasterboltColor;//colorID
  #else
    cRGB mainColor;
    cRGB clashColor;
    cRGB blasterboltColor;
  #endif
  }sndProfile[SOUNDFONT_QUANTITY + 2];
}storage;
#endif // PIXELBLADE
// ====================================================================================
// ===               			BUTTONS CALLBACK FUNCTIONS                 			===
// ====================================================================================

void mainClick() {
#if defined LS_BUTTON_DEBUG
	Serial.println(F("Main button click."));
#endif
	if (SaberState==S_SABERON) {
    if (lockuponclash) {
      lockuponclash=false;
      HumRelaunch();
      ActionModeSubStates=AS_HUM;
      #if defined LS_BUTTON_DEBUG
            Serial.println(F("End clash triggered lockup (either pre or active phase)"));
      #endif  
    }
    else {
      lockuponclash=true;
#if defined LS_BUTTON_DEBUG
      Serial.println(F("Start clash triggered lockup (either pre or active phase)"));
#endif 
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
    SinglePlay_Sound(1);
    delay(50);
    if (ConfigModeSubStates == CS_VOLUME) {
      confParseValue(storage.volume, 0, 30, 1);
      storage.volume = value;
      BladeMeter(value*100/30);
      Set_Volume();
      #if defined LS_INFO
              Serial.println(storage.volume);
      #endif      
    }
    else if (ConfigModeSubStates == CS_SOUNDFONT) {
      play = false;
      confParseValue(storage.soundFont, 2, SOUNDFONT_QUANTITY + 1, 1);
      storage.soundFont = value;
      soundFont.setID(value);
      SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE

      delay(150);
      #if defined LS_INFO
              Serial.println(soundFont.getID());
      #endif      
    }
#ifdef COLORS
    else if (ConfigModeSubStates == CS_MAINCOLOR) {
      confParseValue(storage.sndProfile[storage.soundFont].mainColor, 0, COLORS - 1, 1);
      storage.sndProfile[storage.soundFont].mainColor =value;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        lightOn(currentColor);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_CLASHCOLOR) {
      confParseValue(storage.sndProfile[storage.soundFont].clashColor, 0, COLORS - 1, 1);
      storage.sndProfile[storage.soundFont].clashColor =value;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(currentColor);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_BLASTCOLOR) {
      confParseValue(storage.sndProfile[storage.soundFont].blasterboltColor, 0, COLORS - 1, 1);
      storage.sndProfile[storage.soundFont].blasterboltColor =value;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(currentColor);
      #endif  // PIXELBLADE
    }
    //modification=0;  // reset config mode change indicator 
#else // not COLORS
    #if defined(PIXELBLADE) or defined(STAR_LED)
    else if (ConfigModeSubStates == CS_MAINCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].mainColor,modification, MAX_BRIGHTNESS, true);
      storage.sndProfile[storage.soundFont].mainColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].mainColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].mainColor.b=currentColor.b;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        lightOn(currentColor, NUMPIXELS/2, NUMPIXELS-6);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_CLASHCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].clashColor,modification, MAX_BRIGHTNESS, true);
      storage.sndProfile[storage.soundFont].clashColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].clashColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].clashColor.b=currentColor.b;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        lightOn(currentColor, 1, NUMPIXELS/2-1);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_BLASTCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].blasterboltColor,modification, MAX_BRIGHTNESS, true);
      storage.sndProfile[storage.soundFont].blasterboltColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].blasterboltColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].blasterboltColor.b=currentColor.b;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
      #endif  // PIXELBLADE
    }
    #endif // PIXELBLADE or STAR_LED
#endif // COLORS/not COLORS   
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
    switch(ConfigModeSubStates) {
      case CS_BATTERYLEVEL:
      #if defined STAR_LED
          lightOff(ledPins);
      #else
          lightOff();
      #endif        
        ConfigModeSubStates=CS_SOUNDFONT;
        SinglePlay_Sound(5);
        delay(600);
        SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
            getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE
        delay(500);        
        break;
      case CS_SOUNDFONT:
        #ifdef LEDSTRINGS
          #if defined LS_FSM
            Serial.print(F("Volume"));
          #endif  
          ConfigModeSubStates=CS_VOLUME;
          BladeMeter(storage.volume*100/30);
          SinglePlay_Sound(4);
          delay(500);
        #else
          ConfigModeSubStates=CS_MAINCOLOR;
          SinglePlay_Sound(6);
          delay(500); 
          #if defined LS_FSM
            Serial.print(F("Main color"));
          #endif        
          #if defined STAR_LED
            getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, NUMPIXELS/2, NUMPIXELS-6);
            #endif 
          #endif  // PIXELBLADE
        #endif // is/isnot LEDSTRINGS
        break;
    #ifdef DEEP_SLEEP
      case CS_SLEEPINIT:
          ConfigModeSubStates=CS_SOUNDFONT;
          SinglePlay_Sound(5);
          delay(600);
          SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
            getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE
          delay(500);      
          break;
    #endif // DEEP_SLEEP
      case CS_VOLUME:
        #ifdef DEEP_SLEEP
          ConfigModeSubStates=CS_SLEEPINIT;
          // repeat a beep 2 times
          SinglePlay_Sound(1);
          delay(500);
          SinglePlay_Sound(1);
          delay(500);
        #else // no sleep mode capability
          lightOff();
          ConfigModeSubStates=CS_SOUNDFONT;
          SinglePlay_Sound(5);
          delay(600);
          SinglePlay_Sound(soundFont.getMenu());
            #if defined STAR_LED
              getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
              lightOn(ledPins, currentColor);
            #endif  // STAR_LED
            #if defined PIXELBLADE
              getColor(storage.sndProfile[storage.soundFont].mainColor);
              for (uint8_t i = 0; i < 6; i++) {
                digitalWrite(ledPins[i], HIGH);
              }
                lightOn(currentColor);
            #endif  // PIXELBLADE
          delay(500);
        #endif // DEEP_SLEEP        
        break;
      case CS_MAINCOLOR:
        ConfigModeSubStates=CS_CLASHCOLOR;
        SinglePlay_Sound(7);
        delay(500); 
        #if defined LS_FSM
          Serial.print(F("Clash color"));
        #endif        
        #if defined STAR_LED
          getColor(currentColor, storage.sndProfile[storage.soundFont].clashColor);
          lightOn(ledPins, currentColor);
        #endif  // STAR_LED
        #if defined PIXELBLADE
          getColor(storage.sndProfile[storage.soundFont].clashColor);
          for (uint8_t i = 0; i < 6; i++) {
            digitalWrite(ledPins[i], HIGH);
          }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, 1, NUMPIXELS/2-1);
            #endif 
         #endif  // PIXELBLADE
        break;
      case CS_CLASHCOLOR:
        ConfigModeSubStates=CS_BLASTCOLOR;
        SinglePlay_Sound(8);
        delay(500); 
        #if defined LS_FSM
          Serial.print(F("Blaster color"));
        #endif        
        #if defined STAR_LED
          getColor(currentColor, storage.sndProfile[storage.soundFont].blasterboltColor);
          lightOn(ledPins, currentColor);
        #endif  // STAR_LED
        #if defined PIXELBLADE
          getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
          for (uint8_t i = 0; i < 6; i++) {
            digitalWrite(ledPins[i], HIGH);
          }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
            #endif 
         #endif  // PIXELBLADE
        break;
      case CS_BLASTCOLOR:
        #if defined LS_FSM
          Serial.print(F("Volume"));
        #endif  
        ConfigModeSubStates=CS_VOLUME;
        BladeMeter(storage.volume*100/30);
        SinglePlay_Sound(4);
        delay(500); 
        break;   
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

#else  // not SINGLEBUTTON
// Change Menu
/*
    switch(ConfigModeSubStates) {
      case CS_MAINCOLOR:
        ConfigModeSubStates=CS_CLASHCOLOR;
        SinglePlay_Sound(7);
        delay(500); 
        #if defined LS_FSM
          Serial.print(F("Clash color"));
        #endif        
        #if defined STAR_LED
          getColor(currentColor, storage.sndProfile[storage.soundFont].clashColor);
          lightOn(ledPins, currentColor);
        #endif  // STAR_LED
        #if defined PIXELBLADE
          getColor(storage.sndProfile[storage.soundFont].clashColor);
          for (uint8_t i = 0; i < 6; i++) {
            digitalWrite(ledPins[i], HIGH);
          }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, 1, NUMPIXELS/2-1);
            #endif 
         #endif  // PIXELBLADE
        break;
      case CS_CLASHCOLOR:
        ConfigModeSubStates=CS_BLASTCOLOR;
        SinglePlay_Sound(8);
        delay(500); 
        #if defined LS_FSM
          Serial.print(F("Blaster color"));
        #endif        
        #if defined STAR_LED
          getColor(currentColor, storage.sndProfile[storage.soundFont].blasterboltColor);
          lightOn(ledPins, currentColor);
        #endif  // STAR_LED
        #if defined PIXELBLADE
          getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
          for (uint8_t i = 0; i < 6; i++) {
            digitalWrite(ledPins[i], HIGH);
          }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
            #endif 
         #endif  // PIXELBLADE
        break;
      case CS_BLASTCOLOR:
        #if defined LS_FSM
          Serial.print(F("Volume"));
        #endif  
        ConfigModeSubStates=CS_VOLUME;
        SinglePlay_Sound(4);
        delay(500); 
        break;   
      }*/
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
    switch(ConfigModeSubStates) {
      case CS_BATTERYLEVEL:
      #if defined STAR_LED
          lightOff(ledPins);
      #else
          lightOff();
      #endif        
        ConfigModeSubStates=CS_SOUNDFONT;
        SinglePlay_Sound(5);
        delay(600);
        SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            lightOff();
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE
        delay(500);        
        break;
      case CS_SOUNDFONT:
        #ifdef LEDSTRINGS
          #if defined LS_FSM
            Serial.print(F("Volume"));
          #endif  
          ConfigModeSubStates=CS_VOLUME;
          BladeMeter(storage.volume*100/30);
          SinglePlay_Sound(4);
          delay(500);
        #else  // not LEDSTRINGS
          ConfigModeSubStates=CS_MAINCOLOR;
          SinglePlay_Sound(6);
          delay(500); 
          #if defined LS_FSM
            Serial.print(F("Main color"));
          #endif        
          #if defined STAR_LED
              getColor(storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, NUMPIXELS/2, NUMPIXELS-6);
            #endif 
          #endif  // PIXELBLADE
        #endif // is/isnot LEDSTRINGS
        break;
      case CS_VOLUME:
        #ifdef DEEP_SLEEP
          ConfigModeSubStates=CS_SLEEPINIT;
          // repeat a beep 2 times
          SinglePlay_Sound(1);
          delay(500);
          SinglePlay_Sound(1);
          delay(500);
        #else // no sleep mode capability
          ConfigModeSubStates=CS_SOUNDFONT;
          SinglePlay_Sound(5);
          delay(600);
          SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
            getColor(currentColor, storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            lightOff();
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE
          delay(500);      
        #endif  // DEEP_SLEEP
        break;
      case CS_MAINCOLOR:
        ConfigModeSubStates=CS_CLASHCOLOR;
        SinglePlay_Sound(7);
        delay(500); 
        #if defined LS_FSM
          Serial.print(F("Clash color"));
        #endif        
        #if defined STAR_LED
          getColor(storage.sndProfile[storage.soundFont].clashColor);
          lightOn(ledPins, currentColor);
        #endif  // STAR_LED
        #if defined PIXELBLADE
          getColor(storage.sndProfile[storage.soundFont].clashColor);
          for (uint8_t i = 0; i < 6; i++) {
            digitalWrite(ledPins[i], HIGH);
          }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, 1, NUMPIXELS/2-1);
            #endif 
         #endif  // PIXELBLADE
        break;
      case CS_CLASHCOLOR:
        ConfigModeSubStates=CS_BLASTCOLOR;
        SinglePlay_Sound(8);
        delay(500); 
        #if defined LS_FSM
          Serial.print(F("Blaster color"));
        #endif        
        #if defined STAR_LED
          getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
          lightOn(ledPins, currentColor);
        #endif  // STAR_LED
        #if defined PIXELBLADE
          getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
          for (uint8_t i = 0; i < 6; i++) {
            digitalWrite(ledPins[i], HIGH);
          }
            #ifdef COLORS
              lightOn(currentColor);
            #else  // not COLORS
              lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
            #endif 
         #endif  // PIXELBLADE
        break;
      case CS_BLASTCOLOR:
        #if defined LS_FSM
          Serial.print(F("Volume"));
        #endif  
        ConfigModeSubStates=CS_VOLUME;
        BladeMeter(storage.volume*100/30);
        SinglePlay_Sound(4);
        delay(500); 
        break;   
      #ifdef DEEP_SLEEP
      case CS_SLEEPINIT:
          ConfigModeSubStates=CS_SOUNDFONT;
          SinglePlay_Sound(5);
          delay(600);
          SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
              getColor(storage.sndProfile[storage.soundFont].mainColor);
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE
          delay(500);      
          break;
      #endif // DEEP_SLEEP
      }
#else  // SINGLEBUTTON
//Leaving Config Mode
  if (ConfigModeSubStates!=CS_MAINCOLOR and ConfigModeSubStates!=CS_CLASHCOLOR and ConfigModeSubStates!=CS_BLASTCOLOR) {
    changeMenu = false;
    SaberState=S_STANDBY;
    PrevSaberState=S_CONFIG;
    #ifdef PIXELBLADE
      pixelblade_KillKey_Disable();
    #endif
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
    #if defined(PIXELBLADE) or defined(STAR_LED)
    #ifndef COLORS
    //if (ConfigModeSubStates==CS_MAINCOLOR or ConfigModeSubStates==CS_CLASHCOLOR or ConfigModeSubStates==CS_BLASTCOLOR) {
    //  modification=GravityVector();
    //}
    if (ConfigModeSubStates==CS_MAINCOLOR) {
      //confParseValue(storage.sndProfile[storage.soundFont].mainColor, 0, 100 - 1, 1);
      ColorMixing(storage.sndProfile[storage.soundFont].mainColor,modification,false);
      storage.sndProfile[storage.soundFont].mainColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].mainColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].mainColor.b=currentColor.b;
          #ifdef PIXELBLADE
            lightOn(currentColor, NUMPIXELS/2, NUMPIXELS-6);
          #else if STAR_LED
            lightOn(ledPins, currentColor);
          #endif
      delay(50);
    
      }
    else if (ConfigModeSubStates==CS_CLASHCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].clashColor,modification,false);
      storage.sndProfile[storage.soundFont].clashColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].clashColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].clashColor.b=currentColor.b;
          #ifdef PIXELBLADE
            lightOn(currentColor, 1, NUMPIXELS/2-1);
          #else if STAR_LED
            lightOn(ledPins, currentColor);
          #endif
      delay(50);
    }
    else if (ConfigModeSubStates==CS_BLASTCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].blasterboltColor,modification,false);
      storage.sndProfile[storage.soundFont].blasterboltColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].blasterboltColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].blasterboltColor.b=currentColor.b;
          #ifdef PIXELBLADE
            lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
          #else if STAR_LED
            lightOn(ledPins, currentColor);
          #endif
      delay(50);
    }
  #endif // not COLORS
  #endif // PIXELBLADE or STAR_LED
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
      SinglePlay_Sound(2);
      delay(50);
    if (ConfigModeSubStates == CS_VOLUME) {
      //play=true;
      confParseValue(storage.volume, 0, 30, -1);
      storage.volume = value;
      BladeMeter(storage.volume*100/30);
      #ifdef OLD_DPFPLAYER_LIB
        mp3_set_volume (storage.volume);
      #else
        Set_Volume(); // Too Slow: we'll change volume on exit
      #endif
      delay(50);
      #if defined LS_INFO
        Serial.println(storage.volume);
      #endif      
    }
    else if (ConfigModeSubStates == CS_SOUNDFONT) {
      play = false;
      confParseValue(storage.soundFont, 2, SOUNDFONT_QUANTITY + 1, -1);
      storage.soundFont = value;
      soundFont.setID(value);
      SinglePlay_Sound(soundFont.getMenu());
          #if defined STAR_LED
            #ifdef COLORS
              getColor(storage.sndProfile[storage.soundFont].mainColor);
            #else
              getColor(storage.sndProfile[storage.soundFont].mainColor);
            #endif
            lightOn(ledPins, currentColor);
          #endif  // STAR_LED
          #if defined PIXELBLADE
            getColor(storage.sndProfile[storage.soundFont].mainColor);
            for (uint8_t i = 0; i < 6; i++) {
              digitalWrite(ledPins[i], HIGH);
            }
              lightOn(currentColor);
          #endif  // PIXELBLADE

      delay(150);
      #if defined LS_INFO
        Serial.println(soundFont.getID());
      #endif      
    }
#ifdef COLORS
    else if (ConfigModeSubStates == CS_MAINCOLOR) {
      confParseValue(storage.sndProfile[storage.soundFont].mainColor, 0, COLORS - 1, -1);
      storage.sndProfile[storage.soundFont].mainColor =value;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        lightOn(currentColor);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_CLASHCOLOR) {
      confParseValue(storage.sndProfile[storage.soundFont].clashColor, 0, COLORS - 1, -1);
      storage.sndProfile[storage.soundFont].clashColor =value;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(currentColor);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_BLASTCOLOR) {
      confParseValue(storage.sndProfile[storage.soundFont].blasterboltColor, 0, COLORS - 1, -1);
      storage.sndProfile[storage.soundFont].blasterboltColor =value;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(currentColor);
      #endif  // PIXELBLADE
    }
#else // not COLORS
    #if defined(PIXELBLADE) or defined(STAR_LED)
    else if (ConfigModeSubStates == CS_MAINCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].mainColor,modification, MAX_BRIGHTNESS, true);
      storage.sndProfile[storage.soundFont].mainColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].mainColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].mainColor.b=currentColor.b;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].mainColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        lightOn(currentColor, NUMPIXELS/2, NUMPIXELS-6);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_CLASHCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].clashColor,modification, MAX_BRIGHTNESS, true);
      storage.sndProfile[storage.soundFont].clashColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].clashColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].clashColor.b=currentColor.b;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].clashColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        lightOn(currentColor, 1, NUMPIXELS/2-1);
      #endif  // PIXELBLADE
    }
    else if (ConfigModeSubStates == CS_BLASTCOLOR) {
      ColorMixing(storage.sndProfile[storage.soundFont].blasterboltColor,modification, MAX_BRIGHTNESS, true);
      storage.sndProfile[storage.soundFont].blasterboltColor.r=currentColor.r;
      storage.sndProfile[storage.soundFont].blasterboltColor.g=currentColor.g;
      storage.sndProfile[storage.soundFont].blasterboltColor.b=currentColor.b;
      #ifdef STAR_LED
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightOn(ledPins, currentColor);
      #endif // STAR_LED
      #ifdef PIXELBLADE
        lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
      #endif  // PIXELBLADE
    }
    #endif // PIXELBLADE or STAR_LED
#endif // COLORS
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
		blink=0;
		if (soundFont.getLockup()) {
			LoopPlay_Sound(soundFont.getLockup());
		}
	} else if (SaberState==S_CONFIG) {
      if (ConfigModeSubStates!=CS_MAINCOLOR and ConfigModeSubStates!=CS_CLASHCOLOR and ConfigModeSubStates!=CS_BLASTCOLOR) {
        //Leaving Config Mode
    		changeMenu = false;
    		//	repeat = true;
        SaberState=S_STANDBY;
        PrevSaberState=S_CONFIG;
      }
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
    #if defined(PIXELBLADE) or defined(STAR_LED)
      #ifndef COLORS
        if (ConfigModeSubStates==CS_MAINCOLOR) {
          ColorMixing(storage.sndProfile[storage.soundFont].mainColor,modification,false);
          storage.sndProfile[storage.soundFont].mainColor.r=currentColor.r;
          storage.sndProfile[storage.soundFont].mainColor.g=currentColor.g;
          storage.sndProfile[storage.soundFont].mainColor.b=currentColor.b;
          #ifdef PIXELBLADE
            lightOn(currentColor, NUMPIXELS/2, NUMPIXELS-6);
          #else if STAR_LED
            lightOn(ledPins, currentColor);
          #endif
          delay(50);
          }
        else if (ConfigModeSubStates==CS_CLASHCOLOR) {
          ColorMixing(storage.sndProfile[storage.soundFont].clashColor,modification,false);
          storage.sndProfile[storage.soundFont].clashColor.r=currentColor.r;
          storage.sndProfile[storage.soundFont].clashColor.g=currentColor.g;
          storage.sndProfile[storage.soundFont].clashColor.b=currentColor.b;
          #ifdef PIXELBLADE
            lightOn(currentColor, 1, NUMPIXELS/2-1);
          #else if STAR_LED
            lightOn(ledPins, currentColor);
          #endif
          delay(50);
        }
        else if (ConfigModeSubStates==CS_BLASTCOLOR) {
          ColorMixing(storage.sndProfile[storage.soundFont].blasterboltColor,modification,false);
          storage.sndProfile[storage.soundFont].blasterboltColor.r=currentColor.r;
          storage.sndProfile[storage.soundFont].blasterboltColor.g=currentColor.g;
          storage.sndProfile[storage.soundFont].blasterboltColor.b=currentColor.b;
          #ifdef PIXELBLADE
            lightOn(currentColor, NUMPIXELS*3/4-5, NUMPIXELS*3/4);
          #else if STAR_LED
            lightOn(ledPins, currentColor);
          #endif
          delay(50);
        }
      #endif // not COLORS
    #endif // PIXELBLADE or STAR_LED

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


