/*
 * Light.h
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 */

#if not defined LIGHT_H_
#define LIGHT_H_

#include <Arduino.h>
#include <WS2812.h>
#include "Config.h"


enum AccentLedAction_En {AL_PULSE, AL_ON, AL_OFF};

#if defined ACCENT_LED
#if defined SOFT_ACCENT

struct softPWM {
  uint8_t dutyCycle; // in percent
  bool revertCycle;
  uint8_t state;
  uint16_t tick;
} pwmPin = { 100, false, LOW, 0 };
#endif
#endif

// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

void BladeMeter (int meterLevel); 

#if defined LEDSTRINGS

void lightOn(uint8_t ledPins[], int8_t segment = -1);
void lightOff();

void lightIgnition(uint8_t ledPins[], uint16_t time, uint8_t type);
void lightRetract(uint8_t ledPins[], uint16_t time, uint8_t type);

void FoCOn (uint8_t pin);
void FoCOff (uint8_t pin);

void lightFlicker(uint8_t ledPins[], uint8_t type, uint8_t value = 0,uint8_t AState=0);
  #ifdef JUKEBOX
    void JukeBox_Stroboscope(uint8_t ledPins[]);
  #endif
#endif
#if defined STAR_LED

void lightOn(uint8_t ledPins[], cRGB color);
void lightOff(uint8_t ledPins[]);
#ifndef COLORS
void ColorMixing(cRGB colorID, int8_t mod, uint8_t maxBrightness=MAX_BRIGHTNESS, bool Saturate=false);
#endif
void lightIgnition(uint8_t ledPins[], cRGB color, uint16_t time);
void lightRetract(uint8_t ledPins[], cRGB color, uint16_t time);

void lightFlicker(uint8_t ledPins[], cRGB color, uint8_t value = 0);

#ifdef COLORS
void getColor(uint8_t color); //getColor
#else
void getColor(cRGB color); //getColor
#endif

#ifdef JUKEBOX
    void JukeBox_Stroboscope();
#endif
#endif  //  STAR_LED

#if defined PIXELBLADE

void pixelblade_KillKey_Enable();
void pixelblade_KillKey_Disable();

void lightOn(cRGB color, int8_t StartPixel=-1, int8_t StopPixel=-1);
void lightOff();

void lightIgnition(cRGB color, uint16_t time, uint8_t type);
void lightRetract( uint16_t time, uint8_t type);

#ifdef COLORS
void lightBlasterEffect( uint8_t pixel, uint8_t range, uint8_t SndFnt_MainColor);
#else
void lightBlasterEffect( uint8_t pixel, uint8_t range, cRGB SndFnt_MainColor);
#endif
void lightFlicker( uint8_t value = 0,uint8_t AState=0);

#ifdef COLORS
void getColor(uint8_t color); //getColor
#else
void getColor(cRGB color); //getColor
void ColorMixing(cRGB colorID, int8_t mod, uint8_t maxBrightness=MAX_BRIGHTNESS, bool Saturate=false);
#endif
void RampPixels(uint16_t RampDuration, bool DirectionUpDown);

#ifdef FIREBLADE
void FireBlade();
cRGB HeatColor( uint8_t temperature);
uint8_t scale8_video( uint8_t i, uint8_t scale);
#endif

#ifdef JUKEBOX
void JukeBox_Stroboscope(cRGB color);
#endif

#endif

void accentLEDControl(AccentLedAction_En AccentLedAction);
void PWM();
#endif /* LIGHT_H_ */


