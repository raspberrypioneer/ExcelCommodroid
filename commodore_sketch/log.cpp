#include "log.h"

#ifdef LOG_ENABLED

#include <stddef.h>

void Log(byte severity, char facility, char* msg)
{

	char strBuf[64];
  sprintf_P(strBuf, (PGM_P)F("D%c%c%s"), pgm_read_byte(severity), facility, msg);
	COMPORT.println(strBuf);

} // Log

#endif
