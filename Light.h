/*
 * Light.h
 *
 *  Created on: 21 Octber 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
 * Source :  https://github.com/Protonerd/FX-SaberOS
 */

#if not defined LIGHT_H_
#define LIGHT_H_

#include <Arduino.h>
#include <WS2812.h>
#include "Config_HW.h"
#include "Config_SW.h"

enum AccentLedAction_En {AL_PULSE, AL_ON, AL_OFF};


// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

void BladeMeter (uint8_t ledPins[], int meterLevel); 

// unified light functions
void lightOn(uint8_t ledPins[], int8_t segment = -1, cRGB color={0,0,0}, int8_t StartPixel=-1, int8_t StopPixel=-1);
void lightOff(uint8_t ledPins[], int8_t segment = -1, int8_t StartPixel=-1, int8_t StopPixel=-1);
void getColor(cRGB color={0,0,0}); //getColor
void RampBlade(uint16_t RampDuration, bool DirectionUpDown, int8_t StartPixel=-1, int8_t StopPixel=-1);
void lightIgnition(uint8_t ledPins[], uint16_t time, uint8_t type, cRGB color={0,0,0}, int8_t StartPixel=-1, int8_t StopPixel=-1);
void lightRetract(uint8_t ledPins[], uint16_t time, uint8_t type,cRGB color={0,0,0}, int8_t StartPixel=-1, int8_t StopPixel=-1);
void lightFlicker(uint8_t ledPins[],uint8_t type, uint8_t value = 0,cRGB maincolor={0,0,0}, cRGB clashcolor={0,0,0},uint8_t AState=0, int8_t StartPixel=-1, int8_t StopPixel=-1);
void ColorMixing(cRGB colorID={0,0,0}, int8_t mod=-1, uint8_t maxBrightness=MAX_BRIGHTNESS, bool Saturate=false);
void lightBlasterEffect(uint8_t ledPins[], uint8_t pixel, uint8_t range, uint16_t B_time=BLASTER_FX_DURATION, cRGB SndFnt_MainColor={0,0,0});
void lightClashEffect(uint8_t ledPins[], cRGB color={0,0,0});

#ifdef JUKEBOX
  #ifdef PIXELBLADE
    void JukeBox_Stroboscope(cRGB color={0,0,0});
  #else
    void JukeBox_Stroboscope(unit8_t ledPins[], cRGB color={0,0,0});
  #endif
#endif
void pixelblade_KillKey_Enable();
void pixelblade_KillKey_Disable();

void getColorFix(uint8_t colorID);

void FoCOn (uint8_t pin);
void FoCOff (uint8_t pin);

#ifdef ADF_PIXIE_BLADE
  void InitAdafruitPixie(uint8_t ledPins[]);
#endif

#ifdef PIXELBLADE
  #ifdef ANIBLADE
  void FireBlade(uint8_t DominantColor=0);
  cRGB HeatColor( uint8_t temperature, uint8_t DominantColor);
  uint8_t scale8_video( uint8_t i, uint8_t scale);
  uint8_t DominantMainColor(cRGB color={0,0,0});
  #endif
#endif

void accentLEDControl(AccentLedAction_En AccentLedAction);
//void PWM();
//#ifdef PIXEL_ACCENT
  void pixelAccentUpdate();
  void AccentMeter (int MeterLevel);
//#endif
#endif /* LIGHT_H_ */


