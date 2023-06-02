#ifndef DATASETTE_H
#define DATASETTE_H

#include "global_defines.h"

#ifdef DATASETTE_ENABLED

#include <Arduino.h>

#define XFR_SIZE 64
#define NUM_XFRS 8
#define BUF_SIZE (XFR_SIZE*NUM_XFRS)

class Datasette
{
public:
	void init(void);  //
  void setPins(byte motorPin, byte readPin, byte writePin, byte sensePin);  //Pin settings
  void handler(void);  //Handle the datasette operations

private:
  void execute(int cmd);
  void read_motor();
  void send_tap_interval(unsigned long interval);
  void record();
  void buffer_next();

	// Pin assignments
  //GND pin connect to C64 pin 1-A (Ground)
  byte m_motorPin;   //C64 pin 3-C (Motor) with a ~100K resistor to a separate GND to let capacitor fully discharge
  byte m_readPin;    //C64 pin 4-D (Read)
  byte m_writePin;   //C64 pin 5-E (Write)
  byte m_sensePin;   //C64 pin 6-F (Sense)

  // The host (PC / Android phone) sends data to the Arduino system buffer which is buffered into memory
  // The Arduino serial buffer is 64 bytes which must be the same on the host. A serial buffer of 48 bytes has also been used successfully

  int buf[BUF_SIZE];
  int buf_pos, xfr_pos, buf_entries;

  // Flags used for play / record / motor state
  int playing;
  int pre_buffering;
  int recording;
  int recording_start;
  int xmotor_on;

  // Variables for pulse length in play mode
  unsigned long pulse_length;
  unsigned long last_pulse;
  int reading_extended;
  unsigned long extended_data[3];

  // Variables for pulse interval in record mode
  int last_write_val;
  unsigned long last_write_time;

};

#endif
#endif
