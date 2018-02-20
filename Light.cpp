/*
 * Light.cpp
 *
 * author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)lightClashEffect
 * Source :  https://github.com/Protonerd/FX-SaberOS
 */
#include "Light.h"
#include "Config_HW.h"
#include "Config_SW.h"
#include "ConfigMenu.h"
#include "SoundFont.h"

#if defined PIXELBLADE
#include <WS2812.h>
#endif


// global Saber state and Sub State variables
extern SaberStateEnum SaberState;
extern SaberStateEnum PrevSaberState;
extern ActionModeSubStatesEnum ActionModeSubStates;
extern ConfigModeSubStatesEnum ConfigModeSubStates;
extern ActionModeSubStatesEnum PrevActionModeSubStates;
extern ConfigModeSubStatesEnum PrevConfigModeSubStates;
//extern SubStateEnum SubState;

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
extern uint8_t ledPins[];
# if defined ACCENT_LED
  unsigned long lastAccent = millis();
#endif
#if defined SOFT_ACCENT
  unsigned long lastAccentTick = micros();
#endif


#ifdef JUKEBOX
#define SAMPLESIZEAVERAGE 30
#endif

bool fireblade=false;

#ifdef PIXELBLADE // FIREBLADE
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
static uint8_t Fire_Cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
static uint8_t Fire_Sparking = 100;
//#ifdef CROSSGUARDSABER
//static byte heat[MN_STRIPE];  
//static byte heat_cg[CG_STRIPE];
//#else
static byte heat[NUMPIXELS];  
//#endif
#define PIXELSTEP 5// how many pixel to treat as a group to save on processing capability
#endif  // PIXELBLADE


/* ***************** UNIFIED LIGHT FUNCTIONS *********************/

#define I_BEGINNEXTSEGMENT 50
#define R_BEGINNEXTSEGMENT 100
#define PULSEFLICKERDEPTH 100
#define PULSEDURATION 500


uint8_t pulseflicker_pwm=0;
bool pulsedir=true;
static uint8_t flickerPos = 0;
static long lastFlicker = millis();
#if defined PIXELBLADE
  extern WS2812 pixels;
#endif

#if defined STAR_LED or defined PIXELBLADE or defined ADF_PIXIE_BLADE
extern cRGB currentColor;
#endif

#ifdef ADF_PIXIE_BLADE
  SoftwareSerial pixieSerial(-1, PIXIEPIN);
  Adafruit_Pixie strip = Adafruit_Pixie(NUMPIXELS, &pixieSerial);
#endif

#ifdef COLOR_PROFILE
// define an array for the 15 color profiles
cRGB colorProfiles[15];
//colorProfiles[0].r=255;
#endif
// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================

/* ***************** UNIFIED LIGHT FUNCTIONS *********************/
void lightOn(uint8_t ledPins[], int8_t segment = -1, cRGB color={0,0,0}, int8_t StartPixel=-1, int8_t StopPixel=-1) {
  // Light On
  #if defined LEDSTRINGS
    if (segment == -1) {
      for (uint8_t i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
    } else {
      digitalWrite(ledPins[segment], HIGH);
    }
  #endif
  #if defined STAR_LED
  // Light On
      analogWrite(ledPins[0], color.r); // RED
      analogWrite(ledPins[1], color.g); // GREEN
      analogWrite(ledPins[2], color.b); // BLUE
  #endif

  #ifdef ADF_PIXIE_BLADE
    for(uint8_t i=0; i< NUMPIXELS; i++) {
      strip.setPixelColor(i, color.r, color.g, color.b);
    }
    strip.show();
  #endif
  
  #ifdef PIXELBLADE
    // Light On
    if (StartPixel == -1 or StopPixel==-1 or StopPixel<StartPixel or StartPixel>NUMPIXELS or StopPixel>NUMPIXELS) {  // if neither start nor stop is defined or invalid range, go through the whole stripe
      for (uint8_t i = 0; i < NUMPIXELS; i++) {
        pixels.set_crgb_at(i, color);
      }
    } else {
      for (uint8_t i = StartPixel-1; i < StopPixel; i++) {
        pixels.set_crgb_at(i, color);
      }
    }
    pixels.sync();
  #endif
} //lightOn

void lightOff() {
  #if defined LEDSTRINGS
    // shut Off
    //Shut down PWM
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0B1));
    TCCR1A &= ~((1 << COM1A1) | (1 << COM1B1));
    TCCR2A &= ~((1 << COM2A1) | (1 << COM2B1));
    //Shut down everything at once
    PORTB &= B11010001;
    PORTD &= B10010111;
  #endif
  
  #if defined STAR_LED
    // shut Off
    digitalWrite(LED_RED, LOW); // RED
    digitalWrite(LED_GREEN, LOW); // GREEN
    digitalWrite(LED_BLUE, LOW); // BLUE
  #endif

  #ifdef ADF_PIXIE_BLADE
    for(uint8_t i=0; i< NUMPIXELS; i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
  #endif
  
  #ifdef PIXELBLADE
    // shut Off
    cRGB value;
    value.b = 0;
    value.g = 0;
    value.r = 0; // RGB Value -> Off
    for (uint16_t i = 0; i < NUMPIXELS; i++) {
      pixels.set_crgb_at(i, value);
    }
    pixels.sync();
  #endif

} //lightOff

void getColor(cRGB color={0,0,0}) {
  #if defined LEDSTRINGS
  
  #endif
  
  #if defined STAR_LED
    currentColor.r = color.r;
    currentColor.g = color.g;
    currentColor.b = color.b;
  #endif
  
  #ifdef ADF_PIXIE_BLADE
    currentColor.r = color.r;
    currentColor.g = color.g;
    currentColor.b = color.b;
  #endif
    
  #ifdef PIXELBLADE
    currentColor.r = color.r;
    currentColor.g = color.g;
    currentColor.b = color.b;
  #endif  
} // getColor

void RampBlade(uint16_t RampDuration, bool DirectionUpDown, int8_t StartPixel=-1, int8_t StopPixel=-1) {
  #if defined LEDSTRINGS
  
  #endif
  
  #if defined STAR_LED

  #endif

  #ifdef ADF_PIXIE_BLADE

  #endif
  
  #ifdef PIXELBLADE
    unsigned long ignitionStart = millis();  //record start of ramp function
    cRGB value;
    if (StartPixel == -1 or StopPixel==-1 or StopPixel<StartPixel or StartPixel>NUMPIXELS or StopPixel>NUMPIXELS) {  // if neither start nor stop is defined or invalid range, go through the whole stripe    // neopixel ramp code from jbkuma
      StartPixel=0;
      StopPixel= NUMPIXELS; 
    }
    if (fireblade) { // #ifdef FIREBLADE
      for (unsigned int i=StartPixel; i<StopPixel; (i=i+5)) { // turn on/off one LED at a time
         FireBlade(storage.sndProfile[storage.soundFont].flickerType-2);
         for(unsigned int j=0; j<StopPixel; j++ ) { // fill up string with data
          if ((DirectionUpDown and j<=i) or (!DirectionUpDown and j<=StopPixel-1-i)){
            }
            else if ((DirectionUpDown and j>i) or (!DirectionUpDown and j>StopPixel-1-i)){
              value.r=0;
              value.g=0;
              value.b=0;  
              //heat[j]=0;
              pixels.set_crgb_at(j, value); // Set value at LED found at index j
            }      
          }
          pixels.sync(); // Sends the data to the LEDs
        }    
    } // fireblade
    else { //#else
      for (unsigned int i = StartPixel; i < StopPixel; i = StopPixel*(millis()-ignitionStart)/RampDuration) { // turn on/off the number of LEDs that match rap timing
          //generate a flicker effect between 65% and 115% of MAX_BRIGHTNESS, with a 1 in 115 chance of flicking to 0
          int flickFactor = random(0,115);
          if (flickFactor < 65 && flickFactor > 0) { flickFactor = 100; } 
         for(uint8_t  j=StartPixel; j<StopPixel; j++ ) { // fill up string with data
          if ((DirectionUpDown and j<=i)){
            value.r = MAX_BRIGHTNESS * i / NUMPIXELS * currentColor.r / rgbFactor * flickFactor / 100;
            value.g = MAX_BRIGHTNESS * i / NUMPIXELS * currentColor.g / rgbFactor * flickFactor / 100;
            value.b = MAX_BRIGHTNESS * i / NUMPIXELS * currentColor.b / rgbFactor * flickFactor / 100;
            } else if (!DirectionUpDown and j<=NUMPIXELS-1-i){
            value.r = MAX_BRIGHTNESS * (NUMPIXELS - i) / NUMPIXELS * currentColor.r / rgbFactor * flickFactor / 100;
            value.g = MAX_BRIGHTNESS * (NUMPIXELS - i) / NUMPIXELS * currentColor.g / rgbFactor * flickFactor / 100;
            value.b = MAX_BRIGHTNESS * (NUMPIXELS - i) / NUMPIXELS * currentColor.b / rgbFactor * flickFactor / 100;
            } else if ((DirectionUpDown and j>i) or (!DirectionUpDown and j>NUMPIXELS-1-i)){
            value.r=0;
            value.g=0;
            value.b=0;      
          }      
          pixels.set_crgb_at(j, value);
        }
         pixels.sync(); // Sends the data to the LEDs
         delay(RampDuration/(StopPixel-StartPixel)); //match the ramp duration to the number of pixels in the string
      }
    } // #endif
  #endif  
} // RampBlade

void lightIgnition(uint8_t ledPins[], uint16_t time, uint8_t type, cRGB color={0,0,0}, int8_t StartPixel=-1, int8_t StopPixel=-1) {
  #if defined LEDSTRINGS
  
  uint8_t LS_Status[6];
  bool ongoing=true;

  switch (type) {
    case 0:
     for (uint8_t i=0; i<6; i++) {
      LS_Status[i]=0;
     }
     while (ongoing) {  // do the loops as long the variable is set to false, when the last segment finsihed the ramp
      for (uint8_t i = 0; i < 6; i++) {
        analogWrite(ledPins[i], LS_Status[i]);
        if (i==0 and LS_Status[i]<255) {
          LS_Status[i]++;
        }
        else if (i>0 and LS_Status[i-1]>=I_BEGINNEXTSEGMENT and LS_Status[i]<255) {
          LS_Status[i]++;
        }
        if (LS_Status[5]==255) {
          ongoing=false;
        }
      }
      delayMicroseconds(time * (1000/(5*I_BEGINNEXTSEGMENT+255)));
     }
     // ramp down to MAX_BRIGHTNESS
     for (uint8_t j = 255; j >= MAX_BRIGHTNESS; j--) {
      for (uint8_t i = 0; i < 6; i++) {
        analogWrite(ledPins[i], j);
      }
      delay(3);
     }
   /*
// Light up the ledstrings Movie-like
    for (uint8_t i = 0; i < 6; i++) {
      for (uint8_t j=0; j<=MAX_BRIGHTNESS;j+=10) {
        analogWrite(ledPins[i], j);
        delay(time / (6*25));
      }
      //delay(time / (5*10));
    }
    */
    break;
    case 1:
    for (int8_t i = 5; i >= 0; i--) {
      for (uint8_t j = 0; j <= i; j++) {
        if (j > 0) {
          digitalWrite(ledPins[j - 1], LOW);
        }
        digitalWrite(ledPins[j], HIGH);
        delay(time / 20);
      }
    }
    break;
  }
  #endif
  
  #if defined STAR_LED
    // Fade in to Maximum brightness
     for (uint8_t fadeIn = 0; fadeIn < 255; fadeIn++) {
      analogWrite(LED_RED, color.r * fadeIn / 255); // RED
      analogWrite(LED_GREEN, color.g * fadeIn / 255); // GREEN
      analogWrite(LED_BLUE, color.b * fadeIn / 255); // BLUE
      delay(time / 255);
    }
  #endif

  #ifdef ADF_PIXIE_BLADE
 
  // Fade in to Maximum brightness
    for (uint8_t fadeIn = 0; fadeIn < 255; fadeIn++) {
      for(uint8_t i=0; i< NUMPIXELS; i++) {        
        strip.setPixelColor(i, color.r * fadeIn / 255, color.g * fadeIn / 255, color.b * fadeIn / 255);
      }
    strip.show();
    delay(time / 255);
    }
  #endif
  
  #ifdef PIXELBLADE
    cRGB value;
    if (StartPixel == -1 or StopPixel==-1 or StopPixel<StartPixel or StartPixel>NUMPIXELS or StopPixel>NUMPIXELS) {  // if neither start nor stop is defined or invalid range, go through the whole stripe    // neopixel ramp code from jbkuma
      StartPixel=0;
      StopPixel= NUMPIXELS; 
    }
    value.r = MAX_BRIGHTNESS * color.r / rgbFactor;
    value.g = MAX_BRIGHTNESS * color.g / rgbFactor;
    value.b = MAX_BRIGHTNESS * color.b / rgbFactor;
    //switch (type) {
    //  case 0:
    //  // Light up the ledstrings Movie-like
      RampBlade(time, true, StartPixel, StopPixel);
    //}
  #endif  
} // lightIgnition

void lightRetract(uint8_t ledPins[], uint16_t time, uint8_t type,cRGB color={0,0,0}) {
  #if defined LEDSTRINGS

    uint8_t LS_Status[6];
    bool ongoing=true;

    switch (type) {
      case 0:
      // Light off the ledstrings Movie Like
       for (uint8_t i=0; i<6; i++) {
        LS_Status[i]=MAX_BRIGHTNESS;
       }
       while (ongoing) {  // do the loops as long the variable is set to false, when the last segment finsihed the ramp
        for (uint8_t i = 0; i < 6; i++) {
          if (i==5 and LS_Status[i]>0) {
            LS_Status[i]--;
          }
          else if (i<5 and LS_Status[i+1]<=R_BEGINNEXTSEGMENT and LS_Status[i]>0) {
            LS_Status[i]--;
          }
          if (LS_Status[0]==0) {
            ongoing=false;
          }
          analogWrite(ledPins[i], LS_Status[i]);
        }
        delayMicroseconds(time * (1000/(5*(MAX_BRIGHTNESS-R_BEGINNEXTSEGMENT)+MAX_BRIGHTNESS)));
       }
      break;
      case 1:
  // Light off the ledstrings invert
      for (int8_t i = 5; i >= 0; i--) {
        for (uint8_t j = 0; j <= i; j++) {
          if (j > 0) {
            digitalWrite(ledPins[j - 1], HIGH);
          }
          digitalWrite(ledPins[j], LOW);
          delay(time / 20);
        }
      }
      break;
    }
  #endif
  
  #if defined STAR_LED
    // Fade in to Maximum brightness
     for (uint8_t fadeIn = 255; fadeIn > 0; fadeIn--) {
      analogWrite(LED_RED, color.r * fadeIn / 255); // RED
      analogWrite(LED_GREEN, color.g * fadeIn / 255); // GREEN
      analogWrite(LED_BLUE, color.b * fadeIn / 255); // BLUE
      delay(time / 255);
    }
  #endif

  #ifdef ADF_PIXIE_BLADE
 
  // Fade in to Maximum brightness
    for (uint8_t fadeIn = 255; fadeIn > 0; fadeIn--) {
      for(uint8_t i=0; i< NUMPIXELS; i++) {        
        strip.setPixelColor(i, color.r * fadeIn / 255, color.g * fadeIn / 255, color.b * fadeIn / 255);
      }
    strip.show();
    delay(time / 255);
    }
  #endif
  
  #ifdef PIXELBLADE
    //switch (type) {
    //  case 0:
      // Light off the ledstrings Movie Like
      cRGB value;
      value.b = 0;
      value.g = 0;
      value.r = 0; // RGB Value -> Off
      RampBlade(time, false);
      //break;
  if (fireblade) { // #ifdef FIREBLADE
/*    #ifdef CROSSGUARDSABER
      for(unsigned int j=0; j<CG_STRIPE; j++ ) { // clear the heat static variables
        heat_cg[j]=0;
      }  
      for(unsigned int j=0; j<MN_STRIPE; j++ ) { // clear the heat static variables
        heat[j]=0;
      } 
    #else */
      for(unsigned int j=0; j<NUMPIXELS; j++ ) { // clear the heat static variables
        heat[j]=0;
      }  
    //#endif
  } // #endif
   //}  
  #endif  
} // lightRetract

void lightFlicker(uint8_t ledPins[],uint8_t type, uint8_t value = 0,cRGB maincolor={0,0,0}, cRGB clashcolor={0,0,0},uint8_t AState=0, int8_t StartPixel=-1 , int8_t StopPixel=-1) {
    uint8_t variation = abs(analogRead(SPK1) - analogRead(SPK2));
    uint8_t brightness;

    if (not value) {
  // Calculation of the amount of brightness to fade
       brightness = constrain(MAX_BRIGHTNESS
      - (abs(analogRead(SPK1) - analogRead(SPK2)))/8,0,255);
    } else {
      brightness = value;
    }
  #if defined LS_HEAVY_DEBUG
    Serial.print(F("Brightness: "));
    Serial.print(brightness);
    Serial.print(F("   SPK1: "));
    Serial.print(analogRead(SPK1));
    Serial.print(F("   SPK2: "));
    Serial.println(analogRead(SPK2));
  #endif
  
  #if defined LEDSTRINGS

  
    switch (type) {
      case 0:
      // // AudioTracker Flickering
      for (uint8_t i = 0; i <= 5; i++) {
        analogWrite(ledPins[i], brightness);
      }
      break;
      case 1:
      // anarchic Flickering
      for (uint8_t i = 0; i <= 5; i++) {
        if (i != flickerPos)
        analogWrite(ledPins[i], brightness - variation / 2);
        else
        analogWrite(ledPins[i], MAX_BRIGHTNESS);
      }
      if ((flickerPos != 0
          and millis() - lastFlicker > (120 - (100 - 15 * flickerPos)))
          or (flickerPos == 0 and millis() - lastFlicker > 300)) {
        flickerPos++;
        lastFlicker = millis();
        if (flickerPos == 6) {
          flickerPos = 0;
        }
      }
      break;
      case 2:
      // pulse Flickering
      if (((millis()-lastFlicker>=PULSEDURATION/PULSEFLICKERDEPTH) and AState != AS_BLADELOCKUP) or ((millis()-lastFlicker>=2) and AState == AS_BLADELOCKUP)) {
        lastFlicker=millis();
        for (uint8_t i = 0; i <= 5; i++) {
          analogWrite(ledPins[i],MAX_BRIGHTNESS - pulseflicker_pwm);
        }
        if (pulsedir) {
          pulseflicker_pwm++;
        }
        else {
          pulseflicker_pwm--;
        }
        if (pulseflicker_pwm == PULSEFLICKERDEPTH) { 
          pulsedir=false;
        }
        else if (pulseflicker_pwm == 0) {
          pulsedir=true;
        }
      }
      break;
    }
  #endif
  
  #if defined STAR_LED
  switch (type) {
    case 0: // AudioTracker Flickering
        if (AState==AS_BLADELOCKUP) { //animate blade in lockup mode
          // gives 25% chance to flick larger range for better randomization
          int lockupFlick = random(0,39); 
          if (lockupFlick < 25) {
            analogWrite(LED_RED, (brightness * maincolor.r / rgbFactor)); // RED
            analogWrite(LED_GREEN, (brightness * maincolor.g / rgbFactor)); // GREEN
            analogWrite(LED_BLUE, (brightness * maincolor.b / rgbFactor)); // BLUE  
          } else if (lockupFlick < 35) {
            analogWrite(LED_RED, (brightness * clashcolor.r / rgbFactor)); // RED
            analogWrite(LED_GREEN, (brightness * clashcolor.g / rgbFactor)); // GREEN
            analogWrite(LED_BLUE, (brightness * clashcolor.b / rgbFactor)); // BLUE  
          }
          else  { // simple white
            analogWrite(LED_RED, MAX_BRIGHTNESS); // RED
            analogWrite(LED_GREEN, MAX_BRIGHTNESS); // GREEN
            analogWrite(LED_BLUE, MAX_BRIGHTNESS); // BLUE  
          }
        }
        else {
          analogWrite(LED_RED, (brightness * maincolor.r / rgbFactor)); // RED
          analogWrite(LED_GREEN, (brightness * maincolor.g / rgbFactor)); // GREEN
          analogWrite(LED_BLUE, (brightness * maincolor.b / rgbFactor)); // BLUE  
        }
      break;
    case 1: // pulse flickering
      if (((millis()-lastFlicker>=PULSEDURATION/PULSEFLICKERDEPTH) and AState != AS_BLADELOCKUP) or ((millis()-lastFlicker>=2) and AState == AS_BLADELOCKUP)) {
        lastFlicker=millis();
        analogWrite(LED_RED, ((MAX_BRIGHTNESS - pulseflicker_pwm) * maincolor.r / rgbFactor)); // RED
        analogWrite(LED_GREEN, ((MAX_BRIGHTNESS - pulseflicker_pwm) * maincolor.g / rgbFactor)); // GREEN
        analogWrite(LED_BLUE, ((MAX_BRIGHTNESS - pulseflicker_pwm) * maincolor.b / rgbFactor)); // BLUE  
        if (pulsedir) {
          pulseflicker_pwm++;
        }
        else {
          pulseflicker_pwm--;
        }
        if (pulseflicker_pwm == PULSEFLICKERDEPTH) { 
          pulsedir=false;
        }
        else if (pulseflicker_pwm == 0) {
          pulsedir=true;
        }
      }
      break;
    case 2: // static blade
      analogWrite(LED_RED, maincolor.r ); // RED
      analogWrite(LED_GREEN, maincolor.g); // GREEN
      analogWrite(LED_BLUE, maincolor.b); // BLUE  
      break;      
  }
  #endif

  #ifdef ADF_PIXIE_BLADE
      for(uint8_t i=0; i< NUMPIXELS; i++) {
        Serial.print("\t");Serial.print(brightness);Serial.print("\t");Serial.print(maincolor.g);Serial.print("\t");Serial.println((brightness * maincolor.r) / rgbFactor);
        //strip.setPixelColor(i, ((brightness * maincolor.r) / rgbFactor), ((brightness * maincolor.r) / rgbFactor), ((brightness * maincolor.r) / rgbFactor));//maincolor.r, maincolor.g, maincolor.b);
        strip.setPixelColor(i, maincolor.r,maincolor.g, maincolor.b);//maincolor.r, maincolor.g, maincolor.b);
      }
      strip.show();
  #endif
  
  #ifdef PIXELBLADE
      if (StartPixel == -1 or StopPixel==-1 or StopPixel<StartPixel or StartPixel>NUMPIXELS or StopPixel>NUMPIXELS) {  // if neither start nor stop is defined or invalid range, go through the whole stripe    // neopixel ramp code from jbkuma
        StartPixel=0;
        StopPixel= NUMPIXELS; 
      }
      int flickFactor = random(0,255);
      if (flickFactor > 3 && flickFactor < 170) { flickFactor = 255; }
      //brightness = 255 * flickFactor / 100;
      brightness = flickFactor;
      cRGB color;
  
      switch (type) {
        case 0:
        // AudioTracker Flickering
        if (AState==AS_BLADELOCKUP) { //animate blade in lockup mode
          // gives 25% chance to flick larger range for better randomization
          int lockupFlick = random(0,39); 
          if (lockupFlick < 20) {
            color.r = brightness * maincolor.r / rgbFactor;
            color.g = brightness * maincolor.g / rgbFactor;
            color.b = brightness * maincolor.b / rgbFactor;
          } else if (lockupFlick < 30) {
            color.r = brightness * clashcolor.r / rgbFactor;
            color.g = brightness * clashcolor.g / rgbFactor;
            color.b = brightness * clashcolor.b / rgbFactor;
          }
          else  { // simple white
            color.r = MAX_BRIGHTNESS;
            color.g = MAX_BRIGHTNESS;
            color.b = MAX_BRIGHTNESS;
          }
        } else {  //normal operation
          color.r = brightness * maincolor.r / rgbFactor;
          color.g = brightness * maincolor.g / rgbFactor;
          color.b = brightness * maincolor.b / rgbFactor;
        }
      
        for (uint16_t i = StartPixel; i <= StopPixel; i++) {
            pixels.set_crgb_at(i, color); 
        }
        pixels.sync();
        break;
      case 1:
        // Pulse flickering
      // pulse Flickering
        if (((millis()-lastFlicker>=PULSEDURATION/PULSEFLICKERDEPTH) and AState != AS_BLADELOCKUP) or (AState == AS_BLADELOCKUP)) {
          lastFlicker=millis();
          color.r = (MAX_BRIGHTNESS - pulseflicker_pwm) * maincolor.r / rgbFactor;
          color.g = (MAX_BRIGHTNESS - pulseflicker_pwm) * maincolor.g / rgbFactor;
          color.b = (MAX_BRIGHTNESS - pulseflicker_pwm) * maincolor.b / rgbFactor;
          if (pulsedir) {
            pulseflicker_pwm++;
          }
          else {
            pulseflicker_pwm--;
          }
          if (pulseflicker_pwm == PULSEFLICKERDEPTH) { 
            pulsedir=false;
          }
          else if (pulseflicker_pwm == 0) {
            pulsedir=true;
          }
           for (uint16_t i = StartPixel; i <= StopPixel; i++) {
              pixels.set_crgb_at(i, color); 
          }
  
          pixels.sync();
        }

          break;
      case 2: // fire blade red
        if (fireblade) { // #ifdef FIREBLADE
        
          if (AState==AS_BLADELOCKUP) {
            Fire_Cooling=150;
            Fire_Sparking=50;
          }
          else {
            Fire_Cooling=50;
            Fire_Sparking=100;  
          }
            FireBlade(0);
            pixels.sync(); // Sends the data to the LEDs
        }
        break;
      case 3: // fire blade green
        if (fireblade) { // #ifdef FIREBLADE
        
          if (AState==AS_BLADELOCKUP) {
            Fire_Cooling=200;
            Fire_Sparking=70;
          }
          else {
            Fire_Cooling=100;
            Fire_Sparking=150;  
          }
            FireBlade(1);
            pixels.sync(); // Sends the data to the LEDs
        }
        break;
      case 4: // fire blade blue
        if (fireblade) { // #ifdef FIREBLADE
        
          if (AState==AS_BLADELOCKUP) {
            Fire_Cooling=100;
            Fire_Sparking=20;
          }
          else {
            Fire_Cooling=50;
            Fire_Sparking=100;  
          }
            FireBlade(2);
            pixels.sync(); // Sends the data to the LEDs
        }
        break;
      }
    //} // #endif
  #endif
} // lightFlicker

void ColorMixing(cRGB colorID={0,0,0}, int8_t mod=-1, uint8_t maxBrightness=MAX_BRIGHTNESS, bool Saturate=false) {
  #if defined LEDSTRINGS
  
  #endif
  

  #if defined PIXELBLADE or defined STAR_LED or defined ADF_PIXIE_BLADE
    cRGB mixedColor;
    mixedColor.r=colorID.r;
    mixedColor.g=colorID.g;
    mixedColor.b=colorID.b;
      switch(mod) {
        case(0):
          if (Saturate) {
            mixedColor.r=maxBrightness;
          }
          else {
            mixedColor.r=constrain(colorID.r+1,0,255);
          }
          break;
        case(1):
          if (Saturate) {
            mixedColor.r=0;
          }
          else {
            mixedColor.r=constrain(colorID.r-1,0,255);
          }
          break;
        case(2):
          if (Saturate) {
            mixedColor.g=maxBrightness;
          }
          else {
            mixedColor.g=constrain(colorID.g+1,0,255);
          }
          break;
        case(3):
          if (Saturate) {
            mixedColor.g=0;
          }
          else {
            mixedColor.g=constrain(colorID.g-1,0,255);
          }
          break;
        case(4):
          if (Saturate) {
            mixedColor.b=maxBrightness;
          }
          else {
            mixedColor.b=constrain(colorID.b+1,0,255);
          }
          break;
        case(5):
          if (Saturate) {
            mixedColor.b=0;
          }
          else {
            mixedColor.b=constrain(colorID.b-1,0,255);
          }
          break; 
      }
        getColor(mixedColor);
        //lightOn(mixedColor, 0, NUMPIXELS-6);
        #if defined LS_DEBUG
          //Serial.print(storage.sndProfile[storage.soundFont].mainColor);
          Serial.print("\tR:");
          Serial.print(currentColor.r);
          Serial.print("\tG:");
          Serial.print(currentColor.g);
          Serial.print(" \tB:");
          Serial.println(currentColor.b);
        #endif
          #if defined STAR_LED
            // LED_RED, LED_GREEN, LED_BLUE
            analogWrite(LED_RED,currentColor.r); // RED
            analogWrite(LED_GREEN, currentColor.g); // GREEN
            analogWrite(LED_BLUE, currentColor.b); // BLUE  
          #endif
          #ifdef ADF_PIXIE_BLADE
            for(uint8_t i=0; i< NUMPIXELS; i++) {
              strip.setPixelColor(i, currentColor.r, currentColor.g, currentColor.b);
            }
            strip.show();
          #endif
  #endif
} // ColorMixing

void lightBlasterEffect(uint8_t ledPins[], uint8_t pixel, uint8_t range, cRGB SndFnt_MainColor={0,0,0}) {
  #if defined LEDSTRINGS
    analogWrite(ledPins[random(1,5)], LOW); 
    delay(BLASTER_FX_DURATION); 
  #endif
  
  #if defined STAR_LED
    lightOn(ledPins, -1, currentColor); 
    delay(BLASTER_FX_DURATION);  
  #endif
  
  #ifdef ADF_PIXIE_BLADE
    for(uint8_t i=0; i< NUMPIXELS; i++) {
      strip.setPixelColor(i, currentColor.r, currentColor.g, currentColor.b);
    }
    strip.show();
    delay(BLASTER_FX_DURATION); 
  #endif
            
  #ifdef PIXELBLADE
    cRGB blastcolor;
    cRGB fadecolor;
    blastcolor.r=currentColor.r;
    blastcolor.g=currentColor.g;
    blastcolor.g=currentColor.b;
    getColor(SndFnt_MainColor);  // get the main blade color for the fading effect
    for (uint8_t i = 0; i<=2*range-1;i++) {
      if (fireblade) {
        // fully cool down (switch off LED) of a small segment of the blade, which will go up afterwards
        heat[pixel-i] = 0; // white hot fire burst along the whole blade
      }
      else {
        uint8_t j=i+pixel;
        if (j==pixel or j==pixel+2*range) { // 2 pixels at the edges shall be shut down
          fadecolor.r = 0;
          fadecolor.g = 0;
          fadecolor.b = 0;
          pixels.set_crgb_at(j, fadecolor);
        }
        else if (j==pixel+range+1) { // middle pixel full white
          fadecolor.r = MAX_BRIGHTNESS;
          fadecolor.g = MAX_BRIGHTNESS;
          fadecolor.b = MAX_BRIGHTNESS;
          pixels.set_crgb_at(j, fadecolor);
        }
        else { // rest of the pixels between middle and edge
          pixels.set_crgb_at(j, blastcolor);
        }
        /* 
        for (uint8_t j = 0; j <=range; j++) {
          if (j==i) {
            pixels.set_crgb_at(pixel-j, blastcolor);
            pixels.set_crgb_at(pixel+j, blastcolor);
          }
          else {
            pixels.set_crgb_at(pixel-j, currentColor);
            pixels.set_crgb_at(pixel+j, currentColor);
          }
        }*/
        pixels.sync();
        if (not fireblade) {
          delay(BLASTER_FX_DURATION/(2*range));  // blast deflect should last for ~500ms
        }
      }
    }
  #endif
} // lightBlasterEffect

void pixelblade_KillKey_Enable() {
  #if defined PIXELBLADE or defined ADF_PIXIE_BLADE
    // cut power to the neopixels stripes by disconnecting their GND signal using the LS pins
    #ifdef DIYINO_STARDUST
    for (uint8_t j = 0; j < 3; j++) {
    #endif
    #ifdef DIYINO_PRIME  
    for (uint8_t j = 0; j < 6; j++) {
    #endif
      digitalWrite(ledPins[j], LOW);
    }
      //digitalWrite(3, LOW);
      //digitalWrite(5, LOW);
      //digitalWrite(6, LOW);
      //digitalWrite(9, LOW);
      //digitalWrite(10, LOW);
      //digitalWrite(11, LOW);
      #ifdef PIXELBLADE
        digitalWrite(DATA_PIN,HIGH); // in order not to back-connect GND over the Data pin to the stripes when the Low-Sides disconnect it
      #else if ADF_PIXIE_BLADE
        digitalWrite(PIXIEPIN,HIGH); // in order not to back-connect GND over the Data pin to the stripes when the Low-Sides disconnect it
      #endif
  #endif      
}

void pixelblade_KillKey_Disable() {
  #if defined PIXELBLADE or defined ADF_PIXIE_BLADE
    // cut power to the neopixels stripes by disconnecting their GND signal using the LS pins
    #ifdef DIYINO_STARDUST
    for (uint8_t j = 0; j < 3; j++) {
    #endif
    #ifdef DIYINO_PRIME  
    for (uint8_t j = 0; j < 6; j++) {
    #endif
      digitalWrite(ledPins[j], HIGH);
    }
      //digitalWrite(3, HIGH);
      //digitalWrite(5, HIGH);
      //digitalWrite(6, HIGH);
      //digitalWrite(9, HIGH);
      //digitalWrite(10, HIGH);
      //digitalWrite(11, HIGH);
  #endif
}

void lightClashEffect(uint8_t ledPins[], cRGB color) {

  #if defined LEDSTRINGS
    for (uint8_t i = 0; i <= 5; i++) {
      analogWrite(ledPins[i], 255);
    }
    delay(CLASH_FX_DURATION);  // clash duration
  #endif
  
  #if defined STAR_LED
    getColor(color);
    lightOn(ledPins, -1, currentColor);
    delay(CLASH_FX_DURATION);  // clash duration
  #endif

  #ifdef ADF_PIXIE_BLADE
    //getColor(storage.sndProfile[storage.soundFont].clashColor);
    lightOn(ledPins, -1, color);
    delay(CLASH_FX_DURATION);  // clash duration
  #endif
  
  #if defined PIXELBLADE
    if (fireblade) { // #if defined FIREBLADE  // simply flash white
          //getColor(storage.sndProfile[storage.soundFont].clashColor);
          //lightOn(ledPins, -1, currentColor);
          for( int i = 0; i < NUMPIXELS; i++) {
            // the random() function in this loop causes phantom swings
            heat[i] = constrain(heat[i]+70,0,255); // white hot fire burst along the whole blade
          }
          
    } // fireblade
    else { // #else
          getColor(storage.sndProfile[storage.soundFont].clashColor);
          lightOn(ledPins, -1, currentColor);
          delay(CLASH_FX_DURATION);  // clash duration
    } // #endif
  #endif
  
}

#if defined COLOR_PROFILE and not defined LEDSTRINGS
void getColorFix(uint8_t colorID) {
  switch (colorID) {
  case 0:
//Red
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = 0;
    currentColor.b = 0;
    break;
  case 1:
//Orange
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = MAX_BRIGHTNESS/4;
    currentColor.b = 0;
    break;
  case 2:
//Amber
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = MAX_BRIGHTNESS*66/100;
    currentColor.b = 0;
    break;
  case 3:
//Yellow
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = 0;
    break;
  case 4:
//Lime
    currentColor.r = MAX_BRIGHTNESS*34/100;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = 0;
    break;
  case 5:
//Green
    currentColor.r = 0;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = 0;
    break;
  case 6:
//Cyan
    currentColor.r = 0;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = MAX_BRIGHTNESS*34/100;
    break;
  case 7:
//Light Blue
    currentColor.r = 0;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = MAX_BRIGHTNESS*63/100;
    break;
  case 8:
//Ice Blue
    currentColor.r = 0;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = MAX_BRIGHTNESS;
    break;
  case 9:
//Blue
    currentColor.r = 0;
    currentColor.g = 0;
    currentColor.b = MAX_BRIGHTNESS;
    break;
  case 10:
//Mauve
    currentColor.r = MAX_BRIGHTNESS*12/100;
    currentColor.g = 0;
    currentColor.b = MAX_BRIGHTNESS;
    break;
  case 11:
//Purple
    currentColor.r = MAX_BRIGHTNESS*35/100;
    currentColor.g = 0;
    currentColor.b = MAX_BRIGHTNESS;
    break;
  case 12:
//Pink
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = 0;
    currentColor.b = MAX_BRIGHTNESS;
    break;
  case 13:
//Crimson
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = 0;
    currentColor.b = MAX_BRIGHTNESS*5/100;
    break;
  case 14:
//White
    currentColor.r = MAX_BRIGHTNESS;
    currentColor.g = MAX_BRIGHTNESS;
    currentColor.b = MAX_BRIGHTNESS;
    break;
  default:
// White (if enough voltage)
    currentColor.r = 100;
    currentColor.g = 100;
    currentColor.b = 100;
    break;
  }
} //getColorFix
#endif

void BladeMeter (uint8_t ledPins[], int meterLevel) {  //expects input of 0-100
  //normalize data if to max and min if out of range
  if (meterLevel <= 0) { meterLevel = 0; } 
  if (meterLevel >= 100) { meterLevel = 100; }

#ifdef LEDSTRINGS // light blade as 6 segment bar graph
  meterLevel = meterLevel*6/100;
    for (uint8_t i = 0; i < sizeof(ledPins); i++) {
      if (i <= meterLevel) {
        digitalWrite(ledPins[i], HIGH);
      } else {
        digitalWrite(ledPins[i], LOW);
      }
    }
#endif

#ifdef STAR_LED // light led in gradient from red to green
  analogWrite(LED_RED, (MAX_BRIGHTNESS * (100 - meterLevel))/255);
  analogWrite(LED_GREEN, (MAX_BRIGHTNESS * meterLevel)/255);
  //Serial.println((MAX_BRIGHTNESS * meterLevel)/255);
//  unsigned int meterGreen = meterLevel * 255 / 100;
//  unsigned int meterRed = (100 - meterLevel) * 255 / 100;
#endif

#ifdef PIXELBLADE // light blade as 3 color meter proportionate to length
  cRGB value;
  //set first pixel for accent LED compatability
  if (meterLevel < 30) {
    value.r = MAX_BRIGHTNESS/2;
    value.g = 0;
    value.b = 0;
  } else if (meterLevel < 60) {
    value.r = MAX_BRIGHTNESS/2*0.8;
    value.g = MAX_BRIGHTNESS/2*0.6;
    value.b = 0;
  } else {
    value.r = 0;
    value.g = MAX_BRIGHTNESS/2;
    value.b = 0;
  }
  pixels.set_crgb_at(0, value);

  //set rest of blade
  for (unsigned int i = 1; i < NUMPIXELS; i++) { // turn on/off one LED at a time
      if (i < NUMPIXELS * meterLevel / 100){
        if (i < (30 * NUMPIXELS / 100)) {
          value.r = MAX_BRIGHTNESS;
          value.g = 0;
          value.b = 0;
        } else if (i < (60 * NUMPIXELS / 100)) {
          value.r = MAX_BRIGHTNESS;
          value.g = MAX_BRIGHTNESS;
          value.b = 0;
        } else {
          value.r = 0;
          value.g = MAX_BRIGHTNESS;
          value.b = 0;
        }
      } else {
      value.r=0;
      value.g=0;
      value.b=0;      
      }      
      pixels.set_crgb_at(i, value);
    }
    pixels.sync(); // Sends the data to the LEDs
//    delay(3);
#endif
}

#ifdef ADF_PIXIE_BLADE
void InitAdafruitPixie(uint8_t ledPins[]) {
  //pixieSerial.setSerial(-1, PIXIEPIN);
  pixieSerial.begin(115200); // Pixie REQUIRES this baud rate
  strip.setBrightness(MAX_BRIGHTNESS);  // Adjust as necessary to avoid blinding
  /*pixelblade_KillKey_Disable();
  Serial.println("Red!");
  for(uint8_t i=0; i< NUMPIXELS; i++)
    strip.setPixelColor(i, 255, 0, 0);
  strip.show();
  delay(300);

  Serial.println("Green!");
  for(uint8_t i=0; i< NUMPIXELS; i++)
    strip.setPixelColor(i, 0, 255, 0);
  strip.show();
  delay(300);

  Serial.println("Blue!");
  for(uint8_t i=0; i< NUMPIXELS; i++)
    strip.setPixelColor(i, 0, 0, 255);
  strip.show();
  delay(300);
  currentColor.r = 0;
  currentColor.g = 200;
  currentColor.b = 255;
  lightOn(ledPins, -1, currentColor);
  delay(300);
    for(uint8_t i=0; i< NUMPIXELS; i++)
    strip.setPixelColor(i, 255, 0, 0);
  strip.show();
  delay(1000);
  lightOff();
  getColor(storage.sndProfile[storage.soundFont].mainColor);
  pixelblade_KillKey_Enable();*/
}
#endif



#if defined FoCSTRING
void FoCOn(uint8_t pin) {
	digitalWrite(FoCSTRING, HIGH);
//	PORTC &= ~(1 << PD3);

} //FoCOn
void FoCOff(uint8_t pin) {
	digitalWrite(FoCSTRING, LOW);
//	PORTC |= (1 << PD3);
} //FoCOff
#endif



#ifdef JUKEBOX
void JukeBox_Stroboscope(uint8_t ledPins[]) {
 uint16_t variation = 0;
 uint16_t temp_variation=0;
 for (uint8_t i=0; i<=SAMPLESIZEAVERAGE-1;i++) {
  temp_variation=temp_variation + abs(analogRead(SPK1) - analogRead(SPK2));
 }
 variation=temp_variation/SAMPLESIZEAVERAGE;
  if (variation>=80) {
    analogWrite(ledPins[0], MAX_BRIGHTNESS);
  }
  else analogWrite(ledPins[0], 0);
  if (variation>=110) {
    analogWrite(ledPins[1], MAX_BRIGHTNESS);
  }
  else analogWrite(ledPins[1], 0);  
  if (variation>=140) {
    analogWrite(ledPins[2], MAX_BRIGHTNESS);
  }
  else analogWrite(ledPins[2], 0);  
  if (variation>=170) {
    analogWrite(ledPins[3], MAX_BRIGHTNESS);
  }
  else analogWrite(ledPins[3], 0);
  if (variation>=200) {
    analogWrite(ledPins[4], MAX_BRIGHTNESS);
  }
  else analogWrite(ledPins[4], 0);
  if (variation>=230) {
    analogWrite(ledPins[5], MAX_BRIGHTNESS);
  }
  else analogWrite(ledPins[5], 0);
  //delay(50);
}
#endif



#ifdef JUKEBOX
void JukeBox_Stroboscope(cRGB color) {

 uint16_t variation = 0;
 uint16_t temp_variation=0;
 cRGB tempcolor;

 for (uint8_t i=0; i<=SAMPLESIZEAVERAGE-1;i++) {
  temp_variation=temp_variation + constrain(abs(analogRead(SPK1) - analogRead(SPK2)),0,512);
  //Serial.println(abs(analogRead(SPK1) - analogRead(SPK2)));
 }
 variation=temp_variation/SAMPLESIZEAVERAGE;
  // assumption -> variation max 280
  //Serial.print("\t");Serial.println(variation);


  for (uint16_t i = 1; i <= variation; i++) {
    pixels.set_crgb_at(i, color);
  }
  tempcolor.r = 0;
  tempcolor.g = 0;
  tempcolor.b = 0; // RGB Value -> Off
  for (uint16_t i = (variation)+1; i <= NUMPIXELS; i++) {
    pixels.set_crgb_at(i, tempcolor);
  }
  pixels.sync();  

}
#endif


#ifdef PIXELBLADE
//#ifdef FIREBLADE
void FireBlade(uint8_t DominantColor) {
// Array of temperature readings at each simulation cell
  int pixelnumber;
  
  // Step 1.  Cool down every cell a little
/*#ifdef CROSSGUARDSABER
    for( int i = 0; i < MN_STRIPE; i++) {
      heat[i] = constrain(heat[i] - random(((Fire_Cooling * 10) / MN_STRIPE) + 2),0,255);
    }
    for( int i = 0; i < CG_STRIPE; i++) {
      heat_cg[i] = constrain(heat_cg[i] - random(5),0,255);
    }
#else */
    for( int i = 0; i < NUMPIXELS; i++) {
      // the random() function in this loop causes phantom swings
      heat[i] = constrain(heat[i] - random(((Fire_Cooling * 10) / NUMPIXELS) + 2),0,255);
    }
//#endif

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
/*#ifdef CROSSGUARDSABER
    for( int k= MN_STRIPE - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    for( int k= CG_STRIPE - 1; k >= 2; k--) {
      heat_cg[k] = (heat_cg[k - 1] + heat_cg[k - 2] + heat_cg[k - 2] ) / 3;
    }
#else*/
    for( int k= NUMPIXELS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
//#endif
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
/*#ifdef CROSSGUARDSABER
    if( random(255) < Fire_Sparking ) {
      int y = random(7);
      heat[y] = constrain(heat[y] + random(95)+160,0,255 );
    }
    if( random(255) < 10 ) {
      int y = random(4);
      heat_cg[y] = constrain(heat_cg[0] + random(95)+160,0,255 );  
    } 
#else*/
    if( random(255) < Fire_Sparking ) {
      int y = random(7);
      heat[y] = constrain(heat[y] + random(95)+160,0,255 );
    }
//#endif

    // Step 4.  Map from heat cells to LED colors 
/*#ifdef CROSSGUARDSABER
    for( int j = 0; j < CG_STRIPE; j++) {
      cRGB color = HeatColor( heat_cg[j],DominantColor);
      //if( gReverseDirection ) {
      //  pixelnumber = (CG_STRIPE-1) - j;
      //} else {
      //  pixelnumber = j;
      //}
      LED.set_crgb_at(j, color); // Set value at LED found at index j
    }
    for( int j = CG_STRIPE; j < CG_STRIPE + MN_STRIPE; j++) {
      cRGB color = HeatColor( heat[j],DominantColor);
      //if( gReverseDirection ) {
      //  pixelnumber = (CG_STRIPE + MN_STRIPE-1) - j;
      //} else {
      //  pixelnumber = j;
      //}
      pixels.set_crgb_at(j, color); // Set value at LED found at index j
    }
#else*/
    for( int j = 0; j < NUMPIXELS; j++) {
       cRGB color = HeatColor( heat[j],DominantColor);
        pixels.set_crgb_at(j, color); // Set value at LED found at index j
    }
//#endif
}

// CRGB HeatColor( uint8_t temperature)
//
// Approximates a 'black body radiation' spectrum for
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation'
// spectrum, but it's surprisingly close, and it's fast and small.
//
// On AVR/Arduino, this typically takes around 70 bytes of program memory,
// versus 768 bytes for a full 256-entry RGB lookup table.

cRGB HeatColor( uint8_t temperature, uint8_t DominantColor)
{
    cRGB heatcolor;

    // Scale 'heat' down from 0-255 to 0-191,
    // which can then be easily divided into three
    // equal 'thirds' of 64 units each.
    uint8_t t192 = scale8_video( temperature, 192);
     //Serial.print(F("scale8_video_result: "));
     //Serial.print(temperature);Serial.print("/t");Serial.println(t192);

    // calculate a value that ramps up from
    // zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252

    // now figure out which third of the spectrum we're in:
    if( t192 & 0x80) {
        if (DominantColor==0 or DominantColor>2) { // red
          // we're in the hottest third
          heatcolor.r = 255; // full red
          heatcolor.g = 255; // full green
          heatcolor.b = heatramp; // ramp up blue
        }
        else if (DominantColor==1) { // green
          // we're in the hottest third
          heatcolor.g = 255; // full green
          heatcolor.b = 255; // full blue
          heatcolor.r = heatramp; // ramp up red
        }
        else if (DominantColor==2) { // blue
          // we're in the hottest third
          heatcolor.b = 255; // full blue
          heatcolor.g = 255; // full green
          heatcolor.r = heatramp; // ramp up red
        }
      } else if( t192 & 0x40 ) {
        // we're in the middle third
        if (DominantColor==0 or DominantColor>2) { // red
          heatcolor.r = 255; // full red
          heatcolor.g = heatramp; // ramp up green
          heatcolor.b = 0; // no blue
        }
        else if (DominantColor==1) { // green
          heatcolor.g = 255; // full green
          heatcolor.b = heatramp; // ramp up blue
          heatcolor.r = 0; // no red
        }
        else if (DominantColor==2) { // blue
          heatcolor.b = 255; // full blue
          heatcolor.g = heatramp; // ramp up green
          heatcolor.r = 0; // no red
        }
    } else {
        // we're in the coolest third
        if (DominantColor==0 or DominantColor>2) { // red
          heatcolor.r = heatramp; // ramp up red
          heatcolor.g = 0; // no green
          heatcolor.b = 0; // no blue
        }
        else if (DominantColor==1) { // green
          heatcolor.g = heatramp; // ramp up green
          heatcolor.b = 0; // no blue
          heatcolor.r = 0; // no red          
        }
        else if (DominantColor==2) { // blue
          heatcolor.b = heatramp; // ramp up blue
          heatcolor.g = 0; // no green
          heatcolor.r = 0; // no red         
        }
    }

    return heatcolor;
}

uint8_t scale8_video( uint8_t i, uint8_t scale)
{
//    uint8_t j = (((int)i * (int)scale) >> 8) + ((i&&scale)?1:0);
//    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
//    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
//    return j;
    uint8_t j=0;
    asm volatile(
        "  tst %[i]\n\t"
        "  breq L_%=\n\t"
        "  mul %[i], %[scale]\n\t"
        "  mov %[j], r1\n\t"
        "  clr __zero_reg__\n\t"
        "  cpse %[scale], r1\n\t"
        "  subi %[j], 0xFF\n\t"
        "L_%=: \n\t"
        : [j] "+a" (j)
        : [i] "a" (i), [scale] "a" (scale)
        : "r0", "r1");

    return j;
}

/*uint8_t DominantMainColor(cRGB color={0,0,0}) {
  uint8_t tmp_d;
  if (color.r>=color.g and color.r>=color.b) {
    tmp_d=0; // red
  }
  else if (color.g>=color.r and color.g>=color.b) {
    tmp_d=1; // green
  }
  else if (color.b>=color.r and color.b>=color.g) {
    tmp_d=2; // blue
  }
  else {
    tmp_d=0; // red default
  }
  return tmp_d;
}*/
#endif

#ifdef COLOR_PROFILE
cRGB setColorProfile(cRGB currentcolor) {
// Red  
colorProfiles[0].r=255;
colorProfiles[0].g=0;
colorProfiles[0].b=0;
// Green  
colorProfiles[1].r=0;
colorProfiles[1].g=255;
colorProfiles[1].b=0;
// Blue  
colorProfiles[2].r=0;
colorProfiles[2].g=0;
colorProfiles[2].b=255;
// Orange  
colorProfiles[3].r=255;
colorProfiles[3].g=100;
colorProfiles[3].b=0;
// Cyan  
colorProfiles[4].r=0;
colorProfiles[4].g=100;
colorProfiles[4].b=255;
}
#endif
#if defined ACCENT_LED

void accentLEDControl( AccentLedAction_En AccentLedAction) {

  if (AccentLedAction==AL_PULSE) {
    #if defined HARD_ACCENT
        if (millis() - lastAccent <= 400) {
          analogWrite(ACCENT_LED, millis() - lastAccent);
        } else if (millis() - lastAccent > 400
            and millis() - lastAccent <= 800) {
          analogWrite(ACCENT_LED, 800 - (millis() - lastAccent));
        } else {
          lastAccent = millis();
        }
    #endif
    
    #if defined SOFT_ACCENT
    
        PWM();
    
        if (millis() - lastAccent >= 20) {
          // moved to own funciton for clarity
          fadeAccent();
          lastAccent = millis();
        }
    #endif
  }
  else if (AccentLedAction==AL_ON) {
    digitalWrite(ACCENT_LED,HIGH);
  }
  else {  // AL_OFF
    digitalWrite(ACCENT_LED,LOW);    
  }
}

#if defined SOFT_ACCENT

void PWM() {

  if (micros() - lastAccentTick >= 8) {

    if (pwmPin.state == LOW) {
      if (pwmPin.tick >= pwmPin.dutyCycle) {
        pwmPin.state = HIGH;
      }
    } else {
      if (pwmPin.tick >= abs(100 - pwmPin.dutyCycle)) {
        pwmPin.state = LOW;
        pwmPin.tick = 0;
      }
    }
    pwmPin.tick++;
    digitalWrite(ACCENT_LED, pwmPin.state);
    lastAccentTick = micros();
  }
}

void fadeAccent() {
  // go through each sw pwm pin, and increase
  // the pwm value. this would be like
  // calling analogWrite() on each hw pwm pin
  if (not pwmPin.revertCycle) {
    pwmPin.dutyCycle++;
    if (pwmPin.dutyCycle == 100)
      pwmPin.revertCycle = true;
  } else {
    pwmPin.dutyCycle--;
    if (pwmPin.dutyCycle == 0)
      pwmPin.revertCycle = false;
  }
}
#endif
#endif



