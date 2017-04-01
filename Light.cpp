/*
 * Light.cpp
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 */
#include "Light.h"
#include "Config.h"

#if defined PIXELBLADE
#include <WS2812.h>
#endif

enum SaberStateEnum {S_STANDBY, S_SABERON, S_CONFIG, S_SLEEP, S_JUKEBOX};
enum ActionModeSubStatesEnum {AS_HUM, AS_IGNITION, AS_RETRACTION, AS_BLADELOCKUP, AS_BLASTERDEFLECTMOTION, AS_BLASTERDEFLECTPRESS, AS_CLASH, AS_SWING, AS_SPIN, AS_FORCE};
enum ConfigModeSubStatesEnum {CS_VOLUME, CS_SOUNDFONT, CS_MAINCOLOR, CS_CLASHCOLOR, CS_BLASTCOLOR, CS_FLICKERTYPE, CS_IGNITIONTYPE, CS_RETRACTTYPE, CS_SLEEPINIT, CS_BATTERYLEVEL};
extern SaberStateEnum SaberState;
extern SaberStateEnum PrevSaberState;
extern ActionModeSubStatesEnum ActionModeSubStates;
extern ConfigModeSubStatesEnum ConfigModeSubStates;

# if defined ACCENT_LED
unsigned long lastAccent = millis();
#if defined SOFT_ACCENT
unsigned long lastAccentTick = micros();
#endif
#endif

#ifdef JUKEBOX
#define SAMPLESIZEAVERAGE 30
#endif

#ifdef FIREBLADE
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
static uint8_t Fire_Cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
static uint8_t Fire_Sparking = 100;
#ifdef CROSSGUARDSABER
static byte heat[MN_STRIPE];  
static byte heat_cg[CG_STRIPE];
#else
static byte heat[NUMPIXELS];  
#endif
#endif  // FIREBLADE

// ====================================================================================
// ===              	    			LED FUNCTIONS		                		===
// ====================================================================================
#if defined LEDSTRINGS

#define I_BEGINNEXTSEGMENT 50
#define R_BEGINNEXTSEGMENT 100
#define PULSEFLICKERDEPTH 70
#define PULSEDURATION 1000

static uint8_t flickerPos = 0;
static long lastFlicker = 0;
uint8_t pulseflicker_pwm=0;
bool pulsedir=true;

void lightOn(uint8_t ledPins[], int8_t segment) {
// Light On

	if (segment == -1) {
		for (uint8_t i = 0; i < 6; i++) {
			digitalWrite(ledPins[i], HIGH);
		}
	} else {
		digitalWrite(ledPins[segment], HIGH);
	}
} //lightOn

void lightOff() {
// shut Off
	//Shut down PWM
	TCCR0A &= ~((1 << COM0A1) | (1 << COM0B1));
	TCCR1A &= ~((1 << COM1A1) | (1 << COM1B1));
	TCCR2A &= ~((1 << COM2A1) | (1 << COM2B1));
	//Shut down everything at once
	PORTB &= B11010001;
	PORTD &= B10010111;
} //lightOff



void lightIgnition(uint8_t ledPins[], uint16_t time, uint8_t type) {



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
}				//lightIgnition

void lightRetract(uint8_t ledPins[], uint16_t time, uint8_t type) {

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
     /*for (int8_t i = 5; i >= 0; i--) {
			//BUG CORRECTION:
			//Not uint8_t here because Arduino nano clones did go
			// on an infinite loop for no reason making the board
			// crash at some point.
			digitalWrite(ledPins[i], 0);
			delay(time / 5);
		}*/
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
}				//lightRetract

#if defined CLASHSTRING
void FoCOn(uint8_t pin) {
	digitalWrite(CLASHSTRING, HIGH);
//	PORTC &= ~(1 << PD3);

} //FoCOn
void FoCOff(uint8_t pin) {
	digitalWrite(CLASHSTRING, LOW);
//	PORTC |= (1 << PD3);
} //FoCOff
#endif

void lightFlicker(uint8_t ledPins[], uint8_t type, uint8_t value, uint8_t AState) {
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

	switch (type) {
		case 0:
		// std Flickering
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
} //lightFlicker

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

#endif
#if defined STAR_LED

extern cRGB currentColor;

void lightOn(uint8_t ledPins[], cRGB color) {
// Light On
    analogWrite(ledPins[0], MAX_BRIGHTNESS * color.r / rgbFactor); // RED
    analogWrite(ledPins[1], MAX_BRIGHTNESS * color.g / rgbFactor); // GREEN
    analogWrite(ledPins[2], MAX_BRIGHTNESS * color.b / rgbFactor); // BLUE
} //lightOn

void lightOff(uint8_t ledPins[]) {
// shut Off
	for (uint8_t i = 0; i <= 2; i++) {
		analogWrite(ledPins[i], LOW);
	}
} //lightOff

#ifndef COLORS
void ColorMixing(cRGB colorID, int8_t mod, uint8_t maxBrightness, bool Saturate) {
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
        //#if defined LS_DEBUG
          //Serial.print(storage.sndProfile[storage.soundFont].mainColor);
          Serial.print("\tR:");
          Serial.print(currentColor.r);
          Serial.print("\tG:");
          Serial.print(currentColor.g);
          Serial.print(" \tB:");
          Serial.println(currentColor.b);
        //#endif
      // LED_RED, LED_GREEN, LED_BLUE
      analogWrite(LED_RED,currentColor.r); // RED
      analogWrite(LED_GREEN, currentColor.g); // GREEN
      analogWrite(LED_BLUE, currentColor.b); // BLUE  
}
#endif // not COLORS
  
void lightIgnition(uint8_t ledPins[], cRGB color, uint16_t time) {
Serial.println("Ignition");
// Fade in to Maximum brightness
	for (uint8_t fadeIn = 255; fadeIn > 0; fadeIn--) {
    analogWrite(ledPins[0], (MAX_BRIGHTNESS / fadeIn) * color.r / rgbFactor); // RED
    analogWrite(ledPins[1], (MAX_BRIGHTNESS / fadeIn) * color.g / rgbFactor); // GREEN
    analogWrite(ledPins[2], (MAX_BRIGHTNESS / fadeIn) * color.b / rgbFactor); // BLUE
		delay(time / 255);
	}
} //lightIgnition

void lightRetract(uint8_t ledPins[], cRGB color, uint16_t time) {
// Fade out

	for (uint8_t fadeOut = 1; fadeOut < 255; fadeOut++) {
    analogWrite(ledPins[0], (MAX_BRIGHTNESS / fadeOut) * color.r / rgbFactor); // RED
    analogWrite(ledPins[1], (MAX_BRIGHTNESS / fadeOut) * color.g / rgbFactor); // GREEN
    analogWrite(ledPins[2], (MAX_BRIGHTNESS / fadeOut) * color.b / rgbFactor); // BLUE
		delay(time / 255);
	}

	lightOff(ledPins);
} //lightRetract

void lightFlicker(uint8_t ledPins[], cRGB color, uint8_t value) {
	uint8_t brightness;
	if (not value) {
// Calculation of the amount of brightness to fade
		brightness = MAX_BRIGHTNESS
		- (abs(analogRead(SPK1) - analogRead(SPK2)));
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
    analogWrite(ledPins[0], (brightness * color.r / rgbFactor)); // RED
    analogWrite(ledPins[1], (brightness * color.g / rgbFactor)); // GREEN
    analogWrite(ledPins[2], (brightness * color.b / rgbFactor)); // BLUE
} //lightFlicker


#ifdef COLORS
void getColor(uint8_t color) {
#else //  not COLOR
void getColor(cRGB color) {
#endif

#ifdef COLORS
  //currentColor[3] = colorID;
  switch (color) {
  case 0:
//Red
    currentColor.r = 100;
    currentColor.g = 0;
    currentColor.b = 0;
    break;
  case 1:
//Yellow
    currentColor.r = 100;
    currentColor.g = 100;
    currentColor.b = 0;
    break;
  case 2:
//Green
    currentColor.r = 0;
    currentColor.g = 100;
    currentColor.b = 0;
    break;
  case 3:
//Aqua
    currentColor.r = 0;
    currentColor.g = 100;
    currentColor.b = 100;
    break;
  case 4:
//Blue
    currentColor.r = 0;
    currentColor.g = 0;
    currentColor.b = 100;
    break;
  case 5:
//Fuschia
    currentColor.r = 100;
    currentColor.g = 0;
    currentColor.b = 100;
    break;

  case 6:
//DarkGrey
    currentColor.r = 100;
    currentColor.g = 100;
    currentColor.b = 100;
    break;
  case 7:
//DarkOrange
    currentColor.r = 1000;
    currentColor.g = 76;
    currentColor.b = 0;
    break;
  case 8:
//DarkViolet
    currentColor.r = 100;
    currentColor.g = 0;
    currentColor.b = 100;
    break;
  case 9:
//DodgerBlue
    currentColor.r = 24;
    currentColor.g = 80;
    currentColor.b = 100;
    break;
  case 10:
//Gold
    currentColor.r = 100;
    currentColor.g = 120;
    currentColor.b = 0;
    break;
  case 11:
//GoldenRod
    currentColor.r = 100;
    currentColor.g = 112;
    currentColor.b = 24;
    break;
  case 12:
//Indigo
    currentColor.r = 80;
    currentColor.g = 0;
    currentColor.b = 100;
    break;
  case 13:
//LightGreen
    currentColor.r = 90;
    currentColor.g = 100;
    currentColor.b = 90;
    break;

  default:
// White (if enough voltage)
    currentColor.r = 50;
    currentColor.g = 50;
    currentColor.b = 50;
    break;
  }
#else  // not COLOR
    currentColor.r = color.r;
    currentColor.g = color.g;
    currentColor.b = color.b;
#endif
} //getColor
#ifdef JUKEBOX
void JukeBox_Stroboscope() {
  
}
#endif


#endif

#if defined PIXELBLADE
static uint8_t flickerPos = 0;
static long lastFlicker = millis();
extern WS2812 pixels;

extern cRGB currentColor;

void pixelblade_KillKey_Enable() {
  // cut power to the neopixels stripes by disconnecting their GND signal using the LS pins
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
    digitalWrite(DATA_PIN,HIGH); // in order not to back-connect GND over the Data pin to the stripes when the Low-Sides disconnect it
}

void pixelblade_KillKey_Disable() {
  // cut power to the neopixels stripes by disconnecting their GND signal using the LS pins
    digitalWrite(3, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
}

void lightOn(cRGB color, int8_t StartPixel, int8_t StopPixel) {
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
} //lightOn

void lightOff() {
// shut Off
	cRGB value;
	value.b = 0;
	value.g = 0;
	value.r = 0; // RGB Value -> Off
	for (uint16_t i = 0; i < NUMPIXELS; i++) {
		pixels.set_crgb_at(i, value);
	}
	pixels.sync();
} //lightOff

#ifndef COLORS
void ColorMixing(cRGB colorID, int8_t mod, uint8_t maxBrightness, bool Saturate) {
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
  }
#endif

void lightIgnition(cRGB color, uint16_t time, uint8_t type) {
	cRGB value;
	value.r = MAX_BRIGHTNESS * color.r / rgbFactor;
	value.g = MAX_BRIGHTNESS * color.g / rgbFactor;
	value.b = MAX_BRIGHTNESS * color.b / rgbFactor;
	//switch (type) {
	//case 0:
// Light up the ledstrings Movie-like
    RampPixels(time, true);
/*		for (uint16_t i = 0; i < NUMPIXELS; i++) {
			pixels.set_crgb_at(i, value);
			i++;
			pixels.set_crgb_at(i, value);
			pixels.sync();
      //delay(time/NUMPIXELS);
			delayMicroseconds((time * 1000) / NUMPIXELS);
		}*/
		//Serial.println(pixels.getBrightness());
		//break;
		/*
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
		 */
	//}
}				//lightIgnition

void lightRetract(uint16_t time, uint8_t type) {
	//switch (type) {
	//case 0:
		// Light off the ledstrings Movie Like
		cRGB value;
		value.b = 0;
		value.g = 0;
		value.r = 0; // RGB Value -> Off
    RampPixels(time, false);
		/*for (uint16_t i = NUMPIXELS; i > 0; i--) {
			//BUG CORRECTION:
			//Not uint8_t here because Arduino nano clones did go
			// on an infinite loop for no reason making the board
			// crash at some point.
			pixels.set_crgb_at(i, value);
			i--;
			pixels.set_crgb_at(i, value);
			pixels.sync();
			delayMicroseconds((time * 1000) / NUMPIXELS);
		}*/
		//break;
		/*
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
		 */
	//}
#ifdef FIREBLADE
#ifdef CROSSGUARDSABER
  for(unsigned int j=0; j<STRIPE1; j++ ) { // clear the heat static variables
    heat_cg[j]=0;
  }  
  for(unsigned int j=0; j<STRIPE2; j++ ) { // clear the heat static variables
    heat[j]=0;
  } 
#else
  for(unsigned int j=0; j<NUMPIXELS; j++ ) { // clear the heat static variables
    heat[j]=0;
  }  
#endif
#endif
}				//lightRetract

#ifdef COLORS
void lightBlasterEffect(uint8_t pixel, uint8_t range, uint8_t SndFnt_MainColor) {
#else
void lightBlasterEffect(uint8_t pixel, uint8_t range, cRGB SndFnt_MainColor) {
#endif
  cRGB blastcolor;
  cRGB fadecolor;
  blastcolor.r=currentColor.r;
  blastcolor.g=currentColor.g;
  blastcolor.g=currentColor.b;
  getColor(SndFnt_MainColor);  // get the main blade color for the fading effect
  for (uint8_t i = 0; i<=2*range-1;i++) {
    for (uint8_t j = 0; j <=range; j++) {
  	//for (uint8_t j = (pixel - range); j < (pixel + range); j++) {
      //if (i<=range) {
        //fadecolor.r=((2*range-(i))*blastcolor.r + ((i)*currentColor.r))/2*range;
        //fadecolor.g=((2*range-(i))*blastcolor.g + ((i)*currentColor.g))/2*range;
        //fadecolor.b=((2*range-(i))*blastcolor.b + ((i)*currentColor.b))/2*range;
      //}
      //else {
        //fadecolor.r=((range-(i-range+j))*blastcolor.r + ((i-range-j)*currentColor.r))/range;
        //fadecolor.g=((range-(i-range+j))*blastcolor.g + ((i-range-j)*currentColor.g))/range;
        //fadecolor.b=((range-(i-range+j))*blastcolor.b + ((i-range-j)*currentColor.b))/range;        
      //}
      //fadecolor.r=((range-(i-abs(j-pixel)))*blastcolor.r + ((i+abs(j-pixel))*currentColor.r))/range;
      //fadecolor.g=((range-(i-abs(j-pixel)))*blastcolor.g + ((i+abs(j-pixel))*currentColor.g))/range;
      //fadecolor.b=((range-(i-abs(j-pixel)))*blastcolor.b + ((i+abs(j-pixel))*currentColor.b))/range;
      //fadecolor.r=((range-i)*blastcolor.r + (i*currentColor.r))/range;
      //fadecolor.g=((range-i)*blastcolor.g + (i*currentColor.g))/range;
      //fadecolor.b=((range-i)*blastcolor.b + (i*currentColor.b))/range;
      //pixels.set_crgb_at(j, fadecolor);
      
      if (j==i) {
      //if ((j==pixel-i) or (j==pixel+i)) {
        pixels.set_crgb_at(pixel-j, blastcolor);
        pixels.set_crgb_at(pixel+j, blastcolor);
      }
      else {
      //else if ((j<pixel-i) or (j>pixel+i)){
  		  pixels.set_crgb_at(pixel-j, currentColor);
        pixels.set_crgb_at(pixel+j, currentColor);
      }
  	}
  	pixels.sync();
    delay(BLASTER_FX_DURATION/(2*range));  // blast deflect should last for ~500ms
  }
}

void lightFlicker(uint8_t value,uint8_t AState) {
	uint8_t variation = abs(analogRead(SPK1) - analogRead(SPK2));
	uint8_t brightness;
#ifdef FIREBLADE

  if (AState==AS_BLADELOCKUP) {
    Fire_Cooling=150;
    Fire_Sparking=50;
  }
  else {
    Fire_Cooling=50;
    Fire_Sparking=100;  
  }
    FireBlade();
    pixels.sync(); // Sends the data to the LEDs
#else

//if (type == 0) {  //sound based flicker
//  if (not value) {
// Calculation of the amount of brightness to fade
//     brightness = constrain(MAX_BRIGHTNESS
//    - (abs(analogRead(SPK1) - analogRead(SPK2)))/8,0,255);
//  } else {
//    brightness = value;
//  }
//} else { //algorithm based flicker
  int flickFactor = random(0,255);
  if (flickFactor > 3 && flickFactor < 170) { flickFactor = 255; }
  brightness = flickFactor;
//}

#if defined LS_HEAVY_DEBUG
	Serial.print(F("Brightness: "));
	Serial.print(brightness);
	Serial.print(F("   SPK1: "));
	Serial.print(analogRead(SPK1));
	Serial.print(F("   SPK2: "));
	Serial.println(analogRead(SPK2));
#endif

//	switch (type) {
//	case 0:
	// std Flickering
  cRGB color;
  if (AState==AS_BLADELOCKUP) { //animate blade in lockup mode
    int lockupFlick = random(0,39); 
    if (lockupFlick < 10) {
      color.b = brightness * currentColor.r / rgbFactor;
      color.g = brightness * currentColor.g / rgbFactor;
      color.r = brightness * currentColor.b / rgbFactor;
    } else if (lockupFlick < 20) {
      color.r = brightness * currentColor.r / rgbFactor;
      color.b = brightness * currentColor.g / rgbFactor;
      color.g = brightness * currentColor.b / rgbFactor;
    } else if (lockupFlick < 30) {
      color.b = brightness * currentColor.r / rgbFactor;
      color.r = brightness * currentColor.g / rgbFactor;
      color.g = brightness * currentColor.b / rgbFactor;
    } else {
      color.r = brightness * currentColor.r / rgbFactor;
      color.g = brightness * currentColor.g / rgbFactor;
      color.b = brightness * currentColor.b / rgbFactor;
    }
  } else {  //normal operation
    color.r = brightness * currentColor.r / rgbFactor;
    color.g = brightness * currentColor.g / rgbFactor;
    color.b = brightness * currentColor.b / rgbFactor;
  }

  for (uint16_t i = 0; i <= NUMPIXELS; i++) {
      pixels.set_crgb_at(i, color); 
  }
	pixels.sync();

//		break;
	/*
	 case 1:
	 // pulse Flickering
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
	 // anarchic Flickering
	 for (uint8_t i = 0; i <= 5; i++) {
	 randomSeed(analogRead(ledPins[i]));
	 analogWrite(ledPins[i],
	 MAX_BRIGHTNESS - random(variation, variation * 2));
	 }
	 break;
	 */
//	}
#endif
} //lightFlicker

/*
 * Colors are defined in percentage of brightness.
 *
 */
#ifdef COLORS
void getColor(uint8_t color) {
#else
void getColor(cRGB color) {
#endif

#ifdef COLORS
  switch (color) {
  case 0:
//Red
    currentColor.r = 100;
    currentColor.g = 0;
    currentColor.b = 0;
    break;
  case 1:
//Yellow
    currentColor.r = 100;
    currentColor.g = 100;
    currentColor.b = 0;
    break;
  case 2:
//Green
    currentColor.r = 0;
    currentColor.g = 100;
    currentColor.b = 0;
    break;
  case 3:
//Aqua
    currentColor.r = 0;
    currentColor.g = 100;
    currentColor.b = 100;
    break;
  case 4:
//Blue
    currentColor.r = 0;
    currentColor.g = 0;
    currentColor.b = 100;
    break;
  case 5:
//Fuschia
    currentColor.r = 100;
    currentColor.g = 0;
    currentColor.b = 100;
    break;

  case 6:
//DarkGrey
    currentColor.r = 100;
    currentColor.g = 100;
    currentColor.b = 100;
    break;
  case 7:
//DarkOrange
    currentColor.r = 1000;
    currentColor.g = 76;
    currentColor.b = 0;
    break;
  case 8:
//DarkViolet
    currentColor.r = 100;
    currentColor.g = 0;
    currentColor.b = 100;
    break;
  case 9:
//DodgerBlue
    currentColor.r = 24;
    currentColor.g = 80;
    currentColor.b = 100;
    break;
  case 10:
//Gold
    currentColor.r = 100;
    currentColor.g = 120;
    currentColor.b = 0;
    break;
  case 11:
//GoldenRod
    currentColor.r = 100;
    currentColor.g = 112;
    currentColor.b = 24;
    break;
  case 12:
//Indigo
    currentColor.r = 80;
    currentColor.g = 0;
    currentColor.b = 100;
    break;
  case 13:
//LightGreen
    currentColor.r = 90;
    currentColor.g = 100;
    currentColor.b = 90;
    break;

  default:
// White (if enough voltage)
    currentColor.r = 50;
    currentColor.g = 50;
    currentColor.b = 50;
    break;
  }
#else
    currentColor.r = color.r;
    currentColor.g = color.g;
    currentColor.b = color.b;
#endif
} //getColor


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

// neopixel ramp code from jbkuma
void RampPixels(uint16_t RampDuration, bool DirectionUpDown) {
  unsigned long ignitionStart = millis();  //record start of ramp function
  cRGB value;
#ifdef FIREBLADE
  for (unsigned int i=0; i<NUMPIXELS; (i=i+5)) { // turn on/off one LED at a time
     FireBlade();
     for(unsigned int j=0; j<NUMPIXELS; j++ ) { // fill up string with data
      if ((DirectionUpDown and j<=i) or (!DirectionUpDown and j<=NUMPIXELS-1-i)){
        }
        else if ((DirectionUpDown and j>i) or (!DirectionUpDown and j>NUMPIXELS-1-i)){
          value.r=0;
          value.g=0;
          value.b=0;  
          //heat[j]=0;
          pixels.set_crgb_at(j, value); // Set value at LED found at index j
        }      
      }
      pixels.sync(); // Sends the data to the LEDs
    }    
#else
  for (unsigned int i = 0; i < NUMPIXELS; i = NUMPIXELS*(millis()-ignitionStart)/RampDuration) { // turn on/off the number of LEDs that match rap timing
      //generate a flicker effect between 65% and 115% of MAX_BRIGHTNESS, with a 1 in 115 chance of flicking to 0
      int flickFactor = random(0,115);
      if (flickFactor < 65 && flickFactor > 0) { flickFactor = 100; } 
     for(uint8_t  j=0; j<NUMPIXELS; j++ ) { // fill up string with data
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
     delay(RampDuration/NUMPIXELS); //match the ramp duration to the number of pixels in the string
  }
#endif
}

#ifdef FIREBLADE
void FireBlade() {
// Array of temperature readings at each simulation cell
  int pixelnumber;
  
  // Step 1.  Cool down every cell a little
#ifdef CROSSGUARDSABER
    for( int i = 0; i < MN_STRIPE; i++) {
      heat[i] = constrain(heat[i] - random(((Fire_Cooling * 10) / MN_STRIPE) + 2),0,255);
    }
    for( int i = 0; i < CG_STRIPE; i++) {
      heat_cg[i] = constrain(heat_cg[i] - random(5),0,255);
    }
#else
    for( int i = 0; i < NUMPIXELS; i++) {
      // the random() function in this loop causes phantom swings
      heat[i] = constrain(heat[i] - random(((Fire_Cooling * 10) / NUMPIXELS) + 2),0,255);
      //delayMicroseconds(1000);    
    }
#endif

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
#ifdef CROSSGUARDSABER
    for( int k= MN_STRIPE - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    for( int k= CG_STRIPE - 1; k >= 2; k--) {
      heat_cg[k] = (heat_cg[k - 1] + heat_cg[k - 2] + heat_cg[k - 2] ) / 3;
    }
#else
    for( int k= NUMPIXELS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
#endif
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
#ifdef CROSSGUARDSABER
    if( random(255) < Fire_Sparking ) {
      int y = random(7);
      heat[y] = constrain(heat[y] + random(95)+160,0,255 );
    }
    if( random(255) < 10 ) {
      int y = random(4);
      heat_cg[y] = constrain(heat_cg[0] + random(95)+160,0,255 );  
    } 
#else
    if( random(255) < Fire_Sparking ) {
      int y = random(7);
      heat[y] = constrain(heat[y] + random(95)+160,0,255 );
    }
#endif

    // Step 4.  Map from heat cells to LED colors 
#ifdef CROSSGUARDSABER
    for( int j = 0; j < CG_STRIPE; j++) {
      cRGB color = HeatColor( heat_cg[j]);
      //if( gReverseDirection ) {
      //  pixelnumber = (CG_STRIPE-1) - j;
      //} else {
      //  pixelnumber = j;
      //}
      LED.set_crgb_at(j, color); // Set value at LED found at index j
    }
    for( int j = CG_STRIPE; j < CG_STRIPE + MN_STRIPE; j++) {
      cRGB color = HeatColor( heat[j]);
      //if( gReverseDirection ) {
      //  pixelnumber = (CG_STRIPE + MN_STRIPE-1) - j;
      //} else {
      //  pixelnumber = j;
      //}
      pixels.set_crgb_at(j, color); // Set value at LED found at index j
    }
#else
    for( int j = 0; j < NUMPIXELS; j++) {
      cRGB color = HeatColor( heat[j]);
      int pixelnumber;
      //if( gReverseDirection ) {
      //  pixelnumber = (NUMPIXELS-1) - j;
      //} else {
      //  pixelnumber = j;
      //}
      pixels.set_crgb_at(j, color); // Set value at LED found at index j
    }
#endif
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

cRGB HeatColor( uint8_t temperature)
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
        // we're in the hottest third
        heatcolor.r = 255; // full red
        heatcolor.g = 255; // full green
        heatcolor.b = heatramp; // ramp up blue

    } else if( t192 & 0x40 ) {
        // we're in the middle third
        heatcolor.r = 255; // full red
        heatcolor.g = heatramp; // ramp up green
        heatcolor.b = 0; // no blue

    } else {
        // we're in the coolest third
        heatcolor.r = heatramp; // ramp up red
        heatcolor.g = 0; // no green
        heatcolor.b = 0; // no blue
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

#endif

#else  // blade type

#endif  // blade type

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


void BladeMeter (int meterLevel) {  //expects input of 0-100
  //normalize data if to max and min if out of range
  if (meterLevel <= 0) { meterLevel = 0; } 
  if (meterLevel >= 100) { meterLevel = 100; }

#ifdef LEDSTRINGS // light blade as 6 segment bar graph
  int ledPins[] = {LS1, LS2, LS3, LS4, LS5, LS6};
  meterLevel = meterLevel*6/100;
    for (uint8_t i = 0; i < 6; i++) {
      if (i <= meterLevel) {
        digitalWrite(ledPins[i], HIGH);
      } else {
        digitalWrite(ledPins[i], LOW);
      }
    }
#endif

#ifdef STAR_LED // light led in gradient from red to green
  analogWrite(LED_RED, MAX_BRIGHTNESS * (100 - meterLevel) * 255 / 100 / rgbFactor);
  analogWrite(LED_GREEN, MAX_BRIGHTNESS * meterLevel * 255 / 100 / rgbFactor);
  analogWrite(LED_BLUE, 0);
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

