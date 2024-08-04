//Mixed board pin to AVR chip pin mappings across port B and F
# define IEC_INPUT_B            PINB
# define IEC_OUTPUT_B           PORTB
# define IEC_OPIN_ATN           PB5  //pin 9

# define IEC_INPUT_F            PINF
# define IEC_OUTPUT_F           PORTF
# define IEC_OPIN_SRQ           PF5  //pin 20
# define IEC_OPIN_DATA          PF6  //pin 19
# define IEC_OPIN_CLOCK         PF7  //pin 18

# define IEC_OBIT_ATN   _BV(IEC_OPIN_ATN)
# define IEC_OBIT_DATA  _BV(IEC_OPIN_DATA)
# define IEC_OBIT_CLOCK _BV(IEC_OPIN_CLOCK)
# define IEC_OBIT_SRQ   _BV(IEC_OPIN_SRQ)

# define CONFIG_MCU_FREQ 16000000

#ifdef __ASSEMBLER__

.global asm_epyxcart_send_byte

#endif

#ifndef __ASSEMBLER__

extern "C" uint8_t asm_epyxcart_send_byte(uint8_t byte);

#endif