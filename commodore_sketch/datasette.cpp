#include "datasette.h"

#ifdef DATASETTE_ENABLED

void Datasette::init(void)
{
  pinMode(m_motorPin, INPUT);
  pinMode(m_readPin, OUTPUT);
  pinMode(m_writePin, INPUT);
  pinMode(m_sensePin, OUTPUT);
  
  digitalWrite(m_readPin, HIGH);
  digitalWrite(m_sensePin, HIGH);
}

void Datasette::setPins(byte motorPin, byte readPin, byte writePin, byte sensePin)
{
  m_motorPin = motorPin;
  m_readPin = readPin;
  m_writePin = writePin;
  m_sensePin = sensePin;

} // setPins

void Datasette::handler(void)
{
  read_motor();

  if(recording) {
    record();
    if(COMPORT.available()>0) {
      int cmd=COMPORT.read();
      execute(cmd);
    }
    return;
  }

  if(!playing) {
    delay(100);
    if(COMPORT.available()>0) {
      int cmd=COMPORT.read();
      execute(cmd);
    }
    return;
  }

  if(!xmotor_on) return;

  // prepare buffer
  if(buf_pos>=BUF_SIZE) buf_pos=0;
  if(buf_entries<NUM_XFRS) buffer_next();
  else pre_buffering=0;
  if(pre_buffering) return;
  // first byte is control
  if(buf_pos%XFR_SIZE==0) {
    execute(buf[buf_pos++]);
    if(!playing) return;
  }
  
  unsigned long tap_data=(unsigned long)buf[buf_pos++];

  if(buf_pos%XFR_SIZE==0) buf_entries--;

  if(buf_entries<1) COMPORT.println('E');

  // calculate the pulse interval
  if(reading_extended) {
    reading_extended--;
    extended_data[reading_extended]=tap_data;
    if(reading_extended) {
      return;
    } else {
      pulse_length=(extended_data[0]<<16)+(extended_data[1]<<8)+extended_data[2];
      pulse_length*=1.015;
    }
  } else if(tap_data==0) {
    reading_extended=3;
    return;
  } else {
    pulse_length=tap_data*8.12;
  }
  
  // send a 1 at half interval
  while(micros()<(last_pulse+(pulse_length/2)));     
  digitalWrite(m_readPin, 1);

  // send a 0 at full interval
  while(micros()<(last_pulse+pulse_length));
  digitalWrite(m_readPin, 0);

  last_pulse=micros();

}

// Execute command received from the host
void Datasette::execute(int cmd) {


  switch(cmd) {

    // nop (do nothing)
    case 'Z':
      break;

    // ping
    case 'P':
      COMPORT.println('P');
      break;

    // play / read
    case 'R':
      playing=1;
      pre_buffering=1;
      buf_pos=0;
      buf_entries=0;
      xfr_pos=-1;

      COMPORT.println('N');
      break;

    // stop
    case 'r':
      playing=0;
      recording=0;
      break;

    // sense on
    case 'S':
      digitalWrite(m_sensePin, 0);
      break;

    // sense off
    case 's':
      digitalWrite(m_sensePin, 1);
      break;

    // record on
    case 'W':
      recording=1;
      recording_start=1;
      break;

  }
}

// Get motor pin value
void Datasette::read_motor() {
  xmotor_on = digitalRead(m_motorPin);
}

// For save / record:
// Calculate and send data (pulse interval) to host
void Datasette::send_tap_interval(unsigned long interval) {
  unsigned long tap_interval=interval/8.12;
  if(tap_interval==0) tap_interval++;
  if(tap_interval<256) {
    COMPORT.println(tap_interval);
    return;
  }

  COMPORT.println((byte) 0x00);
  COMPORT.println(interval&0xff);
  COMPORT.println((interval&0xff00)>>8);
  COMPORT.println((interval&0xff0000)>>16);

}

// For save / record:
// Read the write pin value from the Commodore and if value changed and set HIGH, send the calculated pulse interval to host
void Datasette::record() {
  int write=digitalRead(m_writePin);
  if(write!=last_write_val) {
    last_write_val=write;
    if(write==HIGH) {
      unsigned long time_now=micros();
      if(recording_start) {
        recording_start=0;
      } else {
        unsigned long interval=time_now-last_write_time;
        send_tap_interval(interval);
      }
      last_write_time=time_now;
    }
  }
}

// For read / play:
// Request the next buffer size of data (XFR_SIZE)
// Then copy Arduino serial buffer into memory buffer
void Datasette::buffer_next() {

  if(xfr_pos==-1) {

  //Check for Uno, for some unknown reason, COMPORT.available() reports 63 when there actually are 64 bytes in the buffer
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
    if(COMPORT.available()+1<XFR_SIZE) return;
  #else
    if(COMPORT.available()<XFR_SIZE) return;
  #endif

    else xfr_pos=0;
    return;
  }

  if(xfr_pos==XFR_SIZE) {
    buf_entries++;
    COMPORT.println('N');
    xfr_pos=-1;
    return;
  }

  int pos_base=buf_pos-(buf_pos%XFR_SIZE);
  int buf_base=buf_entries*XFR_SIZE;
  for(int i=0; i<8; i++) {
    int index=(pos_base+buf_base+xfr_pos)%BUF_SIZE;
    buf[index]=COMPORT.read();
    xfr_pos++;
    if(xfr_pos==XFR_SIZE) break;
  }
}

#endif
