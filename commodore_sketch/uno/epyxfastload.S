#include "epyxfastload.h"
#include <avr/io.h>

#ifndef CONFIG_MCU_FREQ
#  error "CONFIG_MCU_FREQ must be defined for delay cycle calculation."
#endif

        .section .text

        ;; ===================================================================
        ;;  Utility routines
        ;; ===================================================================

        ; RCALL and RET each take an additional cycle on MCUs with >128kB
        ; program memory (and therefore a 22 bit PC).
        ; These are defined as offset to the (16 bit PC) base value instead of
        ; absolute cycles because that simplifies delay cycle calculation.
#ifndef __AVR_3_BYTE_PC__
#  define RCALL_OFFSET  0
#  define RET_OFFSET    0
#else
#  define RCALL_OFFSET  1
#  define RET_OFFSET    1
#endif

        ;;  Used by the macros below, don't call directly
        nop
        nop
delay_loop:
        dec     r18             ; 1 (ldi) + 3*#r18 - 1 (last brne not taken)
        brne    delay_loop
delay_7:                        ; 3+4=7 (16 bit PC) / 4+5=9 (22 bit PC)
        ret

        ;; This macro waits for the specified number of cycles
        ;;  Uses r18
        .macro delay_cycles num
        .if     \num >= 10 + RCALL_OFFSET + RET_OFFSET
        ldi     r18, (\num - (7 + RCALL_OFFSET + RET_OFFSET)) / 3
        rcall   delay_loop - 2*((\num - (10 + RCALL_OFFSET + RET_OFFSET)) % 3)
        .elseif \num >= 7 + RCALL_OFFSET + RET_OFFSET
        rcall   delay_7
        delay_cycles \num - (7 + RCALL_OFFSET + RET_OFFSET)
        .elseif \num >= 2
        rjmp    .+0
        delay_cycles \num-2
        .elseif \num == 1
        nop
        .endif
        .endm


        ;; Delay in 0.5us resolution. Only used if the additional resolution is
        ;; necessary, most places use delay_us below (which calls this macro).
        ;;
        ;; offset is the number of cycles to add to the result of the
        ;; conversion from [us]. Can be either positive or negative, but a
        ;; negative value can result in an illegal (ie negative) delay.
        .macro  delay_05us us05:req, offset=0
        .if ((\us05)*CONFIG_MCU_FREQ%(2*1000000)         != 0) || \
            ((\us05)*CONFIG_MCU_FREQ/(2*1000000)+\offset <  0)
        .error "Invalid delay value and/or CONFIG_MCU_FREQ"
        .endif
        delay_cycles (\us05) * CONFIG_MCU_FREQ/(2*1000000) + \offset
        .endm

        ;; Delay in 1us resolution.
        .macro  delay_us us:req, offset=0
        delay_05us 2*(\us), \offset
        .endm

        ;; send bits 7 and 5 of r0 to clock/data
        ;; masked contents of IEC_OUTPUT expected in r19
        ;; 8 (or 9 for 22 bit PC MCUs) cycles from rcall to out, 4 (or 5) to return
epyx_bitpair:
        ;; rcall - 3 or 4
        bst     r0, 7                      ; 1 ; Store bit 7 of r0 into T flag
        bld     r19, IEC_OPIN_CLOCK        ; 1 ; Load T flag into CLOCK bit of r19
        bst     r0, 5                      ; 1 ; Store bit 5 of r0 into T flag
        bld     r19, IEC_OPIN_DATA         ; 1 ; Load T flag into DATA bit of r19
        out     _SFR_IO_ADDR(IEC_OUTPUT_D), r19 ; 1
        ret                                ; 4 or 5

        ;;
        ;; Send a byte using the Epyx Fastload cartridge protocol
        ;;
        .global asm_epyxcart_send_byte
asm_epyxcart_send_byte:
        ;; DATA and CLOCK high
        sbi     _SFR_IO_ADDR(IEC_OUTPUT_D), IEC_OPIN_DATA  ;Set pin to high
        sbi     _SFR_IO_ADDR(IEC_OUTPUT_D), IEC_OPIN_CLOCK  ;Set pin to high
        delay_us 1

        ;; prepare data
        in      r19, _SFR_IO_ADDR(IEC_OUTPUT_D)  ;Read port D (set of pin values) into r19
        andi    r19, ~(IEC_OBIT_DATA|IEC_OBIT_CLOCK|IEC_OBIT_ATN|IEC_OBIT_SRQ)  ;ORs the values of the pins (8 bit values each) and bitwise inverts the result

        ;; wait for DATA high or ATN low
1:      sbis    _SFR_IO_ADDR(IEC_INPUT_D), IEC_OPIN_ATN
        rjmp    epyxcart_atnabort
        sbis    _SFR_IO_ADDR(IEC_INPUT_D), IEC_OPIN_DATA
        rjmp    1b

        com     r24                 ; 1 ; Flip all the bits / aka bitwise inversion / aka one's complement
        mov     r0, r24             ; 1
        delay_us 10, -8-RCALL_OFFSET  ; Used with the com r24 above
        rcall   epyx_bitpair        ; 8+4 or 9+5 - bits 7 and 5

        lsl     r0                  ; 1
        delay_us 10, -13-RET_OFFSET-RCALL_OFFSET
        rcall   epyx_bitpair        ; 8+4 or 9+5 - bits 6 and 4

        swap    r24                 ; 1
        mov     r0, r24             ; 1
        delay_us 10, -14-RET_OFFSET-RCALL_OFFSET
        rcall   epyx_bitpair        ; 8+4 or 9+5 - bits 3 and 1

        lsl     r0                  ; 1
        delay_us 10, -13-RET_OFFSET-RCALL_OFFSET
        rcall   epyx_bitpair        ; 8+4 or 9+5 - bits 2 and 0

        delay_us 20, -RET_OFFSET    ; final delay so the data stays valid long enough

        clr     r24
        ret

epyxcart_atnabort:
        ldi     r24, 1
        ret

        .end
