#include "iec_driver.h"
#include "interface.h"

#define DEFAULT_BAUD_RATE 115200
#define SERIAL_TIMEOUT_MSECS 1000

#define HANDSHAKE_READY "<CON>\r"
#define HANDSHAKE_SEND "<AOK>"
#define HANDSHAKE_OK "<END>\r"

#define CONNECT_BLINKS 2

static IEC iec(8);
static Interface iface(iec);

unsigned mode, deviceNumber, atnPin, clockPin, dataPin, resetPin;

void setup()
{

  //Initialize serial and wait for port to open
  Serial.begin(DEFAULT_BAUD_RATE);
  while (!Serial);
  Serial.setTimeout(SERIAL_TIMEOUT_MSECS);

  connectMediaHost();
  iec.setDeviceNumber(deviceNumber);
  iec.setPins(atnPin, clockPin, dataPin, resetPin);
  iec.init();

} // setup

void loop()
{

  if(IEC::ATN_RESET == iface.handler()) {
    while(IEC::ATN_RESET == iface.handler());  // Wait to get out of reset
  }

} // loop

//Establish connection with the media host
static void connectMediaHost()
{
  char tempBuffer[64];

  //Initial handshake
  while (true) {

    //Empty serial buffer
    while(Serial.available())
      Serial.read();

    //Send connect token to media host
    Serial.write(HANDSHAKE_READY);
    delay(1000);

    //Check for acknowledgement ok response and continue when received
    if (Serial.find(HANDSHAKE_SEND) > 0)
      break;

  } // while not connected

  //Receive the pins and other assignments from the media host
  while (true) {
    if(Serial.readBytesUntil('\r', tempBuffer, sizeof(tempBuffer))) {
      sscanf_P(tempBuffer, (PGM_P)F("%u|%u|%u|%u|%u|%u"),
              &mode, &deviceNumber, &atnPin, &clockPin, &dataPin, &resetPin);
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
  Serial.write(HANDSHAKE_OK);

} // connectMediaHost
