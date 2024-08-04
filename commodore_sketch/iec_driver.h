#ifndef IEC_DRIVER_H
#define IEC_DRIVER_H

// Enable this to debug the IEC lines (checking soldering and physical connections). See project README.TXT
//#define DEBUGLINES

#include <Arduino.h>
#include "cbmdefines.h"

class IEC
{
public:

	enum IECState {
		noFlags   = 0,
		eoiFlag   = (1 << 0),   // might be set by Iec_receive
		atnFlag   = (1 << 1),   // might be set by Iec_receive
		errorFlag = (1 << 2)  // If this flag is set, something went wrong
	};

	// Return values for checkATN:
	enum ATNCheck {
		ATN_IDLE = 0,       // Nothing received of our concern
		ATN_CMD = 1,        // A command is received
		ATN_CMD_LISTEN = 2, // A command is received and data is coming to us
		ATN_CMD_TALK = 3,   // A command is received and we must talk now
		ATN_ERROR = 4,      // A problem occoured, reset communication
		ATN_RESET = 5       // The IEC bus is in a reset state (RESET line)
	};

	// IEC ATN commands:
	enum ATNCommand {
		ATN_CODE_LISTEN = 0x20,
		ATN_CODE_TALK = 0x40,
		ATN_CODE_DATA = 0x60,
		ATN_CODE_CLOSE = 0xE0,
		ATN_CODE_OPEN = 0xF0,
		ATN_CODE_UNLISTEN = 0x3F,
		ATN_CODE_UNTALK = 0x5F
	};

	// ATN command struct maximum command length:
	enum {
		ATN_CMD_MAX_LENGTH = 40
	};
	// default device number listening unless explicitly stated in ctor:
	enum {
		DEFAULT_IEC_DEVICE = 8
	};

	typedef struct _tagATNCMD {
		byte code;
		byte str[ATN_CMD_MAX_LENGTH];
		byte strLen;
	} ATNCmd;

	IEC(byte deviceNumber = DEFAULT_IEC_DEVICE);
	~IEC()
	{ }

	// Initialise iec driver
	//
	boolean init();

	// Checks if CBM is sending an attention message. If this is the case,
	// the message is received and stored in atn_cmd.
	//
	ATNCheck checkATN(ATNCmd& cmd);

	// Sends a byte. The communication must be in the correct state: a load command
	// must just have been received. If something is not OK, FALSE is returned.
	//
	boolean send(byte data);

	// Same as IEC_send, but indicating that this is the last byte.
	//
	boolean sendEOI(byte data);

	// A special send command that informs file not found condition
	//
	boolean sendFNF();

	// Receives a byte
	//
	byte receive();

	byte deviceNumber() const;
	void setDeviceNumber(const byte deviceNumber);
	void setPins(byte atn, byte clock, byte data, byte reset);
	IECState state() const;

	//Needed for epyx fastload
	void setClock(boolean state);
	void setData(boolean state);
	byte getATN();
	byte getData();

	//Read byte using gijoe protocol
	int16_t gijoe_read_byte(void);

#ifdef DEBUGLINES
	unsigned long m_lastMillis;
	void testINPUTS();
	void testOUTPUTS();
#endif

private:
	byte timeoutWait(byte waitBit, boolean whileHigh);
	byte receiveByte(void);
	boolean sendByte(byte data, boolean signalEOI);
	boolean turnAround(void);
	boolean undoTurnAround(void);

	// false = LOW, true == HIGH
	inline boolean readPIN(byte pinNumber)
	{
		// To be able to read line we must be set to input, not driving.
		pinMode(pinNumber, INPUT);
		return digitalRead(pinNumber) ? true : false;
	}

	inline boolean readATN()
	{
		return readPIN(m_atnPin);
	}

	inline boolean readDATA()
	{
		return readPIN(m_dataPin);
	}

	inline boolean readCLOCK()
	{
		return readPIN(m_clockPin);
	}

	inline boolean readRESET()
	{
		return !readPIN(m_resetPin);
	}

	// true == PULL == HIGH, false == RELEASE == LOW
	inline void writePIN(byte pinNumber, boolean state)
	{
		pinMode(pinNumber, state ? OUTPUT : INPUT);
		digitalWrite(pinNumber, state ? LOW : HIGH);
	}

	inline void writeATN(boolean state)
	{
		writePIN(m_atnPin, state);
	}

	inline void writeDATA(boolean state)
	{
		writePIN(m_dataPin, state);
	}

	inline void writeCLOCK(boolean state)
	{
		writePIN(m_clockPin, state);
	}

	// communication must be reset
	byte m_state;
	byte m_deviceNumber;

	byte m_atnPin;
	byte m_dataPin;
	byte m_clockPin;
	byte m_resetPin;
};

#endif
