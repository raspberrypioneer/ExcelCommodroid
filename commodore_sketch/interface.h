#ifndef INTERFACE_H
#define INTERFACE_H

// Enable this for verbose logging of IEC and CBM interfaces.
//#define CONSOLE_DEBUG

#include "iec_driver.h"
#include "cbmdefines.h"

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
	void saveFile();
	void sendFile();
	void sendListing();
	bool removeFilePrefix(void);
	void sendLine(byte len, char* text, word &basicPtr);

	// handler helpers
	void handleATNCmdCodeOpen(IEC::ATNCmd &cmd);
	void handleATNCmdCodeDataTalk(byte chan);
	void handleATNCmdCodeDataListen();
	void handleATNCmdClose();
	void epyxFastloadProgram();
	void epyxFastloadROM();

	// our iec low level driver:
	IEC& m_iec;

	// atn command buffer struct
	IEC::ATNCmd& m_cmd;

};

#endif
