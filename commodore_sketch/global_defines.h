#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) \
	|| defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__)\
	|| defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
#define COMPORT Serial2
#else
#define COMPORT Serial
#endif

// Comment out if never using datasette
//#define DATASETTE_ENABLED
