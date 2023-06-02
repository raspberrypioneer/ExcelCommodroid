#ifndef INTERFACE_H
#define INTERFACE_H

// This should be defined if the RESET line is soldered in the IEC DIN connector. When defined it will let the
// arduino to go into a reset state and wait for the CBM to become ready for communiction.
#define HAS_RESET_LINE

// Define this for speed increase when reading (filling serial buffer while transferring
// to CBM without interrupts off). It is experimental, stability needs to be checked
// further even though it seems to work just fine.
//#define EXPERIMENTAL_SPEED_FIX

// Enable this for verbose logging of IEC and CBM interfaces.
#define CONSOLE_DEBUG

#include "iec_driver.h"
#include "cbmdefines.h"

enum OpenState {
	O_NOTHING,			// Nothing to send / File not found error
	O_INFO,					// User issued a reload sd card
	O_FILE,					// A program file is opened
	O_DIR,					// A listing is requested
	O_FILE_ERR,			// Incorrect file format opened
	O_SAVE_REPLACE	// Save-with-replace is requested
};

// The base pointer of basic.
#define C64_BASIC_START 0x0801

class Interface
{
public:
	Interface(IEC& iec);
	virtual ~Interface() {}

	// The handler returns the current IEC state, see the iec_driver.hpp for possible states.
	byte handler(void);

private:
	void reset(void);
	void saveFile();
	void sendFile();
	void sendListing();
	void sendStatus(void);
	bool removeFilePrefix(void);
	void sendLine(byte len, char* text, word &basicPtr);

	// handler helpers.
	void handleATNCmdCodeOpen(IEC::ATNCmd &cmd);
	void handleATNCmdCodeDataTalk(byte chan);
	void handleATNCmdCodeDataListen();
	void handleATNCmdClose();

	// our iec low level driver:
	IEC& m_iec;
	// This var is set after an open command and determines what to send next
	byte m_openState;			// see OpenState
	byte m_queuedError;

	// atn command buffer struct
	IEC::ATNCmd& m_cmd;

};

#endif
