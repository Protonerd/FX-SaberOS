// Arduino sketch that returns calibration offsets for MPU6050 //   Version 1.1  (31th January 2014)
// Done by Luis Ródenas <luisrodenaslorda@gmail.com>
// Based on the I2Cdev library and previous work by Jeff Rowberg <jeff@rowberg.net>
// Updates (of the library) should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib

// These offsets were meant to calibrate MPU6050's internal DMP, but can be also useful for reading sensors. 
// The effect of temperature has not been taken into account so I can't promise that it will work if you 
// calibrate indoors and then use it outdoors. Best is to calibrate and use at the same room temperature.

/* ==========  LICENSE  ==================================
 I2Cdev device library code is placed under the MIT license
 Copyright (c) 2011 Jeff Rowberg
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 =========================================================
 */

// I2Cdev and MPU6050 must be installed as libraries
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <EEPROMex.h> // include EEPROM library to write the calibrated offset values into the EEPROM

//#define MEMORYBASEMPUCALIBOFFSET 96 // start storing the calibration values starting at this address (LSOS 1.4 and earlier)
#define MEMORYBASEMPUCALIBOFFSET 200 // start storing the calibration values starting at this address (LSOS 1.5+ and FX-SaberOS)
#define DEBUGVERBOSE
///////////////////////////////////   CONFIGURATION   /////////////////////////////
//Change this 3 variables if you want to fine tune the skecth to your needs.
int buffersize=1000;     //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int discardfirstmeas=100;  // Amount of initial measurements to be discarded
int acel_deadzone=10;     //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone=10;     //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)
int accel_offset_divisor=8; //8;
int gyro_offset_divisor=4; //4;
// deadzone: amount of variation between 2 consecutive measurements

// default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az,gx, gy, gz;

int mean_ax,mean_ay,mean_az,mean_gx,mean_gy,mean_gz,state=0;
int ax_offset,ay_offset,az_offset,gx_offset,gy_offset,gz_offset;
int ax_initoffset,ay_initoffset,az_initoffset,gx_initoffset,gy_initoffset,gz_initoffset;
int16_t ax_offsetEEPROM,ay_offsetEEPROM,az_offsetEEPROM,gx_offsetEEPROM,gy_offsetEEPROM,gz_offsetEEPROM;
unsigned int calibratedOffsetAdress = 0;

unsigned int loopcount=0;


///////////////////////////////////   SETUP   ////////////////////////////////////
void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
  Wire.begin();
  // COMMENT NEXT LINE IF YOU ARE USING ARDUINO DUE
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Leonardo measured 250kHz.

  // initialize serial communication
  Serial.begin(115200);

  // initialize device
  accelgyro.initialize();

  // wait for ready
  while (Serial.available() && Serial.read()); // empty buffer
  while (!Serial.available()){
    Serial.println(F("Send any character to start sketch.\n"));
    delay(1500);
  }                
  while (Serial.available() && Serial.read()); // empty buffer again

  // start message
  Serial.println("\nMPU6050 Calibration Sketch");
  delay(2000);
  Serial.println("\nYour MPU6050 should be placed in horizontal position, with package letters facing up. \nDon't touch it until you see a finish message.\n");
  delay(3000);
  // verify connection
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  delay(1000);
  // reset offsets
  ax_initoffset=0;
  ay_initoffset=0;
  az_initoffset=0;
  gx_initoffset=0;
  gy_initoffset=0;
  gz_initoffset=0;

  accelgyro.setXAccelOffset(ax_initoffset);
  accelgyro.setYAccelOffset(ay_initoffset);
  accelgyro.setZAccelOffset(az_initoffset);
  accelgyro.setXGyroOffset(gx_initoffset);
  accelgyro.setYGyroOffset(gy_initoffset);
  accelgyro.setZGyroOffset(gz_initoffset);
  
          // set the fll scale range of the gyro- and accelerometer respectively
        accelgyro.setFullScaleGyroRange(0); //0: 250deg/s | 1: 500deg/s | 2: 1000deg/s | 3: 2000deg/s
        accelgyro.setFullScaleAccelRange(0); //0: 2g | 1: 4g | 2: 8g | 3: 16g
        
  // Get config from EEPROM if there is one
  // or initialise value with default ones set in StoreStruct
  EEPROM.setMemPool(MEMORYBASEMPUCALIBOFFSET, EEPROMSizeATmega328); //Set memorypool base, assume Arduino Uno board

}

///////////////////////////////////   LOOP   ////////////////////////////////////
void loop() {
  
  bool CalibResult=false;
  
  if (state==0){
    Serial.println("\nReading sensors for first time...");
    meansensors();
    //if (abs(mean_ax)>=32000){ax_initoffset=-mean_ax;Serial.println("\nRemove X-axis deadlock...");}
    //if (abs(mean_ay)>=32000){ay_initoffset=-mean_ay;Serial.println("\nRemove Y-axis deadlock...");}
    //if (mean_az<-32000){az_initoffset=-mean_az;Serial.println("\nRemove Z-axis deadlock...");}
    //if (abs(mean_gx)>=32000){gx_initoffset=-mean_gx;Serial.println("\nRemove Gyro X-axis deadlock...");}
    //if (abs(mean_gy)>=32000){gy_initoffset=-mean_gy;Serial.println("\nRemove Gyro Y-axis deadlock...");}
    //if (mean_gz<-32000){gz_initoffset=-mean_gz;Serial.println("\nRemove Gyro Z-axis deadlock...");}
    state++;
    delay(1000);
  }

  if (state==1) {
    Serial.println("\nCalculating offsets...");
    CalibResult=calibration();
    if (CalibResult) {
      Serial.println("\nCalibration successful!");
    }
    else {
      Serial.println("\nCalibration failed!");
    }
    state++;
    delay(1000);
  }

  if (state==2) {
    meansensors();
    ax_offsetEEPROM=ax_offset;
    ay_offsetEEPROM=ay_offset;
    az_offsetEEPROM=az_offset;
    gx_offsetEEPROM=gx_offset;
    gy_offsetEEPROM=gy_offset;
    gz_offsetEEPROM=gz_offset;
    
    Serial.println("\nFINISHED!");
    Serial.print("\nSensor readings with offsets:\t");
    Serial.print(mean_ax); 
    Serial.print("\t");
    Serial.print(mean_ay); 
    Serial.print("\t");
    Serial.print(mean_az); 
    Serial.print("\t");
    Serial.print(mean_gx); 
    Serial.print("\t");
    Serial.print(mean_gy); 
    Serial.print("\t");
    Serial.println(mean_gz);
    Serial.print("Your offsets:\t");
    Serial.print(ax_offset+ax_initoffset); 
    Serial.print("\t");
    Serial.print(ay_offset+ay_initoffset); 
    Serial.print("\t");
    Serial.print(az_offset+az_initoffset); 
    Serial.print("\t");
    Serial.print(gx_offset+gx_initoffset); 
    Serial.print("\t");
    Serial.print(gy_offset+gy_initoffset); 
    Serial.print("\t");
    Serial.println(gz_offset+gz_initoffset); 
    Serial.println("\nData is printed as: acelX acelY acelZ giroX giroY giroZ");
    Serial.println("Check that your sensor readings are close to 0 0 16384 0 0 0");
    Serial.println("If calibration was succesful write down your offsets so you can set them in your projects using something similar to mpu.setXAccelOffset(youroffset)");
    
    // wait for ready
  while (Serial.available() && Serial.read()); // empty buffer
  while (!Serial.available()){
    Serial.println(F("If you want to store the calibrated offset values into EEPROM, press \"y\" \n"));
    delay(1500);
  }
  int inByte=0; 
  if (Serial.available() > 0) {
  //while (Serial.available() && Serial.read()); // empty buffer again
    inByte = Serial.read();
    #ifdef DEBUGVERBOSE
      Serial.print("Pressed key: ");Serial.println(inByte);
    #endif
  }
  
  if (inByte==121) { // "y"
      // 1
        int addressInt = MEMORYBASEMPUCALIBOFFSET;
        int output;
        addressInt       = EEPROM.getAddress(sizeof(int));
        EEPROM.updateInt(addressInt,ax_offsetEEPROM+ax_initoffset);
        #ifdef DEBUGVERBOSE
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
        // 2
        addressInt       = EEPROM.getAddress(sizeof(int));
        EEPROM.updateInt(addressInt,ay_offsetEEPROM+ay_initoffset);
        #ifdef DEBUGVERBOSE
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
        // 3
        addressInt       = EEPROM.getAddress(sizeof(int));
        EEPROM.updateInt(addressInt,az_offsetEEPROM+ax_initoffset);
        #ifdef DEBUGVERBOSE
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
       // 4
        addressInt       = EEPROM.getAddress(sizeof(int));
        EEPROM.updateInt(addressInt,gx_offsetEEPROM+gx_initoffset);
        #ifdef DEBUGVERBOSE
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
        // 5
        addressInt       = EEPROM.getAddress(sizeof(int));
        EEPROM.updateInt(addressInt,gy_offsetEEPROM+gy_initoffset);
        #ifdef DEBUGVERBOSE
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
       // 6
        addressInt       = EEPROM.getAddress(sizeof(int));
        EEPROM.updateInt(addressInt,gz_offsetEEPROM+gz_initoffset);
        #ifdef DEBUGVERBOSE
          output = EEPROM.readInt(addressInt);
          Serial.print("address: ");Serial.println(addressInt);Serial.print("output: ");Serial.println(output);Serial.println("");
        #endif
        Serial.println("Calibrated offset values stored in EEPROM!");
  }
  else {
   Serial.println("Exiting without storing calibrated offset values into EEPROM");
  }
                
 #ifdef DEBUGVERBOSE
   // dump values stored in EEPROM
   for (uint8_t i = 0; i < 255; i++) {
     Serial.println(EEPROM.readByte(i));
   }
 #endif
    while (1);
  }
}

///////////////////////////////////   FUNCTIONS   ////////////////////////////////////
void meansensors(){
  long i=0,buff_ax=0,buff_ay=0,buff_az=0,buff_gx=0,buff_gy=0,buff_gz=0;
  
  while (i<(buffersize+discardfirstmeas+1)){
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    if (i>discardfirstmeas && i<=(buffersize+discardfirstmeas)){ //First 100 measures are discarded
      buff_ax=buff_ax+ax;
      buff_ay=buff_ay+ay;
      buff_az=buff_az+az;
      buff_gx=buff_gx+gx;
      buff_gy=buff_gy+gy;
      buff_gz=buff_gz+gz;
    }
    if (i==(buffersize+100)){
      mean_ax=buff_ax/buffersize;
      mean_ay=buff_ay/buffersize;
      mean_az=buff_az/buffersize;
      mean_gx=buff_gx/buffersize;
      mean_gy=buff_gy/buffersize;
      mean_gz=buff_gz/buffersize;
    }
    i++;
    delay(2); //Needed so we don't get repeated measures
  }
  
  Serial.print("Results of measurements a/g:\t");
  Serial.print(mean_ax); Serial.print("\t");
  Serial.print(mean_ay); Serial.print("\t");
  Serial.print(mean_az); Serial.print("\t");
  Serial.print(mean_gx); Serial.print("\t");
  Serial.print(mean_gy); Serial.print("\t");
  Serial.println(mean_gz);
}

bool calibration(){
  ax_offset=-mean_ax/accel_offset_divisor;
  ay_offset=-mean_ay/accel_offset_divisor;
  //az_offset=-mean_az/accel_offset_divisor;
  az_offset=(16384-mean_az)/8;

  gx_offset=-mean_gx/gyro_offset_divisor;
  gy_offset=-mean_gy/gyro_offset_divisor;
  gz_offset=-mean_gz/gyro_offset_divisor;
  while (1){
    int ready=0;
    accelgyro.setXAccelOffset(ax_offset+ax_initoffset);
    accelgyro.setYAccelOffset(ay_offset+ay_initoffset);
    accelgyro.setZAccelOffset(az_offset+az_initoffset);

    accelgyro.setXGyroOffset(gx_offset+gx_initoffset);
    accelgyro.setYGyroOffset(gy_offset+gy_initoffset);
    accelgyro.setZGyroOffset(gz_offset+gz_initoffset);

    meansensors();
    Serial.println("...");

    if (abs(mean_ax)<=acel_deadzone) ready++;
    else ax_offset=ax_offset-mean_ax/acel_deadzone;

    if (abs(mean_ay)<=acel_deadzone) ready++;
    else ay_offset=ay_offset-mean_ay/acel_deadzone;

    //if (abs(mean_az)<=acel_deadzone) ready++;
    //else az_offset=az_offset-mean_az/acel_deadzone;

    if (abs(16384-mean_az)<=acel_deadzone) ready++;
    else az_offset=az_offset+(16384-mean_az)/acel_deadzone;

    if (abs(mean_gx)<=giro_deadzone) ready++;
    else gx_offset=gx_offset-mean_gx/(giro_deadzone+1);

    if (abs(mean_gy)<=giro_deadzone) ready++;
    else gy_offset=gy_offset-mean_gy/(giro_deadzone+1);

    if (abs(mean_gz)<=giro_deadzone) ready++;
    else gz_offset=gz_offset-mean_gz/(giro_deadzone+1);

    if (ready==6) {
     return true;   
     break;
    }

    Serial.print("Resulting offset calibration value a/g:\t");
  Serial.print(ax_offset+ax_initoffset); Serial.print("\t");
  Serial.print(ay_offset+ay_initoffset); Serial.print("\t");
  Serial.print(az_offset+az_initoffset); Serial.print("\t");
  Serial.print(gx_offset+gx_initoffset); Serial.print("\t");
  Serial.print(gy_offset+gy_initoffset); Serial.print("\t");
  Serial.println(gz_offset+gz_initoffset);
  loopcount=loopcount+1;
  Serial.print("Loop Cnt: ");Serial.println(loopcount);
  if (loopcount==20) {
     return false;   
     break; // exit the calibration routine if no stable results can be obtained after 20 calibration loops
    }
  }
}
