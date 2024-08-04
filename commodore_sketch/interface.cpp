#include <string.h>
#include "interface.h"
#include "atomic.h"
#include "epyxfastload.h"
#include "log.h"

//Retrieve programs to fastload using serial or included binary rom file
#define USE_SERIAL
//#define USE_ROM

#ifdef USE_ROM
#include "incbin.h"
INCBIN(ROMFile, "Pengo.prg");
#endif

using namespace CBM;

namespace {

// Buffer for incoming and outgoing serial bytes and other stuff.
char serCmdIOBuf[MAX_BYTES_PER_REQUEST];

} // unnamed namespace


Interface::Interface(IEC& iec)
	: m_iec(iec)
	// NOTE: Householding with RAM bytes: We use the middle of serial buffer for the ATNCmd buffer info.
	// This is ok and won't be overwritten by actual serial data from the host, this is because when this ATNCmd data is in use
	// only a few bytes of the actual serial data will be used in the buffer.
	, m_cmd(*reinterpret_cast<IEC::ATNCmd*>(&serCmdIOBuf[sizeof(serCmdIOBuf) / 2]))
{}


// send single basic line, including heading basic pointer and terminating zero.
void Interface::sendLine(byte len, char* text, word& basicPtr)
{
	// Increment next line pointer
	basicPtr += len + 5 - 2;  // note: minus two here because the line number is included in the array already

	// Send that pointer
	m_iec.send(basicPtr bitand 0xFF);
	m_iec.send(basicPtr >> 8);

	// Send line contents
	for(byte i = 0; i < len; i++)
		m_iec.send(text[i]);

	// Finish line
	m_iec.send(0);
} // sendLine


void Interface::sendListing()
{
	uint8_t bufLen, bytesRead, bufEnd;
    boolean firstLine = true;

	// Reset basic memory pointer:
	word basicPtr = C64_BASIC_START;

	do {
		//Serial read buffer will be populated in response to the first directory request and subsequent read requests using 'L'
		Serial.readBytes(serCmdIOBuf, 2);
		bufEnd = serCmdIOBuf[0];
		bufLen = serCmdIOBuf[1];
		if (bufLen > 0) {
			bytesRead = Serial.readBytes(serCmdIOBuf, bufLen);
			noInterrupts();
			if (firstLine) {  // Send load address
				m_iec.send(C64_BASIC_START bitand 0xff);
				m_iec.send((C64_BASIC_START >> 8) bitand 0xff);
				firstLine = false;
			}
			sendLine(bufLen, serCmdIOBuf, basicPtr);
			interrupts();
		}
		if (bufEnd == 'L') {  //'Normal' directory line types received are 'L', except for the last one 'l'
			Serial.println('L');  //Request another directory line
		}
	} while (bufEnd == 'L');  //Continue while 'normal' directory lines are being returned

	// End program with two zeros after last line. Last zero goes out as EOI.
	noInterrupts();
	m_iec.send(0);
	m_iec.sendEOI(0);
	interrupts();

} // sendListing


void Interface::sendFile()
{
	uint8_t bufLen, bytesRead, bufEnd, i;
	bool ok = true;

	do {
	
		//Serial read buffer will be populated in response to the first open file request and subsequent read requests using 'R'
		bytesRead = Serial.readBytes(serCmdIOBuf, 2); // read the ack type ('B' or 'E') and length
		bufEnd = serCmdIOBuf[0];
		bufLen = serCmdIOBuf[1];
		bytesRead = Serial.readBytes(serCmdIOBuf, bufLen); // read the program data bytes

		//Ask for more bytes from the PC now, then send the current buffer load to the C64
#if !defined(__AVR_ATmega328P__)  //Not suitable for Arduino uno
		if (bufEnd != 'E') Serial.println('R');
#endif

		for (i = 0; i < bufLen and ok; i++) {
			noInterrupts();
			if (bufEnd == 'E' and i == bufLen - 1)
				ok = m_iec.sendEOI(serCmdIOBuf[i]);  // indicate end of file.
			else
				ok = m_iec.send(serCmdIOBuf[i]);
			interrupts();
		}

		if (!ok) {
			sprintf_P(serCmdIOBuf, (PGM_P)F("sendFile send bytes problem: %u"), i);
			Log(serCmdIOBuf);
		}

#if defined(__AVR_ATmega328P__)  //Suitable for Arduino uno
      	if (bufEnd != 'E') Serial.println('R');
#endif

	} while (bufEnd == 'B' and ok); // keep asking for more as long as we don't get the 'E' or something else (indicating out of sync).

	if (ok) {
		Log("sendFile completed");
	}
	else {
		while (Serial.available())  //Flush out read buffer
			Serial.read();
	}

} // sendFile


void Interface::saveFile()
{
	boolean done = false;
	do {

		// Receive bytes from Commodore until EOI detected
		uint8_t bufLen = 2;  //Allow for 'W' and length prefix bytes
		do {
			noInterrupts();
			serCmdIOBuf[bufLen++] = m_iec.receive();
			interrupts();
			done = (m_iec.state() bitand IEC::eoiFlag) or (m_iec.state() bitand IEC::errorFlag);
		} while ((bufLen < 0xf0) and not done);

		// Send the bytes onto the PC
		serCmdIOBuf[0] = 'W';
		serCmdIOBuf[1] = bufLen;
		Serial.write((const byte*)serCmdIOBuf, bufLen);
		Serial.println();
		Serial.flush();

	} while (not done);

} // saveFile


byte Interface::handler(void)
{
	noInterrupts();
	IEC::ATNCheck retATN = m_iec.checkATN(m_cmd);
	interrupts();

	if(retATN == IEC::ATN_ERROR) {
		strcpy_P(serCmdIOBuf, (PGM_P)F("ATNCMD: IEC_ERROR!"));
		Log(serCmdIOBuf);
	}

	// Did anything happen from the host side?
	else if(retATN not_eq IEC::ATN_IDLE) {
		// A command is received, make cmd string null terminated
		m_cmd.str[m_cmd.strLen] = '\0';
#ifdef CONSOLE_DEBUG
		{
			sprintf_P(serCmdIOBuf, (PGM_P)F("ATN code:%d cmd: %s (len: %d) retATN: %d"), m_cmd.code, m_cmd.str, m_cmd.strLen, retATN);
			Log(serCmdIOBuf);
		}
#endif

		// lower nibble is the channel.
		byte chan = m_cmd.code bitand 0x0F;

		// check upper nibble, the command itself.
		switch(m_cmd.code bitand 0xF0) {
			case IEC::ATN_CODE_OPEN:
				// Open either file or prg for reading, writing or single line command on the command channel.
				// In any case we just issue an 'OPEN' to the host and let it process.
				// Note: Some of the host response handling is done LATER, since we will get a TALK or LISTEN after this.
				// Also, simply issuing the request to the host and not waiting for any response here makes us more
				// responsive to the CBM here, when the DATA with TALK or LISTEN comes in the next sequence.
				handleATNCmdCodeOpen(m_cmd);
			break;

			case IEC::ATN_CODE_DATA:  // data channel opened
				if(retATN == IEC::ATN_CMD_TALK) {
					 // when the CMD channel is read (status), we first need to issue the host request. The data channel is opened directly.
					if(CMD_CHANNEL == chan)
						handleATNCmdCodeOpen(m_cmd);  // Typically an empty command for channel 15 message to Commodore
					handleATNCmdCodeDataTalk(chan);  // Talk to Commodore, sending file and listing data
				}
				else if(retATN == IEC::ATN_CMD_LISTEN) {
					handleATNCmdCodeDataListen();  // Listen for commands / data from the Commodore e.g. save data
				}
				else if(retATN == IEC::ATN_CMD) { // Here we are sending a command to PC and executing it, but not sending response
					if (CMD_CHANNEL == chan) {

						//Check if received M-E for semi-fast / gijoe mode, proceeding to epyx fastload
						if(strcmp(m_cmd.str,"M-E\xa9\x01\r") == 0) {
#ifdef USE_SERIAL
							epyxFastloadProgram();
#endif
#ifdef USE_ROM
							epyxFastloadROM();
#endif
						}
					}
					else
						handleATNCmdCodeOpen(m_cmd);	// back to CBM, the result code of the command is however buffered on the PC side.
				}
				break;

			case IEC::ATN_CODE_CLOSE:
				// handle close with host.
				handleATNCmdClose();
				break;

			case IEC::ATN_CODE_LISTEN:
				//Log("LISTEN");
				break;
			case IEC::ATN_CODE_TALK:
				//Log("TALK");
				break;
			case IEC::ATN_CODE_UNLISTEN:
				//Log("UNLISTEN");
				break;
			case IEC::ATN_CODE_UNTALK:
				//Log("UNTALK");
				break;
			default:
				if(retATN == IEC::ATN_CMD_TALK) {
					//Needed to handle epyx fastload cartridge shortcut dir command (type $ on C64)
					handleATNCmdCodeDataTalk(chan);  // Talk to Commodore, sending file and listing data
				}
				break;
		} // switch
	} // IEC not idle

	return retATN;
} // handler


#ifdef USE_SERIAL
//Open the program and fastload to C64 with data sent serially from PC
void Interface::epyxFastloadProgram()
{
	uint8_t bufLen, bytesRead, bufEnd, i, b;
	uint8_t checksum = 0;
	int16_t j;

	//Switchover to full epyx fastload via semi-fastload gijoe protocol

	//Initial handshake
	noInterrupts();
	m_iec.setData(true);
	m_iec.setClock(false);

	while(m_iec.getData() != false)
	m_iec.setClock(true);
	interrupts();

	//Receive and checksum stage 2
	for (j=0; j<256; j++) {
		noInterrupts();
		b = m_iec.gijoe_read_byte();
		interrupts();
		if (b < 0) {
			Log("epyxFastloadProgram, read checksum error");
			return;
		}

		if (j < 237)
		//Stage 2 has some junk bytes at the end, ignore them
		checksum ^= b;
	}

	//Check for known stage2 loaders
	if (checksum != 0x91 && checksum != 0x5b) {
		Log("epyxFastloadProgram, checksum total error");
	}

	//Receive file name length
	noInterrupts();
	j = m_iec.gijoe_read_byte();
	interrupts();
	if (j < 0) {
		Log("epyxFastloadProgram, file length error");
	}
	bufLen = j + 3;  //Allow for 'O' (open), file/command length and channel

	//Receive file name and build open command
	serCmdIOBuf[0] = 'O';  //Open instruction
	serCmdIOBuf[1] = bufLen;
	serCmdIOBuf[2] = IEC::ATN_CODE_OPEN bitand 0xF;  //channel
	do {
		noInterrupts();
		b = m_iec.gijoe_read_byte();
		interrupts();
		if (b < 0) {
			Log("epyxFastloadProgram, file name error");
			break;
		}

		serCmdIOBuf[--bufLen] = b;
	} while (bufLen > 3);  //Preserve first 3 bytes of serCmdIOBuf 

	m_iec.setClock(true);

	//Request file open from PC which then returns a buffer load of data
	Serial.write((const byte*)serCmdIOBuf, serCmdIOBuf[1]);  //send instruction to PC
	Serial.println();

	// Transfer data via full epyx fastload protocol
	do {

		ATOMIC_BLOCK(ATOMIC_FORCEON) {
			m_iec.setClock(true);
			m_iec.setData(true);
		}

		bytesRead = Serial.readBytes(serCmdIOBuf, 2); // read the ack type usually B/E or X if error
		bufEnd = serCmdIOBuf[0];
		bufLen = serCmdIOBuf[1];
		if (bufEnd == 'X') {  //
			m_iec.sendFNF();  //Error, return file not found on Commodore
			break;
		}
		bytesRead = Serial.readBytes(serCmdIOBuf, bufLen); // read the program data bytes

		//Ask for more bytes from the PC now, then send the current buffer load to the C64
#if !defined(__AVR_ATmega328P__)  //Not suitable for Arduino uno
		if (bufEnd != 'E') Serial.println('R');
#endif
		//Send the program data bytes via epyx fastload protocol
		ATOMIC_BLOCK(ATOMIC_FORCEON) {

			m_iec.setClock(true);
			m_iec.setData(true);

			// send number of bytes in sector
			if (asm_epyxcart_send_byte(bufLen)) {
				Log("epyxFastloadProgram, length fail");
				break;
			}

			// send data
			for (i=0; i<bufLen; i++) {
				if (asm_epyxcart_send_byte(serCmdIOBuf[i])) {
					Log("epyxFastloadProgram, send byte fail");
					break;
				}
			}

			// check ATN ok
			if (m_iec.getATN() == false) {
				Log("epyxFastloadProgram, ATN false");
				bufEnd = 'E';
				break;
			}

		}
#if defined(__AVR_ATmega328P__)  //Suitable for Arduino uno
      	if (bufEnd != 'E') Serial.println('R');
#endif

	} while (bufEnd == 'B');

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		m_iec.setClock(true);
		m_iec.setData(true);
	}

}  // epyxFastloadProgram
#endif

#ifdef USE_ROM
//Open the ROM program and fastload to C64
void Interface::epyxFastloadROM()
{
	uint8_t b;
	uint8_t checksum = 0;
	int16_t bufLen, bufEnd, pos, i, j;
	PGM_P binFile = reinterpret_cast<PGM_P>(gROMFileData);

	//Switchover to full epyx fastload via semi-fastload gijoe protocol

	//Initial handshake
	noInterrupts();
	m_iec.setData(true);
	m_iec.setClock(false);

	while(m_iec.getData() != false)
	m_iec.setClock(true);
	interrupts();

	//Receive and checksum stage 2
	for (j=0; j<256; j++) {
		noInterrupts();
		b = m_iec.gijoe_read_byte();
		interrupts();
		if (b < 0) {
			Log("epyxFastloadROM, read checksum error");
			return;
		}

		if (j < 237)
		//Stage 2 has some junk bytes at the end, ignore them
		checksum ^= b;
	}

	//Check for known stage2 loaders
	if (checksum != 0x91 && checksum != 0x5b) {
		Log("epyxFastloadROM, checksum total error");
	}

	//Receive file name length
	noInterrupts();
	j = m_iec.gijoe_read_byte();
	interrupts();
	if (j < 0) {
		Log("epyxFastloadROM, file length error");
	}

	//Read the file name bytes, not needed for this ROM version
	do {
		noInterrupts();
		b = m_iec.gijoe_read_byte();
		interrupts();
		if (b < 0) {
			Log("epyxFastloadROM, file name error");
			break;
		}

		j--;
	} while (j > 0);

	m_iec.setClock(false);

	pos = 0;
    bufLen = min(gROMFileSize, MAX_BYTES_PER_REQUEST-2);

	// Transfer data via full epyx fastload protocol
	ATOMIC_BLOCK(ATOMIC_FORCEON) {

		while (1) {
			m_iec.setClock(true);
			m_iec.setData(true);

			// send number of bytes in sector
			if (asm_epyxcart_send_byte(lowByte(bufLen))) {  //Just the low byte of length is needed (is max 255 $FF)
				Log("epyxFastloadROM, length fail");
				break;
			}

			for (i=0; i<bufLen; i++) {
				if (asm_epyxcart_send_byte(pgm_read_byte(binFile++))) {
					Log("epyxFastloadROM, send byte fail");
					break;
				}
			}

			// check ATN ok
			if (m_iec.getATN() == false) {
				Log("epyxFastloadROM, ATN false");
				break;
			}

			// exit after final sector
			pos+=bufLen;
			if(pos >= gROMFileSize) {
				break;
    		}

			// read next sector
			m_iec.setClock(false);
			bufLen = min((gROMFileSize - pos), MAX_BYTES_PER_REQUEST-2);

		}  // while
	}  // atomic

	m_iec.setClock(true);
	m_iec.setData(true);

}  // epyxFastloadROM
#endif


//Open file section
//cmd is a type
// code is 2 nibbles upper = command, lower = channel
//   upper should be ATN_CODE_OPEN (value 240), lower should be read (value 0), i.e. set to ATN_CODE_OPEN
// str will be the file name
// strlen is the length of the file name string
void Interface::handleATNCmdCodeOpen(IEC::ATNCmd& cmd)
{
	uint8_t bufLen = 3;  //Allow for 'O' (open), file/command length and channel

	serCmdIOBuf[0] = 'O';
	serCmdIOBuf[2] = cmd.code bitand 0xF;  //channel
	memcpy(&serCmdIOBuf[bufLen], cmd.str, cmd.strLen);
	bufLen += cmd.strLen;
	serCmdIOBuf[1] = bufLen;  //file/command length
	Serial.write((const byte*)serCmdIOBuf, bufLen);  //send instruction to PC
	Serial.println();

} // handleATNCmdCodeOpen


// Handle open command response and talk to Commodore, sending file and listing data
void Interface::handleATNCmdCodeDataTalk(byte chan)
{
	while (!Serial.available());  // Wait for a response from the PC
	char r = Serial.peek();  // Peek at response (this leaves the byte in the serial buffer)
	switch(r) {
	case 'B': case 'E':
		sendFile();  //Load program on Commodore
		break;

	case 'L': case 'l':
		sendListing();  //Directory listing on Commodore
		break;

	default:
		uint8_t bytesRead = Serial.readBytes(serCmdIOBuf, 2); // read the two error bytes
		m_iec.sendFNF();  //Error, return file not found on Commodore

	}

} // handleATNCmdCodeDataTalk


// Listen for commands / data from the Commodore
void Interface::handleATNCmdCodeDataListen()
{
	while (!Serial.available());  // Wait for a response from the PC
	char r = Serial.peek();  // Peek at response (this leaves the byte in the serial buffer)
	if (r == 'W') {  // Peek at response (this leaves the byte in the serial buffer)
		saveFile();
	}
	else {
		m_iec.sendFNF();
	}

} // handleATNCmdCodeDataListen


void Interface::handleATNCmdClose()
{
	uint8_t bufLen, bytesRead, bufEnd;

	Serial.println('C');  //Tell PC to close the file
	Serial.readBytes(serCmdIOBuf, 2);
	bufEnd = serCmdIOBuf[0];
	bufLen = serCmdIOBuf[1];
	bytesRead = Serial.readBytes(serCmdIOBuf, bufLen);

} // handleATNCmdClose
