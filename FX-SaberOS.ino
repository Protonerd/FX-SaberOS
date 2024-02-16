/*
   FX-SaberOS V1.0

   released on: 7 October 2017
   author: 		Sebastien CAPOU (neskweek@gmail.com) and Andras Kun (kun.andras@yahoo.de)
   Source : 	https://github.com/Protonerd/FX-SaberOS
   Description:	Operating System for Arduino based LightSaber

   This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
*/
/***************************************************************************************************/
#include <Arduino.h>
#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include <EEPROMex.h>
#include <OneButton.h>
#include <LinkedList.h>
#include <avr/wdt.h>

#include "Buttons.h"
#include "Config_HW.h"
#include "Config_SW.h"
#include "ConfigMenu.h"
#include "Light.h"
#include "Soundfont.h"
/*
* DFPLAYER variables
*/

#include <DFPlayerSerial.h>
#include <DFPlayer.h>
DFPlayer dfplayer;

SoundFont soundFont;
unsigned long sndSuppress = millis();
unsigned long sndSuppress2 = millis();
unsigned long clashSndSuppress = millis();
#ifdef HUM_MODULATION
  unsigned long hmStart;
  unsigned long hmEnd = millis();
#endif
#ifdef LS_LOOPLENGHT
  unsigned long loopcurrenttime;
#endif
#ifdef DEEP_SLEEP
  unsigned long sleepTimer = millis();
#endif
bool hum_playing = false; // variable to store whether hum is being played
#ifdef JUKEBOX
bool jukebox_play = false; // indicate whether a song is being played in JukeBox mode
uint8_t jb_track;  // sound file track number in the directory designated for music playback
#endif
#ifdef CROSSGUARDSABER
bool mainignition_done=false;
#endif

/***************************************************************************************************
 * Saber Finite State Machine Custom Type and State Variable
 */
// global Saber state and Sub State variables
extern SaberStateEnum SaberState;
extern SaberStateEnum PrevSaberState;
extern ActionModeSubStatesEnum ActionModeSubStates;
extern ConfigModeSubStatesEnum ConfigModeSubStates;
extern ActionModeSubStatesEnum PrevActionModeSubStates;
extern ConfigModeSubStatesEnum PrevConfigModeSubStates;
//extern SubStateEnum SubState;
/***************************************************************************************************
 * Motion detection Variables
*/
MPU6050 mpu;
#ifdef CLASH_DET_MPU_INT
  I2Cdev i2ccomm;
#endif
// MPU control/status vars
volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus; // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint8_t fifoBuffer[64]; // FIFO storage buffer
uint16_t mpuFifoCount;     // count of all bytes currently in FIFO
// orientation/motion vars
Quaternion curRotation;           // [w, x, y, z]         quaternion container
Quaternion prevRotation;           // [w, x, y, z]         quaternion container
static Quaternion prevOrientation;  // [w, x, y, z]         quaternion container
static Quaternion curOrientation; // [w, x, y, z]         quaternion container
VectorInt16 curAccel;
VectorInt16 prevAccel;
VectorInt16 curDeltAccel;
VectorInt16 prevDeltAccel;

/***************************************************************************************************
 * LED String variables
 */
  cRGB currentColor;
#if defined LEDSTRINGS
  #ifdef DIYINO_PRIME
    uint8_t ledPins[] = {LS1, LS2, LS3, LS4, LS5, LS6};
  #else if defined DIYINO_STARDUST_V2 or defined DIYINO_STARDUST_V3
    uint8_t ledPins[] = {LS1, LS2, LS3};
  #endif
  uint8_t blasterPin;
#endif
#if defined STAR_LED
  uint8_t ledPins[] = {LED_RED, LED_GREEN, LED_BLUE};
#endif
extern bool fireblade;
#if defined PIXELBLADE or defined ADF_PIXIE_BLADE
  #ifdef DIYINO_PRIME
    uint8_t ledPins[] = {LS1, LS2, LS3, LS4, LS5, LS6};
  #else if defined DIYINO_STARDUST_V2 or defined DIYINO_STARDUST_V3
    uint8_t ledPins[] = {LS1, LS2, LS3};
  #endif
  WS2812 pixels(NUMPIXELS);

  cRGB color;
  uint8_t blasterPixel;
#endif
#ifdef PIXEL_ACCENT
  WS2812 accentPixels(NUM_ACCENT_PIXELS);
#endif
uint8_t clash = 0;
bool lockuponclash = false;
uint8_t randomBlink = 0;
/***************************************************************************************************
 * Buttons variables
 */
OneButton mainButton(MAIN_BUTTON, true);
#ifndef SINGLEBUTTON
OneButton lockupButton(AUX_BUTTON, true);
#endif

/***************************************************************************************************
 * ConfigMode Variables
 */
int8_t modification = 0;
int8_t prev_modification = 0;
int16_t value = 0;
uint8_t menu = 0;
bool enterMenu = false;
bool changeMenu = false;
bool play = false;
unsigned int configAdress = 0;
volatile uint8_t portbhistory = 0xFF;     // default is high because the pull-up

struct StoreStruct {
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


/***************************************************************************************************
 * Function Prototypes
 * The following prototypes are not correctly generated by Arduino IDE 1.6.5-r5 or previous
 */
inline void printQuaternion(Quaternion quaternion, long multiplier);
inline void printAcceleration(VectorInt16 aaWorld);

// ====================================================================================
// ===        	       	   			SETUP ROUTINE  	 	                			===
// ====================================================================================
void setup() {
  #ifdef PIXEL_ACCENT
    accentPixels.setOutput(PIXEL_ACCENT_DATA);
    #ifdef ACCENT_SWAP_RG
      accentPixels.setColorOrderRGB();
    #endif
  #endif

	// join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
	TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
			Fastwire::setup(400, true);
#endif
	// Serial line for debug
	Serial.begin(115200);
#ifdef INCLUDE_COMPILE_INFO
	const char compile_date[] = __DATE__ " " __TIME__;
	const char version_file[] = __FILE__;
	Serial.print(F("version file: "));Serial.println(version_file);
	Serial.print(F("compiled on: "));Serial.println(compile_date);Serial.println("");
#endif

	/***** LOAD CONFIG *****/
	// Get config from EEPROM if there is one
	// or initialise value with default ones set in StoreStruct
	EEPROM.setMemPool(MEMORYBASE, EEPROMSizeATmega328); //Set memorypool base to 32, assume Arduino Uno board
	configAdress = EEPROM.getAddress(sizeof(StoreStruct)); // Size of config object

Serial.print("size of StoreStruct: ");Serial.println(sizeof(StoreStruct));
Serial.println(configAdress);
 
	if (!loadConfig()) {
		for (uint8_t i = 0; i <= 2; i++) {
			storage.version[i] = CONFIG_VERSION[i];
		}
		  storage.soundFont = 0;
		  storage.volume = 15;
      for (uint8_t i=0; i<SOUNDFONT_QUANTITY;i++){
        storage.sndProfile[i].swingSensitivity=1000;
        storage.sndProfile[i].flickerType=0;
        storage.sndProfile[i].poweronoffType=0;      
      }
    for (uint8_t i=0; i<SOUNDFONT_QUANTITY;i++){
      storage.sndProfile[i].mainColor.r=MAX_BRIGHTNESS;
      storage.sndProfile[i].mainColor.g=0;
      storage.sndProfile[i].mainColor.b=0;
      storage.sndProfile[i].clashColor.r=0;
      storage.sndProfile[i].clashColor.g=MAX_BRIGHTNESS;
      storage.sndProfile[i].clashColor.b=0;
      storage.sndProfile[i].blasterboltColor.r=0;
      storage.sndProfile[i].blasterboltColor.g=0;
      storage.sndProfile[i].blasterboltColor.b=MAX_BRIGHTNESS;
    }
    saveConfig();
#if defined LS_INFO
    Serial.println(F("DEFAULT VALUE"));
#endif
  }
#if defined LS_INFO
  else {
    Serial.println(F("EEPROM LOADED"));
  }
#endif

// retreive the sound font ID stored in the EEPROM (last configured)
  soundFont.setID(storage.soundFont);
// in case a fireblade flicker type is selected for the active sound font, set the bool variable
// in case a fireblade flicker type is selected for the active sound font, set the bool variable
  if (CS_FLICKERTYPE < CS_LASTMEMBER and (storage.sndProfile[storage.soundFont].flickerType==2 or storage.sndProfile[storage.soundFont].flickerType==3 or storage.sndProfile[storage.soundFont].flickerType==4)) {
    fireblade=true;
   }
   else {
    fireblade=false;
   }
/* CONFIG ITEMS PRESETS */
/* Set default values to parameters which can be modified in config menu, if the corresponding config menu item is disabled */  
// if the config menu does not contain a menu item to define swing sensitivity, default it to 1000 (works very well, mid sensitivity)  
  if (CS_SWINGSENSITIVITY > CS_LASTMEMBER) {
    for (uint8_t i=0; i<SOUNDFONT_QUANTITY;i++){
      storage.sndProfile[i].swingSensitivity=SWING_THRESHOLD;      
    }
  }
  if (CS_VOLUME > CS_LASTMEMBER) {
    storage.volume=31;
  }   

  // enable watchdog to avoid system hang
  wdt_reset();
  wdt_enable(WDTO_8S);
  //WDTCSR = (1<<WDCE) | (1<<WDE) | (1<<WDP3) | (1<<WDP0);
  wdt_reset(); 
/***** LOAD CONFIG *****/

  /***** MP6050 MOTION DETECTOR INITIALISATION  *****/

  // initialize device
#if defined LS_INFO
  Serial.println(F("Initializing I2C devices..."));
#endif
  mpu.initialize();

  // verify connection
#if defined LS_INFO
  Serial.println(F("Testing device connections..."));
  Serial.println(
    mpu.testConnection() ?
    F("MPU6050 connection successful") :
    F("MPU6050 connection failed"));

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
#endif
  devStatus = mpu.dmpInitialize();

  /*
     Those offsets are specific to each MPU6050 device.
     they are found via calibration process.
     See this script http://www.i2cdevlib.com/forums/index.php?app=core&module=attach&section=attach&attach_id=27
  */
#ifdef MPUCALOFFSETEEPROM
  // retreive MPU6050 calibrated offset values from EEPROM
  EEPROM.setMemPool(MEMORYBASEMPUCALIBOFFSET, EEPROMSizeATmega328);
  int addressInt = MEMORYBASEMPUCALIBOFFSET;
  mpu.setXAccelOffset(EEPROM.readInt(addressInt));
#ifdef LS_INFO
  int16_t output;
  output = EEPROM.readInt(addressInt);
  Serial.print("address: "); Serial.println(addressInt); Serial.print("output: "); Serial.println(output); Serial.println("");
#endif
  addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
  mpu.setYAccelOffset(EEPROM.readInt(addressInt));
#ifdef LS_INFO
  output = EEPROM.readInt(addressInt);
  Serial.print("address: "); Serial.println(addressInt); Serial.print("output: "); Serial.println(output); Serial.println("");
#endif
  addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
  mpu.setZAccelOffset(EEPROM.readInt(addressInt));
#ifdef LS_INFO
  output = EEPROM.readInt(addressInt);
  Serial.print("address: "); Serial.println(addressInt); Serial.print("output: "); Serial.println(output); Serial.println("");
#endif
  addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
  mpu.setXGyroOffset(EEPROM.readInt(addressInt));
#ifdef LS_INFO
  output = EEPROM.readInt(addressInt);
  Serial.print("address: "); Serial.println(addressInt); Serial.print("output: "); Serial.println(output); Serial.println("");
#endif
  addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
  mpu.setYGyroOffset(EEPROM.readInt(addressInt));
#ifdef LS_INFO
  output = EEPROM.readInt(addressInt);
  Serial.print("address: "); Serial.println(addressInt); Serial.print("output: "); Serial.println(output); Serial.println("");
#endif
  addressInt = addressInt + 2; //EEPROM.getAddress(sizeof(int));
  mpu.setZGyroOffset(EEPROM.readInt(addressInt));
#ifdef LS_INFO
  output = EEPROM.readInt(addressInt);
  Serial.print("address: "); Serial.println(addressInt); Serial.print("output: "); Serial.println(output); Serial.println("");
#endif
#else // assign calibrated offset values here:
  /* UNIT1 */
  mpu.setXAccelOffset(46);
  mpu.setYAccelOffset(-4942);
  mpu.setZAccelOffset(4721);
  mpu.setXGyroOffset(23);
  mpu.setYGyroOffset(-11);
  mpu.setZGyroOffset(44);
#endif

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
#if defined LS_INFO
    Serial.println(F("Enabling DMP..."));
#endif
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
#if defined LS_INFO
    Serial.println(
      F(
        "Enabling interrupt detection (Arduino external interrupt 0)..."));
#endif
    //		attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
#if defined LS_INFO
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
#endif
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
#if defined LS_INFO
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
#endif
  }

  
  // configure the motion interrupt for clash recognition
  // INT_PIN_CFG register
  // in the working code of MPU6050_DMP all bits of the INT_PIN_CFG are false (0)
  mpu.setDLPFMode(3);
  mpu.setDHPFMode(0);
  //mpu.setFullScaleAccelRange(3);
  mpu.setIntMotionEnabled(true); // INT_ENABLE register enable interrupt source  motion detection
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntFIFOBufferOverflowEnabled(false);
  mpu.setIntI2CMasterEnabled(false);
  mpu.setIntDataReadyEnabled(false);
  //  mpu.setMotionDetectionThreshold(10); // 1mg/LSB
  mpu.setMotionDetectionThreshold(CLASH_THRESHOLD); // 1mg/LSB
  mpu.setMotionDetectionDuration(2); // number of consecutive samples above threshold to trigger int
  Serial.println("checkpoint 0");
  #ifdef CLASH_DET_MPU_INT
      // configure Interrupt with:
    // int level active low
    // int driver open drain
    // interrupt latched until read out (not 50us pulse)
    i2ccomm.writeByte(MPU6050_DEFAULT_ADDRESS, 0x37, 0xF0);
    // enable only Motion Interrut
    i2ccomm.writeByte(MPU6050_DEFAULT_ADDRESS, 0x38, 0x40);
  #endif
  mpuIntStatus = mpu.getIntStatus();


  #ifdef CLASH_DET_MPU_INT
    // define D2 (interrupt0) as input
    pinMode(2, INPUT_PULLUP);
    // enable Arduino interrupt detection
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    // define Interrupt Service for aux. switch
    attachInterrupt(0, ISR_MPUInterrupt, FALLING); // int.0 is the pin2 on the Atmega328P
  #endif
  /***** MP6050 MOTION DETECTOR INITIALISATION  *****/

  /***** LED SEGMENT INITIALISATION  *****/

  // initialize ledstrings segments
  DDRD |= B01101000;
  DDRB |= B00101110;

  //We shut off all pins that could wearing leds,just to be sure
  PORTD &= B10010111;
  PORTB &= B11010001;

#if defined STAR_LED
  //initialise start color
  getColor(storage.sndProfile[storage.soundFont].mainColor);
#endif

#if defined PIXELBLADE
  pixels.setOutput(DATA_PIN); // This initializes the NeoPixel library.
  /*pixelblade_KillKey_Disable();
  currentColor.r = 0;
  currentColor.g = 0;
  currentColor.b = 0;
  lightOn(ledPins, -1, currentColor);
  delay(300);
  lightOff();*/
  getColor(storage.sndProfile[storage.soundFont].mainColor);
  pixelblade_KillKey_Enable();
#endif

#if defined FoCSTRING
  pinMode(FoCSTRING, OUTPUT);
  FoCOff(FoCSTRING);
#endif

#ifdef HARD_ACCENT
  pinMode(ACCENT_LED, OUTPUT);
#endif

  //Randomize randomness (no really that's what it does)
  randomSeed(analogRead(2));

  /***** LED SEGMENT INITIALISATION  *****/

  /***** BUTTONS INITIALISATION  *****/

  // link the Main button functions.
  pinMode(MAIN_BUTTON, INPUT_PULLUP);
  mainButton.setClickMs(CLICK);
  mainButton.setPressMs(PRESS_CONFIG);
  mainButton.attachClick(mainClick);
  mainButton.attachDoubleClick(mainDoubleClick);
  mainButton.attachLongPressStart(mainLongPressStart);
  mainButton.attachLongPressStop(mainLongPressStop);
  mainButton.attachDuringLongPress(mainLongPress);

#ifndef SINGLEBUTTON
  // link the Lockup button functions.
  pinMode(AUX_BUTTON, INPUT_PULLUP);
  lockupButton.setClickMs(CLICK);
  lockupButton.setPressMs(PRESS_CONFIG);
  lockupButton.attachClick(lockupClick);
  lockupButton.attachDoubleClick(lockupDoubleClick);
  lockupButton.attachLongPressStart(lockupLongPressStart);
  lockupButton.attachLongPressStop(lockupLongPressStop);
  lockupButton.attachDuringLongPress(lockupLongPress);
#endif


#ifdef DEEP_SLEEP
  /************ DEEP_SLEEP MODE SETTINGS **********/
  pinMode(MP3_PSWITCH, OUTPUT);
  pinMode(FTDI_PSWITCH, OUTPUT);
  digitalWrite(MP3_PSWITCH, LOW); // enable MP3 player
  digitalWrite(FTDI_PSWITCH, LOW); // enable FTDI player
  // pin change interrupt masks (see below list)
  PCMSK2 |= bit (PCINT20);   // pin 4 Aux button
  PCMSK0 |= bit (PCINT4);    // pin 12 Main button
  delay(300);
#endif // DEEP_SLEEP

  /***** DF PLAYER INITIALISATION  *****/
  InitDFPlayer();
  delay(200);
  #ifdef DFPLAYER_CLONE
  delay(800); // clone chip requires more time to initialize
  #endif   
 // according to debug on 3.11.2017, these 2 lines below cause the sporadic disable of sound. For audio tracker they are not strictly needed.
  //pinMode(SPK1, INPUT);
  //pinMode(SPK2, INPUT);
  SinglePlay_Sound(soundFont.getBoot());//11);
  delay(20);
  if (storage.volume <= 15) {
    Set_Volume(15);
    delay(200);
  }
  
#ifdef ADF_PIXIE_BLADE
  InitAdafruitPixie(ledPins);
#endif
  SinglePlay_Sound(11);
  delay(850);


  /***** Quick Mute *****/
  if (digitalRead(MAIN_BUTTON) == LOW) {
    Set_Volume(0);
    //Serial.println("Muted");
  }
  else {
      Set_Volume(storage.volume);
      //Serial.println("Unmuted");
  }
  /****** INIT SABER STATE VARIABLE *****/
  SaberState = S_STANDBY;
  PrevSaberState = S_SLEEP;
  ActionModeSubStates = AS_HUM;
  #ifdef DEEP_SLEEP
    sleepTimer = millis();
  #endif

  Set_Volume(storage.volume);

} // end setup

// ====================================================================================
// ===               	   			LOOP ROUTINE  	 	                			===
// ====================================================================================
void loop() {

 // pat the dog
  wdt_reset(); // do not remove!!!
 
   // if MPU6050 DMP programming failed, don't try to do anything : EPIC FAIL !
  if (!dmpReady) {
    return;
  }

  mainButton.tick();
#ifndef SINGLEBUTTON
  lockupButton.tick();
#endif

#ifdef LS_LOOPLENGHT
  Serial.println(millis()-loopcurrenttime);
  loopcurrenttime=millis();
#endif
  /*//////////////////////////////////////////////////////////////////////////////////////////////////////////
     ACTION MODE HANDLER
  */ /////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (SaberState == S_SABERON) {
    /*
      // In case we want to time the loop
      Serial.print(F("Action Mode"));
      Serial.print(F(" time="));
      Serial.println(millis());
    */
    if (ActionModeSubStates != AS_HUM) { // needed for hum relauch only in case it's not already being played
      hum_playing = false;
    }
    else { // AS_HUM
      if ((millis() - sndSuppress > HUM_RELAUNCH and not hum_playing)) {
        HumRelaunch();
      }
    }

    if (ActionModeSubStates == AS_IGNITION) {
      /*
          This is the very first loop after Action Mode has been turned on
      */

#ifndef SINGLEBUTTON
      lockupButton.setPressTicks(PRESS_ACTION);
#endif
#if defined PIXELBLADE or defined ADF_PIXIE_BLADE
      pixelblade_KillKey_Disable();
#endif
#if defined LS_INFO
      Serial.println(F("START ACTION"));
#endif

      //Play powerons wavs
      SinglePlay_Sound(soundFont.getPowerOn((storage.soundFont)*NR_FILE_SF));
      // Light up the blade
      pixelblade_KillKey_Disable();
      #ifdef CROSSGUARDSABER
        lightIgnition(ledPins, soundFont.getPowerOnTime(), storage.sndProfile[storage.soundFont].poweronoffType, storage.sndProfile[storage.soundFont].mainColor, MAINBLADE_OFFSET, MAINBLADE_OFFSET+MAINBLADE_LENGTH);
        mainignition_done=true;
      #else // single blade or saber staff
        lightIgnition(ledPins, soundFont.getPowerOnTime(), storage.sndProfile[storage.soundFont].poweronoffType, storage.sndProfile[storage.soundFont].mainColor);
      #endif
      
      sndSuppress = millis()+soundFont.getPowerOnTime()+1000;
      sndSuppress2 = millis()+soundFont.getPowerOnTime()+1000;

    #ifdef CROSSGUARDSABER
      
      // ignite the crossguard after the defined wat time or latest when a new hum shall be relaunched
      while (millis() > sndSuppress2 and millis()-sndSuppress2<STAGGERED_IGNITION_DELAY and millis()-sndSuppress2<HUM_RELAUNCH) {
        lightFlicker(ledPins, storage.sndProfile[storage.soundFont].flickerType, 0, storage.sndProfile[storage.soundFont].mainColor, storage.sndProfile[storage.soundFont].clashColor, ActionModeSubStates, MAINBLADE_OFFSET, MAINBLADE_OFFSET+MAINBLADE_LENGTH);
      }        
        SinglePlay_Sound(soundFont.getClash((storage.soundFont)*NR_FILE_SF));
        lightIgnition(ledPins, CLASH_SUPRESS, storage.sndProfile[storage.soundFont].poweronoffType, storage.sndProfile[storage.soundFont].mainColor, CROSSGUARD_OFFSET, CROSSGUARD_LENGTH);
        sndSuppress2=millis();
        mainignition_done=false; // reset flag for next power up
    #endif
    
      // Get the initial position of the motion detector
      //motionEngine();
      //ActionModeSubStates = AS_HUM;
#if defined ACCENT_LED
      // turns accent LED On
      accentLEDControl(AL_ON);
#endif
  }
    // ************************* blade movement detection ************************************
    //Let's get our values !
    motionEngine();

    /*
       CLASH DETECTION :
       A clash is a violent deceleration when 2 blades hit each other
       For a realistic clash detection it's imperative to detect
       such a deceleration instantenously, which is only feasible
       using the motion interrupt feature of the MPU6050.
    */
    #ifdef CLASH_DET_MPU_POLL
      if (mpuIntStatus > 60 and mpuIntStatus < 70 and ActionModeSubStates != AS_BLADELOCKUP) {
        FX_Clash();
      }
    #endif // CLASH_DET_MPU_POLL

    /*
       SIMPLE BLADE MOVEMENT DETECTION FOR MOTION  TRIGGERED BLASTER FEDLECT
       We detect swings as hilt's orientation change
       since IMUs sucks at determining relative position in space
    */
    // movement of the hilt while blaster move deflect is activated can trigger a blaster deflect
    #ifdef CLASH_DET_MPU_POLL
    else if ((ActionModeSubStates == AS_BLASTERDEFLECTPRESS or (ActionModeSubStates == AS_BLASTERDEFLECTMOTION and (abs(curDeltAccel.y) > storage.sndProfile[storage.soundFont].swingSensitivity // and it has suffisent power on a certain axis
    #endif
    #ifdef CLASH_DET_MPU_INT
    if ((ActionModeSubStates == AS_BLASTERDEFLECTPRESS or (ActionModeSubStates == AS_BLASTERDEFLECTMOTION and (abs(curDeltAccel.y) > storage.sndProfile[storage.soundFont].swingSensitivity // and it has suffisent power on a certain axis
    #endif
              or abs(curDeltAccel.z) > storage.sndProfile[storage.soundFont].swingSensitivity
              or abs(curDeltAccel.x) > storage.sndProfile[storage.soundFont].swingSensitivity))) and (millis() - sndSuppress >= BLASTERBLOCK_SUPRESS)) {

        FX_BlasterBlock();

    }
    // CLASH state, flicker with clash color/brightness for the duration of CLASH_FX_DURATION
    else if (ActionModeSubStates==AS_CLASH){
      // check if duration expired
      if (millis() > sndSuppress and millis()-sndSuppress < CLASH_FX_DURATION) {
        lightFlicker(ledPins, storage.sndProfile[storage.soundFont].flickerType, 0, storage.sndProfile[storage.soundFont].mainColor, storage.sndProfile[storage.soundFont].clashColor, ActionModeSubStates);
      }
      else {
        ActionModeSubStates=AS_HUM;
        sndSuppress=millis();
      }
    }
    /*
       SWING DETECTION
       We detect swings as hilt's orientation change
       since IMUs sucks at determining relative position in space
    */
    else if (
      (not fireblade) and (ActionModeSubStates != AS_BLADELOCKUP
      #ifdef SWING_QUATERNION
        or lockuponclash// end lockuponclash event on a swing, but only if swings are calculated based on quaternions, otherwise swings will
                          // interrut the lockup uncontrollably
      #endif
      )
#ifndef SWING_QUATERNION
      and (abs(curDeltAccel.y) > storage.sndProfile[storage.soundFont].swingSensitivity // and it has suffisent power on a certain axis
              or abs(curDeltAccel.z) > storage.sndProfile[storage.soundFont].swingSensitivity
              or abs(curDeltAccel.x) > storage.sndProfile[storage.soundFont].swingSensitivity)
              and (millis() > sndSuppress and millis() - sndSuppress > SWING_SUPPRESS)
#else // SWING_QUATERNION is defined
        and abs(curRotation.w * 1000) < 999 // some rotation movement have been initiated
      and (
      (
        (millis() > sndSuppress and millis() - sndSuppress > SWING_SUPPRESS) // The movement doesn't follow another to closely
        and (abs(curDeltAccel.x) > storage.sndProfile[storage.soundFont].swingSensitivity  // and it has suffisent power on a certain axis
             or abs(curDeltAccel.z) > storage.sndProfile[storage.soundFont].swingSensitivity
             or abs(curDeltAccel.y) > storage.sndProfile[storage.soundFont].swingSensitivity * 10)
      )
      or (// A reverse movement follow a first one
        (millis() > sndSuppress2 and millis() - sndSuppress2 > SWING_SUPPRESS)   // The reverse movement doesn't follow another reverse movement to closely
        // and it must be a reverse movement on Vertical axis
        and (
          abs(curDeltAccel.x) > abs(curDeltAccel.z)
          and abs(prevDeltAccel.x) > storage.sndProfile[storage.soundFont].swingSensitivity
          and (
            (prevDeltAccel.x > 0
             and curDeltAccel.x < -storage.sndProfile[storage.soundFont].swingSensitivity)
            or (
              prevDeltAccel.x < 0
              and curDeltAccel.x  > storage.sndProfile[storage.soundFont].swingSensitivity
            )
          )
        )
      )
      or (// A reverse movement follow a first one
        (millis() > sndSuppress2 and millis() - sndSuppress2 > SWING_SUPPRESS)  // The reverse movement doesn't follow another reverse movement to closely
        and ( // and it must be a reverse movement on Horizontal axis
          abs(curDeltAccel.z) > abs(curDeltAccel.x)
          and abs(prevDeltAccel.z) > storage.sndProfile[storage.soundFont].swingSensitivity
          and (
            (prevDeltAccel.z > 0
             and curDeltAccel.z < -storage.sndProfile[storage.soundFont].swingSensitivity)
            or (
              prevDeltAccel.z < 0
              and curDeltAccel.z  > storage.sndProfile[storage.soundFont].swingSensitivity
            )
          )
        )
      )
      )
      // the movement must not be triggered by pure blade rotation (wrist rotation)
      and not (
        abs(prevRotation.y * 1000 - curRotation.y * 1000) > abs(prevRotation.x * 1000 - curRotation.x * 1000)
        and
        abs(prevRotation.y * 1000 - curRotation.y * 1000) > abs(prevRotation.z * 1000 - curRotation.z * 1000)
      ) 
#endif // SWING_QUATERNION 
  )     
      { // end of the condition definition for swings



      if ( ActionModeSubStates != AS_BLASTERDEFLECTMOTION and ActionModeSubStates != AS_BLASTERDEFLECTPRESS) {
        /*
            THIS IS A SWING !
        */
        prevDeltAccel = curDeltAccel;
#if defined LS_SWING_DEBUG
        Serial.print(F("SWING\ttime="));
        Serial.println(millis() - sndSuppress);
        Serial.print(F("\t\tcurRotation\tw="));
        Serial.print(curRotation.w * 1000);
        Serial.print(F("\t\tx="));
        Serial.print(curRotation.x);
        Serial.print(F("\t\ty="));
        Serial.print(curRotation.y);
        Serial.print(F("\t\tz="));
        Serial.print(curRotation.z);
        Serial.print(F("\t\tAcceleration\tx="));
        Serial.print(curDeltAccel.x);
        Serial.print(F("\ty="));
        Serial.print(curDeltAccel.y);
        Serial.print(F("\tz="));
        Serial.println(curDeltAccel.z);
#endif
       #ifdef CLASH_DET_MPU_POLL
        motionEngine();
        if (mpuIntStatus > 60 and mpuIntStatus < 70 and ActionModeSubStates != AS_BLADELOCKUP) {
          SinglePlay_Sound(soundFont.getClash((storage.soundFont)*NR_FILE_SF));
          sndSuppress = millis();
          sndSuppress2 = millis();
          /*
             THIS IS A CLASH  !
          */
          ActionModeSubStates = AS_CLASH;
          lightClashEffect(ledPins, storage.sndProfile[storage.soundFont].clashColor);
          if (!fireblade) {
            delay(CLASH_FX_DURATION);  // clash duration
          }       
        }
        else {
       #endif // CLASH_DET_MPU_POLL
          ActionModeSubStates = AS_SWING;
          SinglePlay_Sound(soundFont.getSwing((storage.soundFont)*NR_FILE_SF));
          /* NORMAL SWING */
  
#ifdef SWING_COLORCHANGE
          lightSwingEffect(ledPins);
          if (!fireblade) {
            delay(SWING_FX_DURATION);  // swing duration
          }  
#endif // SWING_COLORCHANGE
  
          if (millis() > sndSuppress and millis() - sndSuppress > SWING_SUPPRESS) {
            sndSuppress = millis();
          }
          if (millis() > sndSuppress2 and millis() - sndSuppress2 > SWING_SUPPRESS) {
            sndSuppress2 = millis();
          }
      #ifdef CLASH_DET_MPU_POLL
      }
      #endif // CLASH_DET_MPU_POLL
      }
    }
    else { // simply flicker
      if (ActionModeSubStates != AS_BLASTERDEFLECTMOTION and ActionModeSubStates != AS_BLADELOCKUP) { // do not deactivate blaster move deflect mode in case the saber is idling
        ActionModeSubStates = AS_HUM;
      }
      else if (ActionModeSubStates == AS_BLASTERDEFLECTMOTION) {
        accentLEDControl(AL_PULSE);
      }
      // relaunch hum if more than HUM_RELAUNCH time elapsed since entering AS_HUM state
      if (millis() > sndSuppress and millis() - sndSuppress > HUM_RELAUNCH and (not hum_playing) and ActionModeSubStates != AS_BLADELOCKUP) {
        HumRelaunch();
      }
#ifdef HUM_MODULATION
      // modulate the hum in function of rotation speed when moving slower than a swing
      bool modulate = max(abs(curRotation.x*1000),max(abs(curRotation.y*1000),abs(curRotation.z*1000))) > 1 // some rotation initiated             
             and millis() - sndSuppress > SWING_SUPPRESS + HUMMOD_SUPRESS // and not following a swing or reverse swing too closely
             and millis() - sndSuppress2 > SWING_SUPPRESS + HUMMOD_SUPRESS;     
      if (hmEnd >= hmStart and modulate and millis() - hmEnd > HUMMOD_SUPRESS) {
        hmStart=millis();
        dfplayer.setEqualizer(4);
      } else if (hmEnd < hmStart and !modulate and millis() - hmStart > HUMMOD_FX_DURATION and millis() - sndSuppress > HUMMOD_FX_DURATION)  {
        dfplayer.setEqualizer(0);
        hmEnd=millis();
      }
#endif      
      getColor(storage.sndProfile[storage.soundFont].mainColor);
      lightFlicker(ledPins, storage.sndProfile[storage.soundFont].flickerType, 0, storage.sndProfile[storage.soundFont].mainColor, storage.sndProfile[storage.soundFont].clashColor, ActionModeSubStates);

      if (lockuponclash) {
        accentLEDControl(AL_PULSE);
      }
    }
    // ************************* blade movement detection ends***********************************

  } ////END ACTION MODE HANDLER///////////////////////////////////////////////////////////////////////////////////////


  /*//////////////////////////////////////////////////////////////////////////////////////////////////////////
     CONFIG MODE HANDLER
  *//////////////////////////////////////////////////////////////////////////////////////////////////////////
  else if (SaberState == S_CONFIG) {
    // read out the motion sensor in order to be able to detect clashes in Config Mode for the "Hit-and-Run"
    motionEngine();
    if ((mpuIntStatus > 60 and mpuIntStatus < 70) and (millis() - sndSuppress >= CLASH_SUPRESS)) {
      sndSuppress = millis();
      // define what shall happen in each Config Sub-State if the Escape Path is activated
      #if defined LS_INFO
        Serial.println("Hit-and-Run is activated");
      #endif
      #ifdef SINGLEBUTTON
        // In case a clash is detected, move to the next menu item in single button mode
        NextConfigState();
      #else
        // in two button mode, activate different Hit-and-Run functions depending on the config state
        // for all other states:
        switch(ConfigModeSubStates) {
           default:
            NextConfigState();
          case CS_VOLUME:
            // turn on the volume full
            storage.volume = 30; //MAX
            BladeMeter(ledPins, storage.volume*100/30);
            Set_Volume(storage.volume); // Too Slow: we'll change volume on exit
            delay(50);
            #if defined LS_INFO
              Serial.println(storage.volume);
            #endif             
            break;
          case CS_SOUNDFONT:
            break;      
          case CS_FLICKERTYPE:
            break;
          case CS_POWERONOFFTYPE:
            break;
          case CS_SWINGSENSITIVITY:
          // upon clash increase swing sensitivity by 1/10th of a g (1g=16384)
          if (storage.sndProfile[storage.soundFont].swingSensitivity <= 14400 ) {
            storage.sndProfile[storage.soundFont].swingSensitivity=storage.sndProfile[storage.soundFont].swingSensitivity+1600;
          }
          else {
            storage.sndProfile[storage.soundFont].swingSensitivity=0;
          }
            BladeMeter(ledPins, (storage.sndProfile[storage.soundFont].swingSensitivity)/100);
            break;
          case CS_SLEEPINIT:
            break;
            }
      #endif // SINGLEBUTTON      
    }
    if (PrevSaberState == S_STANDBY) { // entering config mode
      PrevSaberState = S_CONFIG;
      if (storage.volume <= 15) {
        Set_Volume(15);
        delay(200);
      }
      SinglePlay_Sound(3);
      delay(600);

#if defined PIXELBLADE or defined ADF_PIXIE_BLADE
      pixelblade_KillKey_Disable();
#endif

#if defined LS_INFO
      Serial.println(F("START CONF"));
#endif
      enterMenu = true;
      ConfigModeSubStates=-1;
      NextConfigState();
    }
#if defined PIXELBLADE or defined STAR_LED or defined ADF_PIXIE_BLADE
    if (ConfigModeSubStates == CS_MAINCOLOR or ConfigModeSubStates == CS_CLASHCOLOR or ConfigModeSubStates == CS_BLASTCOLOR) {
      #ifdef GRAVITY_COLOR
        modification = GravityVector();
          switch (modification) {
            case (0): // red +
              currentColor.r = 100; currentColor.g = 0; currentColor.b = 0;
              break;
            case (1): // red -
              currentColor.r = 20; currentColor.g = 0; currentColor.b = 0;
              break;
            case (2): // green +
              currentColor.r = 0; currentColor.g = 100; currentColor.b = 0;
              break;
            case (3): // green -
              currentColor.r = 0; currentColor.g = 20; currentColor.b = 0;
              break;
            case (4): // blue +
              currentColor.r = 0; currentColor.g = 0; currentColor.b = 100;
              break;
            case (5): // blue -
              currentColor.r = 0; currentColor.g = 0; currentColor.b = 20;
              break;
          }
            // in case of a neopixel blade, show the gravity color on the last 5 pixel of the blade tip
            #ifdef PIXELBLADE
              lightOn(ledPins, -1, currentColor, NUMPIXELS - 5, NUMPIXELS);
            #else if STAR_LED
            #endif
          #ifdef ADF_PIXIE_BLADE
            if (ConfigModeSubStates == CS_MAINCOLOR) {
               lightOn(ledPins, -1, storage.sndProfile[storage.soundFont].mainColor);
           }
            else if (ConfigModeSubStates == CS_CLASHCOLOR) {
              lightOn(ledPins, -1, storage.sndProfile[storage.soundFont].clashColor);
            }
            else if (ConfigModeSubStates == CS_BLASTCOLOR) {
              lightOn(ledPins, -1, storage.sndProfile[storage.soundFont].blasterboltColor);
            }
          #endif
      #endif // GRAVITY_COLOR
    }
#endif // color configuration for PIXELBLADE or STAR_LED or ADF_PIXIE_BLADE
    if (ConfigModeSubStates == CS_FLICKERTYPE or ConfigModeSubStates==CS_SOUNDFONT) {
      lightFlicker(ledPins, storage.sndProfile[storage.soundFont].flickerType, 0, storage.sndProfile[storage.soundFont].mainColor, storage.sndProfile[storage.soundFont].clashColor, ActionModeSubStates);
    }
    else if (ConfigModeSubStates == CS_SWINGSENSITIVITY and (abs(curDeltAccel.y) > storage.sndProfile[storage.soundFont].swingSensitivity // and it has suffisent power on a certain axis
              or abs(curDeltAccel.z) > storage.sndProfile[storage.soundFont].swingSensitivity
              or abs(curDeltAccel.x) > storage.sndProfile[storage.soundFont].swingSensitivity)) {
                SinglePlay_Sound(soundFont.getSwing((storage.soundFont)*NR_FILE_SF));
              }
#ifdef BATTERY_CHECK             
    else if (ConfigModeSubStates == CS_BATTERYLEVEL) {
      MonitorBattery();
    }
#endif    
  } //END CONFIG MODE HANDLER

  /*//////////////////////////////////////////////////////////////////////////////////////////////////////////
     STANDBY MODE
  *//////////////////////////////////////////////////////////////////////////////////////////////////////////
  else if (SaberState == S_STANDBY) {
    
    if (PrevSaberState==S_SABERON and ActionModeSubStates == AS_RETRACTION) { // we just leaved Action Mode
      //detachInterrupt(0);
      ActionModeSubStates = AS_HUM;
      PrevSaberState = S_STANDBY;
      #if defined DEEP_SLEEP
        sleepTimer=millis(); // reset sleep time counter
      #endif
      SinglePlay_Sound(soundFont.getPowerOff((storage.soundFont)*NR_FILE_SF));
      changeMenu = false;
      modification = 0;
      #ifdef CROSSGUARDSABER
      mainignition_done=false;
      #endif
#if defined LS_INFO
      Serial.println(F("END ACTION"));
#endif
#ifndef SINGLEBUTTON
      lockupButton.setPressTicks(PRESS_CONFIG);
#endif
      lightRetract(ledPins, soundFont.getPowerOffTime(), storage.sndProfile[storage.soundFont].poweronoffType,storage.sndProfile[storage.soundFont].mainColor);
      pixelblade_KillKey_Enable();
    }
    if (PrevSaberState == S_CONFIG) { // we just leaved Config Mode
      saveConfig();
      PrevSaberState = S_STANDBY;
      #if defined DEEP_SLEEP
        sleepTimer=millis(); // reset sleep time counter
      #endif
      SinglePlay_Sound(3);
      //browsing = false;
     // set back volume to stored config value (if it's 0, saber will be muted)
      Set_Volume(storage.volume);
      delay(200);
      enterMenu = false;
      modification = 0;
      //dfplayer.setVolume(storage.volume);
      menu = 0;
#if defined STAR_LED
      getColor(storage.sndProfile[storage.soundFont].mainColor);
#endif
#if defined PIXELBLADE
      getColor(storage.sndProfile[storage.soundFont].mainColor);
#endif

#if (defined PIXELBLADE or defined ADF_PIXIE_BLADE) and not defined PIXEL_ACCENT
      pixelblade_KillKey_Enable();
#endif

#if defined LS_INFO
      Serial.println(F("END CONF"));
#endif
    }

    // switch of light in Stand-by mode
#if defined STAR_LED
    lightOff(ledPins, -1);
#else
    lightOff(ledPins, -1);
#endif

    if (not mainButton.isIdle()) {
      // read out the motion sensor in order to be able to detect clashes in Idle Mode for "Quick soundfont selection" through hitting the hilt while pressing the main button
      motionEngine();
      if ((mpuIntStatus > 60 and mpuIntStatus < 70) and (millis() - sndSuppress >= CLASH_SUPRESS)) {
        sndSuppress = millis();
        #if defined LS_DEBUG
          Serial.println("Quick soundfont selection");
        #endif
        ConfigModeSubStates=CS_SOUNDFONT; // simulate soundfont selection through menu
        ConfigMenuButtonEventHandler(false, SINGLE_CLICK, 1);       
        QuickSelectButtonEventHandler(); // ensure we don't enter config menu once a clash has been detected
      }
    }

    accentLEDControl(AL_ON);
    #if defined DEEP_SLEEP and SLEEPYTIME>5000
      if (millis() - sleepTimer > SLEEPYTIME) { // after the defined max idle time SLEEPYTIME automatically go to sleep mode
        SaberState=S_SLEEP;
        PrevSaberState=S_STANDBY;
      }
    #endif // DEEP_SLEEP
  } // END STANDBY MODE

#ifdef JUKEBOX
  /*//////////////////////////////////////////////////////////////////////////////////////////////////////////
     JUKEBOX MODE (a.k.a. MP3 player mode
  *//////////////////////////////////////////////////////////////////////////////////////////////////////////

  else if (SaberState == S_JUKEBOX) {
    if (PrevSaberState == S_STANDBY) { // just entered JukeBox mode
      PrevSaberState = S_JUKEBOX;
      SinglePlay_Sound(14);  // play intro sound of JukeBox mode
      delay(2500);
#if defined PIXELBLADE or defined ADF_PIXIE_BLADE
      pixelblade_KillKey_Disable();
#endif
#if defined LS_INFO
      Serial.println(F("START JUKEBOX"));
#endif
      // start playing the first song
      jb_track = NR_CONFIGFOLDERFILES + 1;
      SinglePlay_Sound(jb_track);  // JukeBox dir/files must be directly adjecent to config sounds on the SD card
    }
    if (jukebox_play) {
#ifdef LEDSTRINGS
      JukeBox_Stroboscope(ledPins);
#endif

#ifdef STAR_LED
      JukeBox_Stroboscope();
#endif

#ifdef PIXELBLADE
      getColor(storage.sndProfile[storage.soundFont].mainColor);
      JukeBox_Stroboscope(currentColor);
#endif
    }
  }
#endif  //  JUKEBOX
#ifdef DEEP_SLEEP
  else if (SaberState == S_SLEEP) {

    //if (PrevSaberState == S_CONFIG or ) { // just entered Sleep mode
      //byte old_ADCSRA = ADCSRA;
      // disable ADC to save power
      // disable ADC

      // repeat the interupt mask again here, it is already done in the setup() function
      // but for an unknown reason sometimes the device fails to wake up...
      // pin change interrupt masks (see below list)
      //PCMSK2 |= bit (PCINT20);   // pin 4 Aux button
      //PCMSK0 |= bit (PCINT4);    // pin 12 Main button
      //delay(300);
      //Serial.println("LoL");
      ADCSRA = 0;  // reduces another ~100uA!
      // turns accent LED Off
      accentLEDControl(AL_OFF);
      // disable watchdog before going into sleep mode
      wdt_disable(); // do not remove!!!
      SleepModeEntry();
      // .. and the code will continue from here

      SleepModeExit();
      SaberState = S_STANDBY;
      PrevSaberState = S_SLEEP;
      ADCSRA = 135; // old_ADCSRA;   // re-enable ADC conversion
      // play boot sound
      SinglePlay_Sound(11);
      delay(20);
    //}
  }
#endif // DEEP_SLEEP
#ifdef PIXEL_ACCENT
  pixelAccentUpdate();
#endif
} //loop

// ====================================================================================
// ===           	  			MOTION DETECTION FUNCTIONS	            			===
// ====================================================================================
inline void motionEngine() {
  // if programming failed, don't try to do anything
  if (!dmpReady)
    return;

  // wait for MPU interrupt or extra packet(s) available
  //	while (!mpuInterrupt && mpuFifoCount < packetSize) {
  //		/* other program behavior stuff here
  //		 *
  //		 * If you are really paranoid you can frequently test in between other
  //		 * stuff to see if mpuInterrupt is true, and if so, "break;" from the
  //		 * while() loop to immediately process the MPU data
  //		 */
  //	}
  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  mpuFifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || mpuFifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (mpuFifoCount < packetSize)
      mpuFifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    mpuFifoCount -= packetSize;

#if defined SWING_QUATERNION || defined HUM_MODULATION
    //Making the last orientation the reference for next rotation
    prevOrientation = curOrientation.getConjugate();
#endif // SWING_QUATERNION or HUM_MODULATION
    prevAccel = curAccel;

    //retrieve current orientation value
#if defined SWING_QUATERNION || defined HUM_MODULATION
    mpu.dmpGetQuaternion(&curOrientation, fifoBuffer);
#endif // SWING_QUATERNION or HUM_MODULATION
    mpu.dmpGetAccel(&curAccel, fifoBuffer);
    curDeltAccel.x = prevAccel.x - curAccel.x;
    curDeltAccel.y = prevAccel.y - curAccel.y;
    curDeltAccel.z = prevAccel.z - curAccel.z;

#if defined SWING_QUATERNION || defined HUM_MODULATION
    //We calculate the rotation quaternion since last orientation
    prevRotation = curRotation;
    curRotation = prevOrientation.getProduct(
                    curOrientation.getNormalized());
#endif // SWING_QUATERNION or HUM_MODULATION
#if defined LS_MOTION_HEAVY_DEBUG
    // display quaternion values in easy matrix form: w x y z
    printQuaternion(curRotation);
#endif

  }
} //motionEngine

inline void dmpDataReady() {
  mpuInterrupt = true;
} //dmpDataReady

#if defined LS_MOTION_DEBUG
inline void printQuaternion(Quaternion quaternion) {
  Serial.print(F("\t\tQ\t\tw="));
  Serial.print(quaternion.w * 1000);
  Serial.print(F("\t\tx="));
  Serial.print(quaternion.x);
  Serial.print(F("\t\ty="));
  Serial.print(quaternion.y);
  Serial.print(F("\t\tz="));
  Serial.println(quaternion.z);
} //printQuaternion
#endif

uint8_t GravityVector() {
  uint8_t Orientation; // 0: +X, 1: -X, 2: +Y, 3: -Y, 4: +Z, 5: -Z
  int16_t ax, ay, az;

  //mpu.dmpGetAccel(&curAccel, fifoBuffer);
  mpu.getAcceleration(&ax, &ay, &az);
  //printAccel(ax, ay, az);
  if (ax < 0) {
    Orientation = 1; // -X
  }
  else {
    Orientation = 0; // +X
  }
  if (abs(abs(ax) - 16000) > abs(abs(ay) - 16000)) {
    if (ay < 0) {
      Orientation = 3; // -Y
    }
    else {
      Orientation = 2; // +Y
    }
  }
  if ( (abs(abs(ay) - 16000) > abs(abs(az) - 16000)) and (abs(abs(ax) - 16000) > abs(abs(az) - 16000)) ) {
    if (az < 0) {
      Orientation = 5; // -Z
    }
    else {
      Orientation = 4; // +Z
    }
  }
  //Serial.print(F("\t\Orientation="));
  //Serial.println(Orientation);
  return Orientation;
}

void FX_Clash() {
        #if defined LS_CLASH_DEBUG
              Serial.print(F("CLASH\tmpuIntStatus="));
              Serial.println(mpuIntStatus);
        #endif
        if (lockuponclash) {
          //if (ActionModeSubStates==AS_PREBLADELOCKUP or lockuponclash) {
          //Lockup Start
          ActionModeSubStates = AS_BLADELOCKUP;
          if (soundFont.getLockup((storage.soundFont)*NR_FILE_SF)) {
            LoopPlay_Sound(soundFont.getLockup((storage.soundFont)*NR_FILE_SF));
          }
        }
        else { // ordinary clash
          if (millis() > clashSndSuppress and millis() - clashSndSuppress >= CLASH_SUPRESS) {
            SinglePlay_Sound(soundFont.getClash((storage.soundFont)*NR_FILE_SF));
            /*
               THIS IS A CLASH  !
            */
            ActionModeSubStates = AS_CLASH;
            // the next function only activates the clash color (and flashes in case of fireblade)
            lightClashEffect(ledPins, storage.sndProfile[storage.soundFont].clashColor);
            
            if (!fireblade) {
              //delay(CLASH_FX_DURATION);  // clash duration
              //delayMicroseconds(CLASH_FX_DURATION*1000);
            }
            sndSuppress = millis();
            sndSuppress2 = millis();
            clashSndSuppress=millis();           
          }
        }
}
#ifdef CLASH_DET_MPU_INT
  void ISR_MPUInterrupt() {
      if (SaberState==S_SABERON) {
        FX_Clash();
      }
  }
#endif

void FX_BlasterBlock() {
  
      if (soundFont.getBlaster((storage.soundFont)*NR_FILE_SF)) {
        SinglePlay_Sound(soundFont.getBlaster((storage.soundFont)*NR_FILE_SF));
#if defined STAR_LED
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        //lightOn(ledPins, -1, currentColor);
        //delay(BLASTER_FX_DURATION);
        lightBlasterEffect(ledPins, 0, 0, BLASTER_FX_DURATION,storage.sndProfile[storage.soundFont].blasterboltColor);
#endif //STAR_LED
#if defined PIXELBLADE
      if (fireblade) { // #ifdef FIREBLADE
        blasterPixel = random(NUMPIXELS / 4, NUMPIXELS - 3); //momentary shut off one led segment
        lightBlasterEffect(ledPins, blasterPixel, map(NUMPIXELS, 10, NUMPIXELS-10, 1, 2), BLASTER_FX_DURATION, storage.sndProfile[storage.soundFont].blasterboltColor);
      }
      else { // #else
        lightOn(ledPins, -1, currentColor);
        blasterPixel = random(NUMPIXELS / 4, NUMPIXELS - 3); //momentary shut off one led segment
        getColor(storage.sndProfile[storage.soundFont].blasterboltColor);
        lightBlasterEffect(ledPins, blasterPixel, map(NUMPIXELS, 10, NUMPIXELS-10, 1, 2), BLASTER_FX_DURATION,storage.sndProfile[storage.soundFont].blasterboltColor);
      } //
#endif
        //delay(BLASTER_FX_DURATION);  // blaster bolt deflect duration
        // Some Soundfont may not have Blaster sounds
        if (millis() > sndSuppress and millis() - sndSuppress > 50) {
          //SinglePlay_Sound(soundFont.getBlaster((storage.soundFont)*NR_FILE_SF));
          sndSuppress = millis();
        }
      }  
}

// ====================================================================================
// ===           	  			EEPROM MANIPULATION FUNCTIONS	            		===
// ====================================================================================

inline bool loadConfig() {
  bool equals = true;
  EEPROM.readBlock(configAdress, storage);
  for (uint8_t i = 0; i <= 2; i++) {
    if (storage.version[i] != CONFIG_VERSION[i]) {
      equals = false;
      Serial.println("Wrong config!");
    }
  }
  Serial.println(storage.version);
  //Serial.print("ADC status:"); Serial.println(ADCSRA);
  return equals;
} //loadConfig

inline void saveConfig() {
  EEPROM.updateBlock(configAdress, storage);
//#ifdef LS_DEBUG
  // dump values stored in EEPROM
  //for (uint8_t i = 0; i < 255; i++) {
  //  Serial.print(i); Serial.print("\t"); Serial.println(EEPROM.readByte(i));
  //}
//#endif
} //saveConfig


void DumpConfigEEPROM() {
//#ifdef LS_DEBUG
  // dump values stored in EEPROM
  for (uint8_t i = configAdress; i < configAdress+sizeof(StoreStruct); i++) {
    Serial.print(i); Serial.print("\t"); Serial.println(EEPROM.readByte(i));
  }
//#endif
}
// ====================================================================================
// ===                          SOUND FUNCTIONS                                     ===
// ====================================================================================

void HumRelaunch() {
  LoopPlay_Sound(soundFont.getHum((storage.soundFont)*NR_FILE_SF));
  sndSuppress = millis();
  hum_playing = true;
}

void SinglePlay_Sound(uint8_t track) {
  dfplayer.playPhysicalTrack(track);
  #ifdef DFPLAYER_CLONE
  delay(100); 
  dfplayer.setSingleLoop(false); // fixes incorrect looping of certain sounds on clone chips
  #endif
}

void LoopPlay_Sound(uint8_t track) {
  dfplayer.playSingleLoop(track);
}

void Set_Volume(int8_t volumeSet) {
  dfplayer.setVolume(volumeSet); // Too Slow: we'll change volume on exit
  delay(50);
}

void Set_Loop_Playback() {
  dfplayer.setSingleLoop(true);;
}

void InitDFPlayer() {
  dfplayer.setSerial(DFPLAYER_TX, DFPLAYER_RX);
}

void Pause_Sound() {

  dfplayer.pause();
}

void Resume_Sound() {
  dfplayer.play();
}

#ifdef DEEP_SLEEP
// ====================================================================================
// ===                          SLEEP MODE FUNCTIONS                                ===
// ====================================================================================

void sleepNow()         // here we put the arduino to sleep
{

    power_all_disable ();   // turn off all modules -> no measurable effect
     
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

    sleep_enable();          // enables the sleep bit in the mcucr register
                             // so sleep is possible. just a safety pin 

    // turn off brown-out enable in software -> no measurable effect
    MCUCR = bit (BODS) | bit (BODSE);
    MCUCR = bit (BODS); 
  
    PCIFR  |= bit (PCIF0) | bit (PCIF1) | bit (PCIF2);   // clear any outstanding interrupts
    PCICR  |= bit (PCIE0) | bit (PCIE1) | bit (PCIE2);   // enable pin change interrupts

    //PCMSK0 |= bit (PCINT4);    // enable pin change interrupt pin 12 Main button
    //delay(300);
    //enableInterrupt(12, SleepModeExit, CHANGE);
    
    sleep_mode();            // here the device is actually put to sleep!!
                             // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP


  
    sleep_disable();         // first thing after waking from sleep:
                             // disable sleep...
    detachInterrupt(0);      // disables interrupt 0 on pin 2 so the 
                             // wakeUpNow code will not be executed 
                             // during normal running time.

                             // enable watchdog to avoid system hang
}

void SleepModeEntry() {
  // switch off all LS channels
  pixelblade_KillKey_Enable();
  for (uint8_t i = 0; i < sizeof(ledPins); i++) {
    digitalWrite(ledPins[i], LOW);
  }
  // pin change interrupt masks (see below list)
  //PCMSK2 |= bit (PCINT20);   // pin 4 Aux button
  PCMSK0 |= bit (PCINT4);    // pin 12 Main button
  // contrain the communication signals to the MP3 player to low, otherwise they will back-supply the module
  pinMode(DFPLAYER_RX, OUTPUT);
  digitalWrite(DFPLAYER_RX, LOW);
  pinMode(DFPLAYER_TX, OUTPUT);
  digitalWrite(DFPLAYER_TX, LOW);
  mpu.setSleepEnabled(true);
  Disable_MP3(true);
  //mpu.setSleepEnabled(true); // included as dummy, for an unknown reason if it's not here and the dfplayer.sleep() is commented out, sound is disabled
  //dfplayer.sleep();
  delay (300);
  Disable_FTDI(true);
  sleepNow();     // sleep function called here
}

void SleepModeExit() {

  // cancel sleep as a precaution
  sleep_disable();
  power_all_enable ();   // enable modules again
  Disable_FTDI(false);
  Disable_MP3(false);
  pinMode(DFPLAYER_RX, OUTPUT);
  pinMode(DFPLAYER_TX, INPUT);
  
  // enable watchdog to avoid system hang
  wdt_reset();
  wdt_enable(WDTO_8S);
  //WDTCSR = (1<<WDCE) | (1<<WDE) | (1<<WDP3) | (1<<WDP0);
  wdt_reset(); 
  //digitalWrite(11,HIGH);
  //delay(1000);
  //digitalWrite(11,LOW);
  // MPU init after wake up to avoid system hang
  /*
  delay(300);
  Serial.println("Waking up MPU - who knows, maybe it still sleeps");
  mpu.setSleepEnabled(false);
  Serial.println("Reset MPU I2C Master");
mpu.resetI2CMaster();
Serial.println("Reseting MPU");
  mpu.reset();
  delay(100);
Serial.println("Initializing MPU");
  mpu.initialize();
  */
  setup(); // redo all initializations
}


#endif // DEEP_SLEEP

void Disable_FTDI(bool ftdi_off) {

  if (ftdi_off) {  //  disable FTDI
    digitalWrite(FTDI_PSWITCH, HIGH); // disable the FTDI chip

  }
  else {  //  enable ftdi
    digitalWrite(FTDI_PSWITCH, LOW); // enable the FTDI chip
  }
  
}

void Disable_MP3(bool mp3_off) {

  if (mp3_off) {  //  disable MP3
    digitalWrite(MP3_PSWITCH, HIGH); // disable the MP3 chip and the audio amp
  }
  else {  //  enable MP3
    digitalWrite(MP3_PSWITCH, LOW); // enable the MP3 chip and the audio amp
  }
  
}

#ifdef BATTERY_CHECK
// ====================================================================================
// ===                         BATTERY CHECKING FUNCTIONS                           ===
// ====================================================================================


float batCheck() {
  float sum = 0;
  // take a number of analog samples and add them up
  analogReference(INTERNAL);
  for (int i = 0; i < 10; i++) {
    analogRead(BATTERY_READPIN);  // clear the reads after reference switch
    delay(1);
  }
  for (int i = 0; i < 10; i++) {
    sum += analogRead(BATTERY_READPIN);
    //Serial.println(analogRead(BATTERY_READPIN));
    delay(10);
  }
  // 5.0V is the calibrated reference voltage
  float voltage = ((float)sum / 10 * BATTERY_FACTOR) / 1023.0;
  analogReference(DEFAULT);
  for (int i = 0; i < 10; i++) {
    analogRead(BATTERY_READPIN);  // clear the reads after reference switch
    delay(1);
  }
  Serial.print(F("Battery Level: ")); Serial.println(voltage);
  return voltage;
  // return 3.2; //temporary value for testing
}

void BatLevel_ConfigEnter() {
  #ifdef DIYINO_PRIME
      //      int batLevel = 100 * (1 / batCheck() - 1 / LOW_BATTERY) / (1 / FULL_BATTERY - 1 / LOW_BATTERY);
      int batLevel = 100 * ((batCheck() - LOW_BATTERY) / (FULL_BATTERY - LOW_BATTERY));
  #endif
  #if defined DIYINO_STARDUST_V2 or defined DIYINO_STARDUST_V3
      // flush out the ADC
      getBandgap();
      getBandgap();
      getBandgap();
      int batLevel=((getBandgap()/37)*10);
      Serial.println(batLevel);
  #endif      
      if (batLevel > 95) {        //full
        SinglePlay_Sound(19);
      } else if (batLevel > 60) { //nominal
        SinglePlay_Sound(15);
      } else if (batLevel > 30) { //diminished
        SinglePlay_Sound(16);
      } else if (batLevel > 0) {  //low
        SinglePlay_Sound(17);
      } else {                    //critical
        SinglePlay_Sound(18);
      }
      BladeMeter(ledPins, batLevel);
      AccentMeter(batLevel);
      delay(1000);
}

void MonitorBattery() {
  #ifdef DIYINO_PRIME
    //BatteryStatus=analogRead(BATTERY_READPIN);
  #endif
  #if defined DIYINO_STARDUST_V2 or defined DIYINO_STARDUST_V3
    BladeMeter(ledPins,((getBandgap()-300)/7)*10);
    //Serial.println(getBandgap());
  #endif
}

// Code courtesy of "Coding Badly" and "Retrolefty" from the Arduino forum
// results are Vcc * 100
// So for example, 5V would be 500.
int getBandgap () 
  {
  // REFS0 : Selects AVcc external reference
  // MUX3 MUX2 MUX1 : Selects 1.1V (VBG)  
   ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
   ADCSRA |= bit( ADSC );  // start conversion
   while (ADCSRA & bit (ADSC))
     { }  // wait for conversion to complete
   int results = (((InternalReferenceVoltage * 1024) / ADC) + 5) / 10; 
   //Serial.print("battery voltage: ");Serial.println(results);
   return results;
  } // end of getBandgap
#endif