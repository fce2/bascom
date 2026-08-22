# a basic compiler

***WARNING***
**WIP !**
*Expect problems !*

---

**BC.EXE**

"Yet Anoter Basic Compiler" was taken already :-(
Writing compilers is a common hobby !

---

Another "ancient relict".
My decades old Pascal compiler, reworked now for Basic and tuned "a bit".
1) tokenizes *.bas (text) files to *T.prg (its still interpreted Basic !)
2) compiles *.bas (text) files to *.prg (pure compiled 6502 asm in prg format)
3) compiles *.bas (text) files to *.c (optional: creates a c-variant)

- strict CBM-Basic-V2 support (*--strict*) or with a bit more convenience, e.g. allow 'else' ;-)
- creates annoying stuff on demand only (*--annoying-stuff*): *.asm, *.lbl, *.map, *T.prg
- 12k more (A000-CFFF) with *--hiram* (but trampolines then !)
- full (?) TSB support (*--tsb*)
- AVR (!) support (*--avr*)

---

## Why ?

Why another compiler for a system which is dead for decades ?
There are many compilers already, over the last 50y many languages and dev-envs evolved.
If you want it efficient, use handwritten c (like oscar64 or llvm-mos), not basic.
And really fast is only direct handwritten assembler !
So "why" is a question i cant really answer. My curiosity mainly ?
And then it got better and better, and after I added host, gos and AVR support it started to be something new.

---

## Targets

### The C64 itself

Assume you have `foobar.bas`:
You have 3+ options for a prg:

**tokenized** `foobarT.prg`
	Normally very uninteresting.
	Needed for golden-refs only.
	Only created with `--annoying-stuff` enabled.
**basic-compiled** `foobar.prg`
	No asm needed anymore for games ;-)
	Just `bc foobar`, not even .bas is needed
**c-compiled** e.g. `foobarO.prg`
	Many times even faster if an "external optimizer" (like oscar64) is used.
	`bc foobar c` to create c source instead of a prg.
	`oscar64 foobar.c -o=foobarO.prg` to create the prg then.

#### C64 Extras

**CBM Basic V2**

`--strict`: only "pure" c64 basic is allowed.
Even RND() is original then.

**TSB - Tuned Simons Basic**

`--tsb` enables TSB support.
Or a line with `REM! TSB`.
No extra runtime needed, it creates a standalone prg.
`tsb.h` needed for c-variant.

**Multitasking**

*[heavy WIP !]*

Again an ancient relict.
GOS is my preemptive multitaker from the 90s...

`--gos` adds real preemtive mutitasking !
Or a line with `REM! GOS`.

*Its a very experimental feature now !*

Like GOTO or GOSUB, you can GOTASK.
And optionally YIELD.
YIELD is not needed here, but it looks more scrambled without.

```
	10 GOTASK 40 : REM start line 40 async
	20 GOTASK 50 : REM start line 50 async
	30 PRINT "!";:YIELD:GOTO 30
	40 PRINT "HELLO";:YIELD:GOTO 40
	50 PRINT "WORLD";:YIELD:GOTO 50
```

Each task needs an own stack.
6502 is very limited in stack size, all tasks have to share (the same !) 256 bytes in $01xx.
Each task gets its own slice in $01xx of GOS_STACK_SIZE bytes (which limits the amount of tasks).
With `10 REM! GOS: TASKS:8` or `10 REM! GOS: STACK:32` you can influence the "OS" a bit.

Multitasking makes *everything a bit slower*, even with only one task !
At the moment the whole ZP needs to be saved/restored on a taskswitch, thats really slow !
*Dont use it if you dont need/want it.*

One restriction: max 4 tasks (main + 3 more) are possible due to the 256 byte stack restiction, so each task gets 64 bytes.
This is needed if you basic program does e.g. PRINT, functions needing a bigger stack.
If you restrict the code to peek/pokes only, you can use also 8 tasks.
You can change that with `--gos-tasks=4` and `--gos-stack=64`.

Multitasking it very fragile !
- No stack overflow is checked !
- Its wip !

---

### "Host"

**c-compiled**
You can simply run all your basic programs on the host.
Very convenient for a quick and fast test.
Again, `bc foobar c` to create c source instead of a prg.
Then `gcc -O2 foobar.c -o foobar.exe` and thats it.

#### Host Extras

*hint:* 6502 host-simulation
If your basic program contains assembler (e.g. in datas, read and poked, ...) use `-DSYS6502`.
Then the runtime.h will use my cpu6502.c/h to simulate that on the host.
	`gcc -DSYS6502 -I ../.. -I ../../.. -static -s asm.c -o asm.exe`
(the many ".." are the path to cpu6502.c/h and sim.h on my system)

*hint:* c64 file io
Host debugging, e.g. file i/o, is very convenient.
The runtime reads/writes real "*.seq" files.
This works on the host:
```
	3501 BC=0:OPEN 1,8,2,TN$+",R"
	3502 IF PEEK(144)<>0 THEN CLOSE 1:RETURN
	3503 GET#1,GC$
	3504 IF PEEK(144)<>0 THEN GOTO 3507
	3505 POKE BA+BC,ASC(GC$+CHR$(0)):BC=BC+1
	3506 GOTO 3503
	3507 CLOSE 1:RETURN
```

---

### AVR

**c-compiled**
Program your Arduiono in original C64 Basic V2 !

`--avr` or e.g. `--avr=atmega328p` creates c code for e.g. avr-gcc.
The runtime then prepares the needed PORTB/DDRB/UDR0/...:

```
	bc.exe breath.bas --avr=atmega328p
	avr-gcc.exe -I ../.. -mmcu=atmega328p -Os -ffunction-sections -fdata-sections -DB_TIER=2 -DF_CPU=16000000UL -DBAUD=115200 -Wl,--gc-sections -Wl,-Map,breath.map breath.c -lm -o breath.elf
	avr-objcopy.exe -O ihex breath.elf breath.hex
	avr-objcopy.exe -O binary breath.elf breath.bin
	avr-objdump.exe -d breath.elf > breath.asm
	avrdude.exe -C <<<where your avr stuff is>>>/avrdude.conf -c arduino -p atmega328p -P COM6 -b 115200 -U flash:w:breath.hex
```


**Tiers** `-DB_TIER=<n>` shrinks the runtime to what the program needs:
- `B_TINY`  (0): integer-only (blink). `DOUBLE=long`, no float code linked.
- `B_SMALL` (1): PRINT + int (print). float optional.
- `B_FULL`  (2): float (SQR/mul/INT/PRINT — calc). pulls in sqrt/softfloat.

`PRINT/INPUT` go to UART, e.g. `10 PRINT "HELLO WORLD"` works.

LED blink:
```basic
	10 POKE DDRB, 32
	20 POKE PORTB, 0
	15 SLEEP 500
	30 POKE PORTB, 32
	35 SLEEP 500
	40 GOTO 20
```

A simple "breath" for Arnduino-UNO in 100% C64-Basic-V2:
```basic
	10 PI = 3.141593
	20 N = 64
	30 ST = 2 * PI / N
	40 POKE DDRB, 32
	50 FOR I = 0 TO N - 1
	60 A = I * ST
	70 V = SIN(A)
	80 B% = INT((V + 1) * 127.5)
	90 FOR R = 1 TO 60
	100 FOR K = 0 TO 255
	110 IF K < B% THEN POKE PORTB, 32
	120 IF K >= B% THEN POKE PORTB, 0
	130 NEXT K
	140 NEXT R
	150 NEXT I
	160 GOTO 50
```

**AVR HW Support**
- All chip "registers" like PORTB/DDRB/UDR0/...
- ADC:
	`10 V = ADC(0)`
- Digital-I/O:
	`10 PINMODE 2, 2`
	`20 B = DREAD(2)`
	`30 PINMODE 13, 1`
	`40 DWRITE 13, 1`
- EEPROM:
	`10 EEWRITE 10, 42`
	`20 V = EEREAD(10)`
- I2C:
	`10 I2CSTART`
	`20 I2CWRITE 160`
	`30 I2CWRITE 0`
	`40 I2CSTART`
	`50 I2CWRITE 161`
	`60 V = I2CREAD(0)`
	`70 I2CSTOP`
- Data "manipulation":
	`10 V = ADC(0)`
	`20 P = MAP(V, 0, 1023, 0, 100)`
	`30 P = CONSTRAIN(P, 0, 100)`
	`40 PRINT P; "%"`
- "native" random:
	`10 R = RANDOM(10)`
- Sleep:
	`10 SLEEP 500`
- SPI:
	`10 SPIBEGIN`
	`20 FOR K = 0 TO 9`
	`30 B = SPIMASTER(K)`
	`40 PRINT "TX "; K; " RX "; B`
	`50 SLEEP 200`
	`60 NEXT K`
	`70 GOTO 20`
- Watchdog:
	`10 WDT 30`
	`20 PRINT "WATCHDOG ON"`
	`30 SLEEP 100`
	`40 WDTOFF`

---

## Usage

> Compile manzoo.bas to manzooT.prg and manzoo.prg
**`bc.exe manzoo.bas`**
	creates: `manzoo.prg` is the compiled version of the bas file
**`bc.exe -a manzoo.bas`**
	creates also: `manzooT.prg` is the tokenized version of the bas file

> Compile manzoo.bas to manzoo.c (and then to host manzoo.exe)
**`bc.exe manzoo.bas c`**
	creates: `manzoo.c`
**`gcc manzoo.c -o manzoo.exe`**
	creates: `manzoo.exe` ... Wow, compare manzoo.prg and manzoo.exe how technology has developed !
or
**`oscar64 manzoo.c -o=manzooO.prg`**
	creates: `manzooO.prg`

Very impressive to see the jump from manzooT.prg to manzoo.exe !
Even more impressive: bt2.bas to bt2.exe !

---

## Extras

*hint:* optimizers
`-v` shows used and not used optimizers.

**You can define and include:**

main.bas:
```basic
	10 REM scrollers
#define SCROLL_CYCLIC
#include "2xxx_scroller.bas"
	20 GOSUB 2000
	30 END
```

2xxx_scroller.bas:
```basic
	2000 REM scroller-lib
	2001 ..do some $0400 scrolling stuff here..
#ifdef SCROLL_CYCLIC
	2002 ..scroll cyclic..
#else
	2002 ..scroll not cyclic..
#endif
	2999 RETURN
```

*hint:* tokenized prg
If you create the tokenized prg version with -a you get the "preprocessed" result.

**Easy project setup:**
	`--init <dir>` creates a starter-environment with all you need: A demo program, the needed runtime.h, Makefile, ...
	`--init` checks the current dir for missing files.

**(some) Commandline options**
	`--verbose`			see which optimizers fire (or not)
	`--version`			e.g. "bascom 2026-08-20 11:58:47"
	`--init [<dir>]`	create/init a new project
	`--strict`			use strict cbm basic v2
	`--no-illegals`		the compiler uses normally "verified" illegals to speed up
	`--hiram`			done automatically if needed
	`--annoying-stuff`	create tokenized prg and sidecars
	`--tsb`				support Tunes Simons Basic
	`--gos`				support multitasking
	`--avr[=mcu]`		compile for AVR
	`--source`			"c64v2" for now
	`--target`			"c64" and "x16" for now (Commander X16 is wip)
	`--opt`				none, speed, size, c
	and the usual -o

**"pramgas"** kind of...
	`REM! STRICT`       .
	`REM! NOILLEGALS`   .
	`REM! TSB`          .
	`REM! GOS`          .
	`REM! AVR=`         .
	`REM! SOURCE=`      .
	`REM! TARGET=`      .
	`REM! +SPEED|+SIZE|+RESET` (see 'Regions' below)

**Regions**
If memory gets low, you can select where to spend the valuable ram for speed:
	`10 REM! +SPEED` enables speed optimizer
	`20 ...`         do speedy stuff here
	`50 REM! +SIZE`  enables size optimizer
	`60 ...`         do packed stuff here
	`70 REM! +RESET` resets to default form commandline

---

## Files

- `runtime.h`
	Only needed for OPTIMIZE_C, when compiling the "resúlt c file" !
	3 modes:
		- `TARGET_C64`
			supports cc65, oscar64, llvm-mos, gate -> creates *.prg files
		- `TARGET_HOST`
			supports gcc, msvc, ...                -> creates *.exe files
		- `TARGET_AVR`
			supports avr-gcc, ...                  -> creates *.bin files

- `tsb.h`
	Just in case you want to run TSB programs on your host with `--tsb --opt c` ...

- `avr-crt0.S`
	Only needed for OPTIMIZE_C, when compiling with `--avr` !
	The only purpose is to minimize binary size (well, ~70 bytes), you can drop `-nostartfiles` and the `avr-crt0.o` and go the gcc way.
	"button.bin" has only 116 bytes with this crt0.
	Use only if you dont have ISRs, its very basic, it has no vector support !

---

## Demos

### C64

- `balls.bas`
	A super nice demo i found somewhere.
- `life.bas`
	Conway's game of life.
- `manzoo.bas`
	A super simple Mandelbrot Zoom.
	As prg too slow, as exe too fast.
- `matrix.bas`
	Well, Matrix...

#### GOS

- `gos.bas` hello world tasks
- `gos1.bas` hello world tasks

### Host

- `bt2.bas`
	`bt.bas` is a raytracer ("by marco64 14.06.2008").
	I just added animation (wasd/crsr to move).
	This is not prerendered, zoom out for even more fps !
	Not 30h/frame, but 70+ frames/sec ...
- `mirrorball3.bas`
	I found `mirrorball.tsb` on a disk named "raytr c't 1/86".
	I just added animation (wasd/crsr to move, space to drop).
- `juggler.bas`
	My try of an Amiga juggler, hahaha.
- `boing.bas`
	A try of a "3d-pong".
	For sure the slowest "game" ever (as prg) !
	But too fast to play (as exe).
- `povray.bas`
	Also just a beginning, will never finish, just a test for me...
	But, hey, this works already (use exe, not prg to test):
```basic
	11210 DATA "SCENE", "STILL"
	11220 DATA "CAMERA", 0, 2.5, - 8, 0, 2
	11230 DATA "LIGHT", - 20, 30, - 20
	11240 DATA "PLANE", - 1, 4
	11250 DATA "SPHERE", - 2, 0, 2, 1.0, 0
	11251 DATA "SPHERE",   0, 0, 2, 1.5, 1
	11252 DATA "SPHERE",   2, 0, 2, 0.5, 1
	11260 DATA "BOX", 4, - 1, 1, 6, 2, 3
	11270 DATA "END"
```

### Both

- `asm.bas`
	Demo of integrated assembler and host simulator.

### Games

Some ultra short and mostly really bad demos only, sorry.
All wip, and will most likely never finish.
No game balancing, no correct timing, many bugs.
I just needed some test objects for the compiler.
Feel free to provide good ones.

### AVR

Same here, just some examples for my own ArduinoUNO.
I never tested anything except ATmega328p !
I dont even have much hardware currently, so i tested the LED and UART mainly (well, only) !

- `breath.bas`
	A breathing LED on the Arduiono-UNO.
- `morse.bas`
	Showing PRINT over UART.

---

## Future

Not sure yet.
- performance increase
- X16 full support
- C16/C128/... support (V3.5 / V7)
- more source dialects
- more targets
- funny things like AmigaBasic for C64 ?
- let me know what you are interested in
