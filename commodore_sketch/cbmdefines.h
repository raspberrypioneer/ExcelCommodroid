#ifndef CBMDEFINES_H
#define CBMDEFINES_H

namespace CBM {

// Largest Serial byte buffer request from / to arduino.
#define MAX_BYTES_PER_REQUEST 256

// Device OPEN channels.
// Special channels.
enum IECChannels {
	READPRG_CHANNEL = 0,
	WRITEPRG_CHANNEL = 1,
	CMD_CHANNEL = 15
};

} // namespace CBM

#endif // CBMDEFINES_H
