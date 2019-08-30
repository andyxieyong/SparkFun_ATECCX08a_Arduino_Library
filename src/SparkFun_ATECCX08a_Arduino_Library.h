/*
  This is a library written for the ATECCX08A Criptographic Co-Processor (QWIIC).

  Written by Pete Lewis @ SparkFun Electronics, August 5th, 2019

  The IC uses I2C and 1-wire to communicat. This library only supports I2C.

  https://github.com/sparkfun/SparkFun_ATECCX08A_Arduino_Library

  Do you like this library? Help support SparkFun. Buy a board!

  Development environment specifics:
  Arduino IDE 1.8.1

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
//Teensy
#include "i2c_t3.h"

#else
#include "Wire.h"

//The catch-all default is 32
#define I2C_BUFFER_LENGTH 32

#endif

#define ATECC508A_ADDRESS_DEFAULT 0x60 //7-bit unshifted default I2C Address
// 0x60 on a fresh chip. note, this is software definable

// WORD ADDRESS VALUES
// These are sent in any write sequence to the IC.
// They tell the IC what we are going to do: Reset, Sleep, Idle, Command.
#define WORD_ADDRESS_VALUE_COMMAND 	0x03	// This is the "command" word address, 
//this tells the IC we are going to send a command, and is used for most communications to the IC
#define WORD_ADDRESS_VALUE_IDLE 0x02 // used to enter idle mode

// COMMANDS (aka "opcodes" in the datasheet)
#define COMMAND_OPCODE_INFO 	0x30 // Return device state information.
#define COMMAND_OPCODE_LOCK 	0x17 // Lock configuration and/or Data and OTP zones
#define COMMAND_OPCODE_RANDOM 	0x1B // Create and return a random number (32 bytes of data)
#define COMMAND_OPCODE_READ 	0x02 // Return data at a specific zone and address.
#define COMMAND_OPCODE_WRITE 	0x12 // Return data at a specific zone and address.
#define COMMAND_OPCODE_SHA 		0x47 // Computes a SHA-256 or HMAC/SHA digest for general purpose use by the system.
#define COMMAND_OPCODE_GENKEY 	0x40 // Creates a key (public and/or private) and stores it in a memory key slot
#define COMMAND_OPCODE_NONCE 	0x16 // 

// Lock command PARAM1 zone options (aka Mode). more info at table on datasheet page 75
#define LOCK_ZONE_CONFIG 			0b10000000
#define LOCK_ZONE_DATA_AND_OTP 		0b10000001

// GenKey command PARAM1 zone options (aka Mode). more info at table on datasheet page 71
#define GENKEY_MODE_PUBLIC 			0b00000000
#define GENKEY_MODE_PRIVATE 		0b00001000

#define ZONE_CONFIG 0x00
#define ZONE_OTP 0x01
#define ZONE_DATA 0x02

#define ADDRESS_CONFIG_BLOCK_0 0b00000000 // param2 (byte 0), address block bits: _ _ _ 0  0 _ _ _ 
#define ADDRESS_CONFIG_BLOCK_1 0b00001000 // param2 (byte 0), address block bits: _ _ _ 0  1 _ _ _ 
#define ADDRESS_CONFIG_BLOCK_2 0b00010000 // param2 (byte 0), address block bits: _ _ _ 1  0 _ _ _ 
#define ADDRESS_CONFIG_BLOCK_3 0b00011000 // param2 (byte 0), address block bits: _ _ _ 1  1 _ _ _ 

class ATECCX08A {
  public:
    //By default use Wire, standard I2C speed, and the default ADS1015 address
	#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
	//Teensy
	boolean begin(uint8_t i2caddr = ATECC508A_ADDRESS_DEFAULT, i2c_t3 &wirePort = Wire);
	#else
	boolean begin(uint8_t i2caddr = ATECC508A_ADDRESS_DEFAULT, TwoWire &wirePort = Wire);
	#endif
	
	byte inputBuffer[128]; // used to store messages received from the IC as they come in
	byte configZone[128]; // used to store configuration zone bytes read from device EEPROM
	boolean receiveResponseData(uint8_t length = 0, boolean debug = false);
	boolean checkCount(boolean debug = false);
	boolean checkCrc(boolean debug = false);
	uint8_t countGlobal = 0; // used to add up all the bytes on a long message. Important to reset before each new receiveMessageData();
	void cleanInputBuffer();
	
	boolean wakeUp();
	void idleMode();
	boolean getInfo();
	boolean lockConfig(); // note, this PERMINANTLY disables changes to config zone - including changing the I2C address!
	boolean lockDataAndOTP();
	boolean lock(byte zone);
	
	// Random array and fuctions
	byte random32Bytes[32]; // used to store the complete data return (32 bytes) when we ask for a random number from chip.
	boolean updateRandom32Bytes(boolean debug = false);
	byte getRandomByte(boolean debug = false);
	int getRandomInt(boolean debug = false);
	long getRandomLong(boolean debug = false);
	
	uint8_t crc[2] = {0, 0};
	void atca_calculate_crc(uint8_t length, uint8_t *data);	
	
	// Key functions
	boolean generatePublicKey(byte slot = 0);
	boolean readKeySlot(byte slot = 0);
	boolean storeKeyInSlot(byte slot = 0);
	boolean createMAC(uint8_t *message, uint8_t *generatedMAC);
	boolean verifyMAC(uint8_t *message, uint8_t *receivedMAC);

	boolean read(byte zone, byte address, byte length = 4, boolean debug = false);
	boolean write(byte zone, byte address, byte length, const byte data[]);

	boolean readConfigZone(boolean debug = false);
	boolean nOnce(boolean debug = false);
	
  private:

	#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
	//Teensy
	i2c_t3 *_i2cPort;
	
	#else
	
	TwoWire *_i2cPort;
	
	#endif

	uint8_t _i2caddr;
	
    boolean _printDebug = false; //Flag to print the serial commands we are sending to the Serial port for debug

    Stream *_debugSerial; //The stream to send debug messages to if enabled
};

