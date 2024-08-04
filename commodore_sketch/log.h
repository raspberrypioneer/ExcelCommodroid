#ifndef LOG_H
#define LOG_H

// Comment out for no logging to host
//#define LOG_ENABLED
#ifdef LOG_ENABLED

inline void Log(char* msg)
{
    Serial.print("D:");  //D for debug
    Serial.println(msg);
}

#else

#define Log(msg)

#endif // LOG_ENABLED

#endif
