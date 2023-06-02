#include "global_defines.h"
#include "log.h"
#include "iec_driver.h"
#include "interface.h"

#ifdef DATASETTE_ENABLED
#include "datasette.h"
#endif

#define DEFAULT_BAUD_RATE 115200
#define SERIAL_TIMEOUT_MSECS 1000

#define HANDSHAKE_READY "<CON>\r"
#define HANDSHAKE_SEND "<AOK>"
#define HANDSHAKE_OK "<END>\r"

#define CONNECT_BLINKS 2
#define CBM_DISKDRIVE 0

static IEC iec(8);
static Interface iface(iec);

#ifdef DATASETTE_ENABLED
static Datasette cassette;
#endif

unsigned mode, deviceNumber, atnPin, clockPin, dataPin, resetPin, motorPin, readPin, writePin, sensePin;

void setup()
{

  //Initialize serial and wait for port to open
  COMPORT.begin(DEFAULT_BAUD_RATE);
  while (!COMPORT);
  COMPORT.setTimeout(SERIAL_TIMEOUT_MSECS);

  connectMediaHost();

#ifdef DATASETTE_ENABLED
  if (mode == CBM_DISKDRIVE) {
    iec.setDeviceNumber(deviceNumber);
    iec.setPins(atnPin, clockPin, dataPin, resetPin);
    iec.init();
  }
  else {
    cassette.setPins(motorPin, readPin, writePin, sensePin);
    cassette.init();
  }
#else
  iec.setDeviceNumber(deviceNumber);
  iec.setPins(atnPin, clockPin, dataPin, resetPin);
  iec.init();
#endif

} // setup

void loop()
{

#ifdef DATASETTE_ENABLED
  if (mode == CBM_DISKDRIVE) {
    if(IEC::ATN_RESET == iface.handler()) {
      while(IEC::ATN_RESET == iface.handler());  // Wait to get out of reset
    }
  }
  else {
    cassette.handler();
  }
#else
  if(IEC::ATN_RESET == iface.handler()) {
    while(IEC::ATN_RESET == iface.handler());  // Wait to get out of reset
  }
#endif

} // loop

//Establish connection with the media host
static void connectMediaHost()
{
  char tempBuffer[80];

  //Initial handshake
  while (true) {

    //Empty serial buffer
    while(COMPORT.available())
      COMPORT.read();

    //Send connect token to media host
    COMPORT.println(HANDSHAKE_READY);
    delay(1000);

    //Check for acknowledgement ok response and continue when received
    if (COMPORT.find(HANDSHAKE_SEND) > 0)
      break;

  } // while not connected

  //Receive the pins and other assignments from the media host
  while (true) {
    if(COMPORT.readBytesUntil('\r', tempBuffer, sizeof(tempBuffer))) {
      #ifdef DATASETTE_ENABLED
      sscanf_P(tempBuffer, (PGM_P)F("%u|%u|%u|%u|%u|%u|%u|%u|%u|%u"),
              &mode, &deviceNumber, &atnPin, &clockPin, &dataPin, &resetPin,
              &motorPin, &readPin, &writePin, &sensePin);
      #else
      sscanf_P(tempBuffer, (PGM_P)F("%u|%u|%u|%u|%u|%u"),
              &mode, &deviceNumber, &atnPin, &clockPin, &dataPin, &resetPin);
      #endif
      break;
    }
  }

  //Blink to indicate connected ok
  pinMode(LED_BUILTIN, OUTPUT);  // initialize the onboard LED pin for output
  for (byte i = 0; i < CONNECT_BLINKS; ++i) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250); 
  }
  COMPORT.println(HANDSHAKE_OK);

} // connectMediaHost
