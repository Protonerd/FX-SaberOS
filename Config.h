/*
 * Config.h
 *
 * Created April 1, 2017
 * author:    Andras Kun (kun.andras@yahoo.de), Jason Brinkerhoff (jb@jbkuma.com)
 * source:    https://github.com/Protonerd/FX-SaberOS
 * 
 * based on LightSaber OS originally created by Sebastien CAPOU (neskweek@gmail.com)
 * 
 * !!! IMPORTANT !!! IMPORTANT !!! IMPORTANT !!! IMPORTANT !!!
 * PLEASE READ AND CONFIGURE ALL OF THE SETTINGS PRIOR TO INSTALLING
 */

#if not defined CONFIG_H_
#define CONFIG_H_

/*!!!!!IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT!!!
 *
 * MPU6050 device ORIENTATION
 * Choose which MPU's axis is parallel
 * to your blade axis
 *************************************/
//#define BLADE_X
#define BLADE_Y
//#define BLADE_Z
/************************************/

/*
 * MPU6050 calibrated offset values
 * If defined, calibration values will be retrieved from EEPROM
 * use this option if the MPU6050_calibration sketch wrote the calibrated offsets
 * into the EEPROM (default address is 200)
 * If not defined, you have to note down the calibrated offset values
 * and assign them to the respective variables in the code.
 *************************************/
#define MPUCALOFFSETEEPROM
#ifdef MPUCALOFFSETEEPROM
#define MEMORYBASEMPUCALIBOFFSET 200 // 96 -> increased due to storing RGB PWM values in EEPROM takes a lot of space (3 x 1byte x 3 color types x nr of sound fonts i.e. for 5 sound fonts 45 bytes
#endif
/************************************/

// ==============================================================================
// ===                          BLADE TYPE SETTINGS                           ===
// ==============================================================================
/*****************************************
 * Define only the blade type to be used *
 *****************************************/
//#define LEDSTRINGS
//#define STAR_LED
#define PIXELBLADE

/************************************/
/* SABER TYPE
 * currently in v1.3 only the CROSSGUARDSABER
 * will have any effect on the code
 * due to the fire blade effect
 *************************************/
#define SINGLEBLADE  // i.e. Graflex
//#define SABERSTAFF  // i.e. Darth Maul saber with dual blades
//#define CROSSGUARDSABER  // i.e. Kylo Ren saber

/* DEFAULT CONFIG PARAMETERS
 * Will be overriden by EEPROM settings
 * once the first initialization is completed
 *************************************/
#define VOL               20    //0-30
#define SOUNDFONT         3
#define SWING             1000 //default 1000
#define CLASH_THRESHOLD   10 //default 10

/* MAX_BRIGHTNESS
 *
 * Maximum output of STAR_LED and PIXELBLADE
 * Default = 100 (39,2%) Max=255 Min=0(Off)
 *
 * WARNING ! A too high value may burn
 * your leds. Please make your maths !
 * BE VERY CAREFULL WITH THIS ONE OR 
 * YOU'LL BURN YOUR BLADE'S LED 
 ************************************/
#define MAX_BRIGHTNESS        200

/* DO NOT MODIFY
 * Unless you know what you're doing
 *************************************/
#if defined LEDSTRINGS
#define CONFIG_VERSION     "L01"
#endif
#if defined STAR_LED
#define CONFIG_VERSION     "L02"
#endif
#if defined PIXELBLADE
#define CONFIG_VERSION     "L03"
#endif
#define MEMORYBASE          32
/************************************/

// ==============================================================================
// ===                         BUTTON CONFIGURATION                           ===
// ==============================================================================

/* Set for single button or comment two button configuration */
//#define SINGLEBUTTON

// ==============================================================================
// ===                         PIXEL BLADE SETTINGS                           ===
// ==============================================================================

#if defined PIXELBLADE

// How many leds in one strip?
#define NUMPIXELS 120  // can go up to 120, could lead to memory problems if further increased

#ifdef CROSSGUARDSABER
// define how many pixels are used for the crossguard and how many for the main blade
#define CG_STRIPE 10
#define MN_STRIPE 50
#endif

//#define FIREBLADE 

// Number of color defined
//#define COLORS 14
static const uint8_t rgbFactor = 255;

#endif

// ==============================================================================
// ===                          STAR LED SETTINGS                             ===
// ==============================================================================

#if defined STAR_LED

static const uint8_t rgbFactor = 100;

/* COLORS
 * Number of colors to chose from
 * Range : 6<->600
 * Default: 48
 */
//#define COLORS		 		14
#endif  // BLADE TYPE

// ==============================================================================
// ===                           EFFECT SETTINGS                              ===
// ==============================================================================

// How long do the light effect last for the different FX's
#define CLASH_FX_DURATION     200
#define BLASTER_FX_DURATION   150
#define SWING_FX_DURATION     400


#define BLASTER_FLASH_TIME    3
#define CLASH_FLASH_TIME  	  1

/* FX DURATIONS AND SUPRESS TIMES
 *  effects cannot be retriggered for the duration
 *  of their respective supress pareameters
 *  HUM_RELAUNCH will tell the state machine to relaunch
 *  hum sound after this time period elapses
 */
#define SWING_SUPPRESS          500
#define CLASH_SUPRESS           400  // do not modify below 400, otherwise interlocking clash sounds can occur
#define BLASTERBLOCK_SUPRESS    100
#define HUM_RELAUNCH            5000

/* BLASTER DEFLECT TYPE
 * Define how a blaser bolt deflect is
 * to be triggered
 * Blaster deflect action is started with
 * a single click on the lockup button.
 * if BLASTERCLICKTRIGGER is defined, a blaster deflect
 * will be triggered once on click.
 * if BLATSTERMOVEMENTTRIGGER is defined,
 * blaser deflect is triggered by ensuing swings/movements.
 *************************************/
#define BLASTERCLICKTRIGGER
#ifndef BLASTERCLICKTRIGGER
#define BLATSTERMOVEMENTTRIGGER
#endif


// ==============================================================================
// ===                           DEFINE BOARD TYPE                            ===
// ==============================================================================

#define DIYINO_PRIME
//#define DIYINO_STARTDUST
//#define ARDUINO_HOMEBREW

// ==============================================================================
// ===                            SLEEP SETTINGS                              ===
// ==============================================================================

/* DEEP_SLEEP
 * If you want to enable/disable
 * deep sleep capabalities
 * If you a device with a CPU wich is not
 * an Atmega328 : COMMENT THIS
 *************************************/
#define DEEP_SLEEP
#if defined DEEP_SLEEP
#define SLEEP_TIMER		360000	// sleep after 5 minutes standby, comment for no automatic sleep

#endif  // DEEP_SLEEP

// ==============================================================================
// ===                             PIN DEFINITIONS                            ===
// ==============================================================================

#if defined LEDSTRINGS
  #define LS1 			3 //3
  #define LS2 			5 //5
  #define LS3 			6  //6
  #define LS4 			9  //9
  #define LS5 			10  //10
  #define LS6 			11 //11
  /*   CLASH STRING
   * Enable/disable management of
   * single clash ledstring
   *************************************/
  //#define CLASHSTRING			13
#endif

#ifdef PIXELBLADE
  #define DATA_PIN       13 // D13
  #ifdef DIYINO_PRIME
    #define LS1       3
    #define LS2       5
    #define LS3       6
    #define LS4       9
    #define LS5       10
    #define LS6       11
  #else if DIYINO_STARDUST
    #define LS1       5
    #define LS2       6
    #define LS3       9
  #endif
#endif

#if defined STAR_LED
  #ifdef DIYINO_PRIME 
    #define LED_RED       3
    #define LED_GREEN     5
    #define LED_BLUE      6
  #else if DIYINO_STARDUST
    #define LED_RED 			5
    #define LED_GREEN 	  6
    #define LED_BLUE 			9
  #endif
#endif


#ifdef DIYINO_PRIME 
  #define MP3_PSWITCH   15
  #define FTDI_PSWITCH  16
#else if DIYINO_STARDUST 
  #define MP3_PSWITCH   16
  #define FTDI_PSWITCH  4
#endif


#define DFPLAYER_RX     8
#define DFPLAYER_TX     7
#define SPK1            20 //A6
#define SPK2            21 //A7


#ifdef DIYINO_PRIME 
  #define MAIN_BUTTON     4
  #define LOCKUP_BUTTON   12
#else if DIYINO_STARDUST
  #define MAIN_BUTTON     12
  #define LOCKUP_BUTTON   2
#endif

/*
 * ACCENT_LED
 * Enable/disable management of
 * a button accent led
 *
 * If you define ACCENT_LED beware on which
 * kind of pin you defined it :
 * D3,D5,D6,D9,D10,D11 are hardware PWM
 * others must use software PWM
 *
 * RGB LEDS user should choose tu plu their
 * Accent leds on Hardware PWM pin
 *
 * LEDSTRINGS users have no choice :
 * your forced to use Software Accent LED
 *************************************/
#define ACCENT_LED      14 //A0
#if defined ACCENT_LED
/*
 * Soft or Had PWM for Accent
 */
//#define SOFT_ACCENT //this doesn't work
#if not defined SOFT_ACCENT
#define HARD_ACCENT
#endif
#endif //ACCENT_LED

// ==============================================================================
// ===                      BATTERY CHECK CONFIGURATION                       ===
// ==============================================================================
#define BATTERY_CHECK           // comment to disable
#define BATTERY_FACTOR    6.34  // Callibration value to compensate for component variation
#define BATTERY_READPIN   17    //A3 - read battery level 100kohm from GND, 470kohm from Bat+
#define LOW_BATTERY       3.3   // low voltage for battery, a 5v Arduino or DIYino requires 3.3v 
#define FULL_BATTERY      4.10  // full voltage for battery, nominally 4.2 for a 3.7v battery

// ==============================================================================
// ===                        CONFIG MENU PARAMETERS                          ===
// ==============================================================================
#define JUKEBOX
#if defined STAR_LED
#define CONFIG_BLADE_MAIN_COLOR
#define CONFIG_BLADE_CLASH_COLOR
#endif

#if defined PIXELBLADES
#define CONFIG_BLADE_MAIN_COLOR
#define CONFIG_BLADE_CLASH_COLOR
#define CONFIG_POWERON_EFFECT
#define CONFIG_POWEROFF_EFFECT
#define CONFIG_FLICKER_EFFECT
#endif

#if defined LEDSTRINGS
#define CONFIG_POWERON_EFFECT
#define CONFIG_POWEROFF_EFFECT
#define CONFIG_FLICKER_EFFECT
#endif

// ==============================================================================
// ===                            DEBUG PARAMETERS                            ===
// ==============================================================================
/* LS_INFO
 * For daily use I recommend you comment LS_INFO
 * When you plug your device to USB uncomment LS_INFO !
 */
#define LS_SERIAL  //enable serial communication using Wire library
#if defined LS_SERIAL
//#define LS_FSM
//#define LS_INFO
//#define LS_DEBUG
#endif

#if defined LS_DEBUG
#define LS_BUTTON_DEBUG
//#define LS_MOTION_DEBUG
//#define LS_MOTION_HEAVY_DEBUG
//#define LS_RELAUNCH_DEBUG
//#define LS_DEBUG_SLEEP
#endif

#if defined LS_MOTION_DEBUG
#define LS_SWING_DEBUG
#define LS_SWING_HEAVY_DEBUG
//#define LS_CLASH_DEBUG
//#define LS_CLASH_HEAVY_DEBUG
#endif

#endif /* CONFIG_H_ */
