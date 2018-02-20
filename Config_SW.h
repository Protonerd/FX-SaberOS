/*
 * Config_SW.h
 *
 * Created on: 30 March 2017
 * author:    Andras Kun (kun.andras@yahoo.de)
 * Source :  https://github.com/Protonerd/FX-SaberOS
 * This tab contains all settings necessary to define your saber board's electronic software configuration
 */

#include "Config_HW.h"

#if not defined CONFIG_SW_H_
#define CONFIG_SW_H_


/*
 * DEFAULT CONFIG PARAMETERS
 * Will be overriden by EEPROM settings
 * once the first save will be done
 *************************************/
// memory base in EEPROM for configuration variables, i.e. the config block will start at EEPROM address 32
#define MEMORYBASE       32
#define VOL          20
#define SOUNDFONT       3
#define SWING         1000
#define CLASH_THRESHOLD 10 // 10 was the original value in LSOS, Jason's value changed it to 6, but it resulted in false clash trigges at more forceful swings
/************************************/

#if defined LEDSTRINGS
  #define CONFIG_VERSION     "L01"
#endif
#if defined STAR_LED
  #define CONFIG_VERSION     "L02"
  static const uint8_t rgbFactor = 255;
#endif
#if defined PIXELBLADE
  #define CONFIG_VERSION     "L03"
  static const uint8_t rgbFactor = 255;
#endif
#if defined ADF_PIXIE_BLADE
  #define CONFIG_VERSION     "L05"
  static const uint8_t rgbFactor = 255;
#endif

/*
 * SOFTWARE SWITCHES FOR DFPLAYER CHISET
 *************************************/

#ifdef ADF_PIXIE_BLADE
  #define OLD_DPFPLAYER_LIB
  #include "Adafruit_Pixie.h"
  #include <SoftwareSerial.h>
#endif
/**********END of blade type hardware settings **************************/

/*
 * FX settings
 * With the following compile directives you can fine tune your saber effects like clash, swing, blaster deflect etc.
 */

/*
 * CLASH DETECTION METHOD
 * Clash can be detected by programming the mpu to trigger an interupt
 * on motion. If a motion interrupt is triggered, it can either be
 * detected by connecting the MPU's INT signal to D2 of the AVR (-> CLASH_DET_MPU_INT)
 * or by polling the INT_STATUS register via I2C and determining the value (-> CLASH_DET_MPU_POLL)
 * of bit 6
 */
#define CLASH_DET_MPU_INT
//#define CLASH_DET_MPU_POLL
 
/* FX DURATIONS AND SUPRESS TIMES
 *  effects cannot be retriggered for the duration
 *  of their respective suppress pareameters

 */
#define SWING_SUPPRESS     300
#define CLASH_SUPRESS     200  // do not modify below 400, otherwise interlocking clash sounds can occur
#define BLASTERBLOCK_SUPRESS     200

/*
 *  HUM_RELAUNCH will tell the state machine to relaunch
 *  hum sound after this time period elapses
 */
#define HUM_RELAUNCH     5000

/*
 * How long do the light effect last for the different FX's, choose to synchronize with the sound effect
 */
#define CLASH_FX_DURATION 400
#define BLASTER_FX_DURATION 150
#define SWING_FX_DURATION 400
// select if swing shall be triggered by change in blade orientation and rotation; otherwise swing is simply determined by blade acceleration
//#define SWING_QUATERNION

/*
 * BUTTONS PARAMETERS
 ************************************/
#define CLICK        200 //5    // ms you need to press a button to be a click
#define PRESS_ACTION    600 //200  // ms you need to press a button to be a long press, in action mode
#define PRESS_CONFIG    600 //400  // ms you need to press a button to be a long press, in config mode
/************************************/


/********** END of FX Settings **************/

/* MAX_BRIGHTNESS
 *
 * Maximum output voltage to apply to LEDS
 * Default = 100 (39,2%) Max=255 Min=0(Off)
 *
 * WARNING ! A too high value may burn
 * your leds. Please make your maths !
 * BE VERY CAREFULL WITH THIS ONE OR 
 * YOU'LL BURN YOUR BLADE'S LED 
 ************************************/
#define MAX_BRIGHTNESS    230


/* DEEP_SLEEP
 * If you want to enable/disable
 * deep sleep capabalities
 * Starting from v1.6, deep sleep can be entered from the config menu after the volume setting
 * If you a device with a CPU wich is not
 * an Atmega328 : COMMENT THIS
 *************************************/
//#define DEEP_SLEEP
#if defined DEEP_SLEEP
  #define SLEEPYTIME      60000 //20 secs, after which the board will automatically go to sleep mode (to be implemented)
  #include <avr/sleep.h>
  #include <avr/power.h>
#endif  // DEEP_SLEEP


/* COLOR CHANGE OPTIONS
 *  If you want to use the advanced Gravity Color method developed for FX-SaberOS
 *  leave the GRAVITY_COLOR compile directive defined.
 *  If instead you would like to work with pre-set color profiles (15 different, pre-defined colors)
 *  comment out the line with GRAVITY_COLOR, which will define COLOR_PROFILE to be used.
 *************************************/
//#define GRAVITY_COLOR
#ifndef GRAVITY_COLOR
  #define COLOR_PROFILE
#endif

/*
 * Enable Jukebox, an integrated MP3/WAV player which can play songs/music files
 * stored in the jukebox folder on your storage media (SD-card or SPI flash).
 * Define the number of songs you have using the NR_JUKEBOXSONGS compile statement in Soundfont.h
 */
//#define JUKEBOX


/***** Battery meter settings *****/
const long InternalReferenceVoltage = 1062;  // Adjust this value to your board's specific internal BG voltage
#define BATTERY_FACTOR 5 // Callibration value to compensate for component variation
#define LOW_BATTERY 3.3 // low voltage for battery, a 5v Arduino or DIYino requires 3.3v 
#define FULL_BATTERY 4.15 // full voltage for battery, nominally 4.2 for a 3.7v battery
//cRGB C1;  //Low meter color
//cRGB C2;  //middle meter color
//cRGB C3;  //high meter color

/*
 * Accent LED can be used to illuminate a button with an in-built LED
 * The pin definition for the button LED can be found in hardware config tab
 */
/*
 * Soft or Had PWM for Accent
 */
//#define SOFT_ACCENT
#if not defined SOFT_ACCENT
  #define HARD_ACCENT
#endif
/*
 * DEBUG PARAMETERS
 */
/* LS_INFO
 * For daily use I recommend you comment LS_INFO
 * When you plug your device to USB uncomment LS_INFO !
 */
#define LS_SERIAL  //enable serial communication using Wire library
#if defined LS_SERIAL
//#define LS_FSM
#define LS_INFO
//#define LS_DEBUG
#endif

#if defined LS_DEBUG
//#define LS_BUTTON_DEBUG
//#define LS_MOTION_DEBUG
//#define LS_MOTION_HEAVY_DEBUG
//#define LS_RELAUNCH_DEBUG
//#define LS_DEBUG_SLEEP
#endif

#if defined LS_MOTION_DEBUG
//#define LS_SWING_DEBUG
//#define LS_SWING_HEAVY_DEBUG
//#define LS_CLASH_DEBUG
//#define LS_CLASH_HEAVY_DEBUG
#endif




















#endif /* CONFIG_SW_H_ */

