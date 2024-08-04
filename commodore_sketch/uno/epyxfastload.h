//All board pin to AVR chip pin mappings are located on port D
# define IEC_INPUT_D            PIND
# define IEC_OUTPUT_D           PORTD
# define IEC_OPIN_ATN           PD2  //pin 2
# define IEC_OPIN_CLOCK         PD3  //pin 3
# define IEC_OPIN_DATA          PD4  //pin 4
# define IEC_OPIN_SRQ           PD5  //pin 5

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