/*
 * DFPlayer V 0.3
 *
 * Created on: 	27 feb 2016
 * author: 		Sebastien CAPOU (neskweek@gmail.com)
 * Source : 	https://github.com/neskweek/LightSaberOS
 * Description:	library for DFPlayer mini sound board
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/.
 */

#include "Arduino.h"
#include "DFPlayerSerial.h"

#ifndef _DFPLAYER_H_
#define _DFPLAYER_H_

class DFPlayer {
#define DFPLAYER_INFO
//#define DFPLAYER_DEBUG
//#define DFPLAYER_HEAVY_DEBUG

#define DFPLAYER_BUFFER_LENGTH	10
	/*
	 * MAX_FIFO_SIZE
	 * Actually the real size of the DFPlayer serial FIFO queue is 68 bits.
	 * As each message sent weights 10 bytes, this FIFO can only contains 6
	 * full valid messages.
	 */
#define DFPLAYER_FIFO_SIZE   60

	/**
	 * The value below comes from different source.
	 * See the number right after the comment
	 * 1 => From datasheet PDF
	 * 2 => From DFRobot wiki http://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299
	 * 3 => From original DFPlayer librarie V1.0 made by DFRobot
	 * 4 => From Personal reverse engineering
	 *
	 *
	 * All the value commented and noted UNKNOW :
	 * - were documented by DFRobot but doesn't produce what they are supposed to do
	 * OR
	 * - must exists but I don't know how to use them or what they do.
	 *
	 * Further reverse engineering to come on those.
	 */

	/* OPERATING_DELAY
	 * Uncompressible delay needed for the DFPlayer to receive and apply commands correctly
	 * in milliseconds
	 * if this delay is reduced:
	 * - at best your DFPlayer won't handle the mp3Serial line correctly
	 *   resulting in unpredictable behaviour.
	 * - at worse it may crash the DFPlayer that will require a reset
	 * */
//#define OPERATING_DELAY 155

	/*
	 * DFPLAYER COMMANDS
	 */

#define NEXT 			0x01 //1 command "Next"
#define	PREVIOUS 		0x02 //1 command "Previous"
#define PLAY_TRACK		0x03 //1 command "Play specified track"(NUM) 0-2999
#define VOL_UP			0x04 //1 command "Increase volume"
#define VOL_DWN			0x05 //1 command "Decrease volume"
#define VOLUME			0x06 //1 command "Set specified volume"  range :0-30
#define EQUALIZER		0x07 //1 command "Set Specified EQ" (0/1/2/3/4/5) Normal/Pop/Rock/Jazz/Classic/Base
#define	PLAY_REPEAT 	0x08 //1 command "Play loop the specified track
#define SOURCE			0x09 //1 command "Set Specified playback source"(0/1/2/3/4) U/TF/AUX/SLEEP/FLASH
#define STANDBY			0x0A //1 command "Enter into standby/low power loss"
#define NORMAL			0x0B //1 command "Normal working"
#define	RESET			0x0C //1 command "Reset module"
#define PLAYBACK		0x0D //1 command "Play"
#define	PAUSE			0x0E //1 command "Pause"
#define FOLDER  		0x0F //1 command "Play track in specified folder"
#define	VOL_ADJUST 		0x10 //1 command "Volume adjust set" {DH:Open volume adjust} {DL: set volume gain 0~31}
#define LOOP_ALL 		0x11 //1 command "Play loop all" {1:start repeat play} {0:stop play}2).
#define	MP3_FOLDER		0x12 //2 play specified track from MP3 folder	0-9999
#define	ADVERT 			0x13 //2 Commercials	0-9999
#define	FOLDER1000		0x14 //2 Allows you to play up to 1000 track from a single directory. Support 15 folder
#define	BACKGROUND		0x15 //2 stop playback, play background
#define	STOP			0x16 //2 Stop playback
#define FOLDER_LOOP		0x17 //4
#define SHUFFLE			0x18 //4
#define	SINGLE_REPEAT	0x19 //3	Set/unset current playing track in single repeat mode
#define DAC				0x1A //4	DAC Open/close
#define PLAYLIST		0x30 // Not supported by YX5200-24SS. Only works on YX6300-24SS
#define ADVERT2			0x25 //4 Multi-folder commercials command
#define	PLUG_IN 		0x3A //4
#define	PLUG_OUT 		0x3B //4
#define U_END_PLAY 		0x3C //2U device finished playing last track
#define TF_END_PLAY		0x3D //2TF device finished playing last track
#define	FLASH_END_PLAY 	0x3E //2STAY
#define	INIT 			0x3F //2Send initialization parameters 0 - 0x0F(each bit represent one device of the low-four bits)
#define	ERROR 			0x40 //2Returns an error, request retransmission
#define	REPLY 			0x41 //2Reply
#define	STATUS 			0x42 //2Query the current status
#define	QUERY_VOLUME	0x43 //2Query the current volume
#define	QUERY_EQ		0x44 //2Query the current EQ
#define	QUERY_PLAYMODE 	0x45 //2Query the current playback mode
#define	QUERY_VERSION 	0x46 //2Query the current software version
#define	QUERY_U_FILES 	0x47 //2Query the total number of TF card files
#define	QUERY_TF_FILES 	0x48 //2Query the total number of U-disk files
#define	QUERY_F_FILES	0x49 //2Query the total number of flash files
#define	QUERY_KEEPON 	0x4A //2Keep on
#define	QUERY_U_CUR		0x4B //2Queries the current track of U-Disk
#define	QUERY_TF_CUR 	0x4C //2Queries the current track of TF card
#define	QUERY_F_CUR 	0x4D //2Queries the current track of Flash
	/***************************************/

	/***
	 * Corrected value from personal tests
	 */

public:

	inline DFPlayer() {
		this->device = 1;  // TFCard by default
		this->noReceiveBit = false;
		this->fifoCount = 0;
		this->querying = false;
	}

	inline ~DFPlayer() {
		if (NULL != mp3Serial) {
			delete this->mp3Serial;
		}
	}

	inline DFPlayerSerial * getSerial() {
		return this->mp3Serial;
	}

	inline void setSerial(uint8_t receivePin, uint8_t transmitPin,
	bool inverse_logic = false) {

#ifdef DFPLAYER_INFO
		Serial.println(F("Connecting to DFPlayer..."));
#endif

		if (NULL != this->mp3Serial) {
			delete this->mp3Serial;
		}

		this->mp3Serial = new DFPlayerSerial(receivePin, transmitPin);

		this->mp3Serial->begin(9600);
		reset();
		while (not this->mp3Serial->available()) {
			//waiting for device to be ready
		}
		//if (this->mp3Serial->available()) {
#ifdef DFPLAYER_INFO
		Serial.println(F("Connected to DFPlayer !"));
#endif

		//}

	}

	inline uint8_t getDevice() const {
		return this->device;
	}

	inline void setDevice(uint8_t device) {
		this->device = device;
	}

	inline bool isNoReceiveBit() const {
		return this->noReceiveBit;
	}

	inline void setNoReceiveBit(bool noReceiveBit) {
		this->noReceiveBit = noReceiveBit;
	}

	inline uint8_t getFifoCount() const {
		return this->fifoCount;
	}

	inline uint8_t updateFifoCount() {
		this->fifoCount = mp3Serial->available();
		return this->fifoCount;
	}

	inline bool isQuerying() {
		return querying;
	}

	inline void setQuerying(bool querying) {
		this->querying = querying;
	}

	inline const uint8_t* getRecvBuffer() const {
		return this->recvBuffer;
	}

	inline void resetRecvBuffer() {
		for (uint8_t i = 0; i < DFPLAYER_BUFFER_LENGTH; i++) {
			this->recvBuffer[i] = 0x00;
		}
	}

	inline uint8_t* getSendBuffer() {
		return this->sendBuffer;
	}

	inline void setSendBuffer(uint8_t cmd, uint16_t bit6 = 0,
			uint16_t bit5 = 0) {
		if (not ((bit5 > 255) or (bit6 > 65535) or (bit6 > 255 and bit5 != 0))) {
			this->sendBuffer[0] = 0x7E; // start code
			this->sendBuffer[1] = 0xFF; // Version
			this->sendBuffer[2] = 0x06;	// Length (3+4+5+6+7+8)
			this->sendBuffer[3] = cmd; // Command
			this->sendBuffer[4] = this->noReceiveBit;	// Command feedback
			if (!(bit5 == 0 && bit6 > 255)) { //256 = 0x01 0x00
				this->sendBuffer[5] = bit5; // High data byte - Command Argument 1 (complement) or Argument 2
				this->sendBuffer[6] = bit6; // Low data byte Command Argument 1
			} else if (bit5 == 0 && bit6 > 255) {
				fillData(this->sendBuffer + 5, bit6);
			}
			this->sendBuffer[7] = 0x00; // Cheksum
			this->sendBuffer[8] = 0x00;	// Cheksum
			this->sendBuffer[9] = 0xEF; // End bit
			checksum();
		} else {
			/*
			 * We fill the sendBuffer with bullshit
			 * this will generate an error message on Serial
			 */
			for (uint8_t i = 0; i < DFPLAYER_BUFFER_LENGTH; i++) {
				this->sendBuffer[i] = 0x00;
			}

		}
	} //setSendBuffer

	/*
	 *calculates checksum : 0 - (sum of bits 1 to 6)
	 */
	inline void checksum() {
		uint16_t sum = 0;
		for (uint8_t i = 1; i < 7; i++) {
			sum += this->sendBuffer[i];
		}
		fillData(this->sendBuffer + 7, -sum);
	} //checksum

	/*
	 *
	 */
	inline void send() {

#ifdef DFPLAYER_HEAVY_DEBUG
		printSendBuffer();
#endif
		//	if (this->isEmptyQueue()) {
		this->mp3Serial->write(this->sendBuffer, DFPLAYER_BUFFER_LENGTH);
	//	delay(OPERATING_DELAY);

#ifdef DFPLAYER_DEBUG
		printHumanReadableSendBuffer();
#endif
	} //send

	/*
	 *
	 */
	inline void receive() {
		resetRecvBuffer();
		while (not isNoReceiveBit() and this->mp3Serial->available()
				and (this->mp3Serial->available() % DFPLAYER_BUFFER_LENGTH == 0)
		/*and this->recvBuffer[0] != 0x7E*/) {
			mp3Serial->readBytesUntil(0xEF, this->recvBuffer,
			DFPLAYER_BUFFER_LENGTH);
#ifdef DFPLAYER_DEBUG
			this->printHumanReadableRecvBuffer();
#endif
#ifdef DFPLAYER_HEAVY_DEBUG
			this->printRecvBuffer();
#endif
		}
		this->fifoCount = this->mp3Serial->available();
	} //receive

	inline void reset() {
		setSendBuffer(RESET);
		send();
		//delay(3000); //Mandatory !!
	}
	inline void setVolume(uint8_t value) {
		setSendBuffer(VOLUME, value);
		// this command never receive an answer !
		send();
	}

	inline void setEqualizer(uint8_t value) {
		//"Set Specified EQ" (0/1/2/3/4/5) Normal/Pop/Rock/Jazz/Classic/Base
		setSendBuffer(EQUALIZER, value);
		// this command never receive an answer !
		send();
	}

	inline void playTrackFromDir(uint8_t track, uint8_t folder) {
		setSendBuffer(FOLDER, track, folder);
		send();
	}
	inline void playTrackFromMP3Folder(uint16_t track) {
		setSendBuffer(MP3_FOLDER, track);
		send();
	}
	inline void playPhysicalTrack(uint16_t value = 0) {
		setSendBuffer(PLAY_TRACK, value);
		send();
	}

	inline void setSingleLoop(bool value) {
		//delay(OPERATING_DELAY);
		setSendBuffer(SINGLE_REPEAT, !value);  // !value o_0 ?
		// this command never receive an answer !
		send();
		//delay(OPERATING_DELAY);

	}

	inline void playSingleLoop(uint8_t track, uint8_t folder = 0) {
		setSendBuffer(PLAY_REPEAT, track, folder);
		send();
	}

	inline uint16_t getCurrentTrack() {
		while (this->querying) {
			//Wait for serial line to be ready
		}
		this->querying = true;
		uint8_t command;
		uint16_t result = 0;
		bool found = false;
		switch (this->device) {
		case 0: // U
			command = QUERY_U_CUR;
			break;
		case 1: // TFCard
			command = QUERY_TF_CUR;
			break;
		case 2: // SLEEP ????
			break;
		case 3: // FLASH
			command = QUERY_F_CUR;
			break;
		}

		setSendBuffer(command);
		send();

		while (this->mp3Serial->available()
		 and (this->mp3Serial->available() % DFPLAYER_BUFFER_LENGTH == 0)) {

			receive();

			result = 256 * this->recvBuffer[5] + this->recvBuffer[6];
#ifdef DFPLAYER_HEAVY_DEBUG
			Serial.print(F("DFPLAYER getCurrentTrack searching; command="));
			Serial.print(this->recvBuffer[3], HEX);
			Serial.print(F(" track="));
			Serial.print(result);
			Serial.print(F(" millis="));
			Serial.print(millis());
			Serial.print(F("ms fifoCount="));
			Serial.println(this->fifoCount);
			printRecvBuffer();
#endif
			if (this->recvBuffer[3] == command) {
				found = true;

#ifdef DFPLAYER_HEAVY_DEBUG
				Serial.print(F("DFPLAYER getCurrentTrack command found="));
				Serial.print(this->recvBuffer[3], HEX);
				Serial.print(F("! Will return ="));
				Serial.print(256 * this->recvBuffer[5] + this->recvBuffer[6]);
				Serial.print(F(" "));
				Serial.print(millis());
				Serial.print(F("ms fifoCount="));
				Serial.println(this->fifoCount);
#endif

			}

		}
		if (!found) {
#ifdef DFPLAYER_HEAVY_DEBUG
			Serial.println(
					F(
							"DFPLAYER getCurrentTrack didn't found matching receive code"));
#endif
			// returning bullshit
		}
		this->querying = false;
		return result;
	}

	inline void folderLoop(uint8_t folder) {
		setSendBuffer(FOLDER_LOOP, folder);
		send();
	}
	inline void loopAll(uint8_t track = 0, uint8_t folder = 0) {
		setSendBuffer(LOOP_ALL, track, folder);
		send();
	}
	inline void test(uint8_t track = 0, uint8_t folder = 0) {
		setSendBuffer(QUERY_KEEPON, track, folder);
		send();
	}

#ifdef DFPLAYER_HEAVY_DEBUG
	inline void printRecvBuffer() {
		Serial.print(F("DFPLAYER Received at "));
		Serial.print(millis());
		Serial.print(F("ms:\t"));
		for (uint8_t i = 0; i < DFPLAYER_BUFFER_LENGTH; i++) {
			Serial.print(this->recvBuffer[i], HEX);
			Serial.print(F(" "));
		}
		Serial.print(F("\tfifoCount="));
		Serial.print(this->fifoCount);
		Serial.println();

	}
	inline void printSendBuffer() {
		Serial.print(F("DFPLAYER Sending at "));
		Serial.print(millis());
		Serial.print(F("ms:\t"));
		for (uint8_t i = 0; i < DFPLAYER_BUFFER_LENGTH; i++) {
			Serial.print(this->sendBuffer[i], HEX);
			Serial.print(F(" "));
		}
		Serial.println();
	}

#endif
#ifdef DFPLAYER_DEBUG

	inline void printHumanReadableSendBuffer() {
// processing outgoing message
		uint8_t command = this->sendBuffer[3];

		uint8_t highBit = this->sendBuffer[5];
		uint8_t lowBit = this->sendBuffer[6];
		uint16_t data = 256 * this->sendBuffer[5] + this->sendBuffer[6];

		switch (command) {
			case SINGLE_REPEAT:
			if (!lowBit)
			Serial.println(F("\t\t\tDFPlayer: Enabling single play mode "));
			else
			Serial.println(F("\t\t\tDFPlayer: Disabling single play mode "));
			break;
			case FOLDER:
			Serial.print(F("\t\t\tDFPlayer: Require play of file number "));
			Serial.print(lowBit);
			Serial.print(F(" in directory number "));
			Serial.println(highBit);
			break;
			case VOLUME:
			Serial.print(F("\t\t\tDFPlayer: Setting volume to "));
			Serial.println(lowBit);
			break;
			case RESET:
			Serial.println(F("\t\t\tDFPlayer: Asking for Reset..."));
			break;
			case QUERY_TF_CUR:
			case QUERY_U_CUR:
			case QUERY_F_CUR:
			Serial.println(F("\t\t\tDFPlayer: Query current playing track... "));
			break;
			default:
			// this recvBuffer needs to be added to the switch/case scenario
			Serial.print(F("\t\t\tDFPlayer: Unhandled sended message: "));
			for (uint8_t i = 0; i < DFPLAYER_BUFFER_LENGTH; i++) {
				Serial.print(this->sendBuffer[i], HEX);
				Serial.print(F(" "));
			}

			break;
		}
	} // end printHumanReadableRecvBuffer
	inline void printHumanReadableRecvBuffer() {
// processing incoming message
		const char* errorText[] = {"Module busy", "Currently sleep mode",
			"Serial receiving error", "Checksum incorrect",
			"Specified track is out of current track scope",
			"Specified track is not found", "Advertise error",
			"SD card reading failed", "Entered into sleep mode"};

		uint8_t answer = this->recvBuffer[3];

		uint8_t highBit = this->recvBuffer[5];
		uint8_t lowBit = this->recvBuffer[6];
		uint16_t data = 256 * this->recvBuffer[5] + this->recvBuffer[6];

		switch (answer) {
			case ERROR:
			// error occurs
			Serial.print(F("\t\t\tDFPlayer: Error: "));
			Serial.println(errorText[lowBit - 1]);
			break;
			case PLUG_IN:

			if (lowBit == 2) { // sd card plugged in
				Serial.println(F("\t\t\tDFPlayer: SD card plugged in!"));
			}
			break;
			case PLUG_OUT:
			if (lowBit == 2) { // sd card plugged out
				Serial.println(F("\t\t\tDFPlayer: SD card plugged out!"));
			}
			break;
			case TF_END_PLAY:
			case U_END_PLAY:
			// track finished
			Serial.print(F("\t\t\tDFPlayer: Track finished playing: "));
			Serial.println(data);
			break;
			case INIT:
			if (lowBit == 2) { // sd card online (startup)
				Serial.println(F("\t\t\tDFPlayer: SD card online"));
			}
			break;
			case QUERY_TF_CUR:
			case QUERY_U_CUR:
			case QUERY_F_CUR:
			Serial.print(F("\t\t\tDFPlayer: Current track playing: "));
			Serial.println(data);
			break;
			default:
			// this recvBuffer needs to be added to the switch/case scenario
			Serial.print(F("\t\t\tDFPlayer: Unhandled received message: "));
			for (uint8_t i = 0; i < DFPLAYER_BUFFER_LENGTH; i++) {
				Serial.print(this->recvBuffer[i], HEX);
				Serial.print(F(" "));
			}

			break;
		}
	} // end printHumanReadableRecvBuffer
#endif

	/*
	 * IMPLEMENTED BUT NOT TESTED YET !!!
	 * Be Aware those functions may not work as intended
	 */
	inline void next() {
		setSendBuffer(NEXT);
		send();
	}
	inline void previous() {
		setSendBuffer(PREVIOUS);
		send();
	}

	inline void volumeUp() {
		setSendBuffer(VOL_UP);
		send();
	}
	inline void volumeDown() {
		setSendBuffer(VOL_DWN);
		send();
	}

	inline void play(uint16_t value = 0) {
		setSendBuffer(PLAYBACK, value);
		send();
	}
	inline void pause() {
		setSendBuffer(PAUSE);
		send();
	}
	inline void stop() {
		setSendBuffer(STOP);
		send();
	}
	inline uint8_t countTrackInDir(uint8_t folder) {
		setSendBuffer(MP3_FOLDER);
		send();
	}

	inline void playAdvert(uint16_t value) {
		setSendBuffer(ADVERT);
		send();
	}
	inline void getStatus() {
		setSendBuffer(STATUS);
		send();
	}
	inline uint16_t countTotalFiles() {
		uint8_t command;
		switch (this->device) {
		case 0: // U
			command = QUERY_U_FILES;
			break;
		case 1: // TFCard
			command = QUERY_TF_FILES;
			break;
		case 2: // SLEEP ????
			break;
		case 3: // FLASH
			command = QUERY_F_FILES;
			break;
		}

		setSendBuffer(command);
		send();
	}

private:

//Puts uint16_bigend at the specified memory address
	inline void fillData(uint8_t *buffer, uint16_t data) {
		*buffer = (uint8_t) (data >> 8);
		*(buffer + 1) = (uint8_t) data;
	}

	DFPlayerSerial * mp3Serial;
	uint8_t sendBuffer[10];
	uint8_t recvBuffer[10];
	uint8_t device;
	uint8_t fifoCount;bool noReceiveBit;
 bool querying;
};
#endif // _DFPLAYER_H_
