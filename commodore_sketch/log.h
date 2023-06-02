#ifndef LOG_H
#define LOG_H

// Comment out for no logging to host
//#define LOG_ENABLED
#ifdef LOG_ENABLED

#include <Arduino.h>
#include "global_defines.h"

#define FAC_MAIN 'M'
#define FAC_IEC 'I'
#define FAC_IFACE 'F'

enum Severity { Success, Information, Warning, Error };
void Log(byte severity , char facility, char* msg);

#else

#define Log(severity, facility, msg)

#endif // LOG_ENABLED

#endif
