#include <avr/io.h>

	.section .vectors,"ax",@progbits
	.global __vectors
__vectors:
	rjmp __init

	.section .init0,"ax",@progbits
	.global __init
__init:
	; SP = RAMEND
	ldi	r29, hi8(RAMEND)
	ldi	r28, lo8(RAMEND)
	out	0x3e, r29		; SPH
	out	0x3d, r28		; SPL
	eor	r1, r1			; __zero_reg__ = 0
	out	0x3f, r1		; SREG = 0  (I-bit clear -> no interrupts)

	; copy .data from flash to SRAM (X=dest, Z=flash source, r17=hi8(end))
	ldi	r17, hi8(__data_end)
	ldi	r26, lo8(__data_start)
	ldi	r27, hi8(__data_start)
	ldi	r30, lo8(__data_load_start)
	ldi	r31, hi8(__data_load_start)
	rjmp	1f
2:	lpm	r0, Z+
	st	X+, r0
1:	cpi	r26, lo8(__data_end)
	cpc	r27, r17
	brne	2b

	; clear .bss (X=dest, r1=0)
	ldi	r17, hi8(__bss_end)
	ldi	r26, lo8(__bss_start)
	ldi	r27, hi8(__bss_start)
	rjmp	3f
4:	st	X+, r1
3:	cpi	r26, lo8(__bss_end)
	cpc	r27, r17
	brne	4b

	call main
	cli
5:	rjmp 5b
