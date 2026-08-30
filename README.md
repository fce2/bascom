# ABC - A Basic Compiler

***WARNING***
**WIP !**
*Expect problems !*

Slowly going from "please help to find bugs" to "please try to break it" ;-)
Well, soon...

---

**ABC.EXE**

The name...
- "Yet Anoter Basic Compiler" was taken already :-( [https://github.com/tommyo123/YABCompiler]
- "BasCom" also... [https://www.mcselec.com/index.php]
- I got a complaint about "bc", a tool in linux i never used so far, a very cool thing !
Writing compilers and cool tools is a common hobby !
So its "*A Basic Compiler*", ABC.
Quite memorable and easy to type.
And since i had already "bc.exe", adding an A was easy.
Still searching "more"... But it too long to type everytime:
"abcdefghij.exe" ? "*A Basic Compiler from DE by Fce2, including Gos and Host stuff. I like it ! Jou too?*" (bad, i know)...

---

Its another "ancient relict".
My decades old Pascal compiler, reworked now for Basic and tuned "a bit".
- tokenizes *.bas (text) files to *T.prg (its still interpreted Basic !)
- compiles *.bas (text) files to *.prg (pure compiled 6502 asm in prg format)
- compiles *.bas (text) files to *.c (optional: creates a c-variant)

---

## Why ?

### Why doing this ?

Why another compiler for a system which is dead for decades ?
There are many compilers already, over the last 50y many languages and dev-envs evolved.
If you want it efficient, use handwritten c (like oscar64 or llvm-mos), not basic.
And really fast is only direct handwritten assembler !
So "why" is a question i cant really answer. My curiosity mainly ?
And then it got better and better, and after I added host, gos and AVR support it started to be something new.
I also (re)learned new/old "interesting" things:
C64 basic has no else.
Only the first 2 chars are used for var names.

### Why using abc now !

- Simple to use.
	Just one exe. No installation of anything around, no JDK, .NET, any "cargo", external libs/deps...
	Completely selfcontained, --init creates all you need.
	Really just "abc xyz", not even ".bas" needed, yes, i'm lazy sometimes.
- Win7+ and Linux build
	Linux for x86_64 or aarch64.
	I've no idea (yet) how to create mac binaries, sorry.
- Fast binaries. Still working on it...
	Compare yourself with other compilers or even languages.
- Small binaries if needed. Still working on it...
	Up to (down to ?) 50% size compared to speed mode, but also 50% speed then.
	Packer at compiletime and Unpacker at runtime included. If you want the "raw thing" use --no-pack.
- Fast/Small combinations possible with "pragmas".
	Allows fast/big sections with relevant code to be maximal fast, but the rest very compressed.
- Fast compile time
	Normally <<1s per basic file.
- Very strict CBM-Basic-V2 support for nerds
	With *--strict* even RND is original.
	Or more relaxed with a bit more convenience, e.g. allow 'else' ;-)
- creates annoying stuff on demand only
	*--annoying-stuff* creates: *.asm, *.lbl, *.map, *T.prg
	The asm is just for reference, not (yet) compilable !
	Maybe interesting and not so annoying: the tokenized *T.prg.
- 12k more if needed
	With *--hiram* $A000-$CFFF is used, not only up to $9FFF. Done automatically if it not fits.
	But you have trampolines then (imagine a program in Axxx RAM region calling a ROM function also in $Axxx) !
- full (?) TSB support (*--tsb*)
	No ROMs or other runtime needed, completely self-contained.
	Includes GOS support.
- AVR (!) support (*--avr*)
	Basic support for AVR (yes, both Basics here ;-)
	Full C64-Basic-V2 support.
	But basic support only for AVR. Not the advanced support 6502 has. You understand.
- targets: c64, x16
	Yes, Simons Basic and multitasking also on X16 !
- Multitasking (*--gos*) !
	Last but not least: my favorite gimmick !

---

## Downloads

| what               | where               | how                              |
|--------------------|---------------------|----------------------------------|
|**bin/win/abc.exe** | x64 Windows 7+      | VS2026, no dll needed            |
| bin/win/abcd.exe   | x64 Windows 7+      | cygwin gcc, needs cygwin1.dll !  |
| bin/linux/abc      | x86_64 Linux ≥ 3,2  | gcc, static, compiled in wsl     |
| bin/pi/abc         | aarch64 Linux ≥ 3.7 | gcc, static, compiled on my rpi4 |

**runtime** (only needed for the "c" variants):
(No download needed, all can be created with `--init`)
| what               | why                 | who                           |
|--------------------|---------------------|-------------------------------|
| runtime.h          | for all c compilers | host or target (gcc or oscar) |
| tsb.h              | tsb-on-host only    | host or target (gcc or oscar) |
| x16.h              | x16-on-host only    | not yet !                     |
| avr-crt0.S         | avr only            | very optional                 |

---

## Targets

### The C64 itself

Assume you have `foobar.bas`:
You have many options to get a running prg:

**tokenized** `foobarT.prg`
	Normally very uninteresting.
	Needed for golden-refs only.
	Only created with `--annoying-stuff` enabled.
**basic-compiled** `foobar.prg`
	No asm needed anymore for games ;-)
	Just `abc foobar`, not even .bas is needed
	In different flavors: `abc foobar size` for smaller/slower code.
**c-compiled** e.g. `foobarO.prg`
	Many times even faster if an "external optimizer" (like oscar64) is used:
	`abc foobar c` to create c source instead of a prg.
	`oscar64 foobar.c -o=foobarO.prg` to create the prg then.

#### C64 Extras

##### **CBM Basic V2**

`--strict`: only "pure" c64 basic is allowed.

##### **TSB - Tuned Simons Basic**

`--tsb` enables TSB support.
Or a line with `REM! TSB`.
No extra runtime/rom needed, it creates a standalone prg.
`tsb.h` needed for c-variant only.

##### **Multitasking**

Again an ancient relict.
GOS is my preemptive multitaker from the 90s...
See here, updated to 6502 also: https://github.com/fce2/GOS

`--gos` adds real preemtive mutitasking !
Or a line with `REM! GOS`.

Like GOTO or GOSUB, you can TASYNC a line:
```basic
	10 TASYNC 40
	20 TASYNC 50
	30 PRINT "!"; : GOTO 30
	40 PRINT "HELLO"; : GOTO 40
	50 PRINT "WORLD"; : GOTO 50
```

The scheduler ticks at 10Hz by default: 10 preemptive task switches per second, each time slice 100ms.
If you have 2 tasks, each one will be ticked 5 times/s.
If you have 10 tasks, each one will be ticked only one time/s for max 100ms!
Switch overhead is 1..4ms, depending on ZP usage (e.g. if you work with floats, it takes longer).
The 10 Hz default divides the std jiffy IRQ ($0314) by 5 (PAL 10 Hz; NTSC 12 Hz). NTSC users set `--gos-fast=6` for a true 10 Hz.
`--gos-fast` ticks every jiffy (50/60 Hz), `--gos-fast=17` every 17th jiffy.
The CIAs are not reprogrammed, i used std C64 init, so you can play here also and "tune".

New "T"(ask) commands:
*TASYNC*	start a line async, the only "must" command
	`TASYNC 1000` starts line 1000 async and returns immediately
*TYIELD*	voluntary yield (not needed)
	`TYIELD` if you are done with this ticks work
*TLOCK*		a critical section for all commands after it (not needed)
	`TLOCK:A=A+1:B=B-1` makes sure A and B dont get scrambled
*TSLEEP*	TYIELD is polite, TSLEEP is politer, it allows to prevent very expensive task switches (not needed)
	`TSLEEP 10` sleeps 10 ticks = 1 sec
*TWAIT*		waits for another tasks TSIGNAL, not wasting time
	`TWAIT 1` waits for signal '1'
*TSIGNAL*	signals another tasks TWAIT to continue
	`TSIGNAL 1` signals '1'

Each task needs an own stack.
6502 is very limited in stack size, all tasks have to share (the same !) 256 bytes in $01xx.
Each task gets its own slice in $01xx of GOS_STACK_SIZE bytes (which limits the amount of tasks).
With `10 REM! GOS TASKS 8` or `10 REM! GOS STACK 32` you can influence the "OS" a bit.

*Overhead warning*
*Just dont use it if you dont need/want it.*
Multitasking makes *everything a 'bit' slower*, even with only one task !
At the moment nearly the whole ZP needs to be saved/restored on a taskswitch to make basic work well in all situations, thats really slow !
The current overhead is ~1% to 98% "(G)OS time" ! Up to you !
This mostly depends on your TYIELD usage.

Here its ~1.1% scheduler overhead:
30 and 40 run 100ms each, then a cheap switch.
```basic
	10 REM! GOS
	20 TASYNC 40
	30 GOTO 30
	40 GOTO 40
```

Here its ~97.8% scheduler overhead!
30 and 40 switch only and always.
Well, they dont have to do anything else !
```basic
	10 REM! GOS
	20 TASYNC 40
	30 TYIELD:GOTO 30
	40 TYIELD:GOTO 40
```

One solution for delays is just using many TYIELDs. Each one "sleeps" until the next wakeup.
However, then OS gets a lot of cycles, not other tasks.
The better solution is TSLEEP:
A `TSLEEP 5` is basicly the same as `TYIELD:TYIELD:TYIELD:TYIELD:TYIELD`.
But the ZP overhead of empty switching is prevented !

Btw, the "pure" gos c implementation needs ~1% overhead only (but saves no ZP entries) !

One restriction: <16 tasks (main + 15 more) are possible due to the 256 byte stack restiction, so each task gets 16+ bytes min only.
Use 4 tasks if you basic program does e.g. PRINT, functions needing a bigger stack. Play...
If you restrict the code to peek/pokes only, you can use more tasks.
With simple tasks like `1000 POKE 1024+0,PEEK(1024+0)+1:GOTO 1000` i got 14 running.
You can change that with `--gos-tasks=4` or `--gos-stack=64` or with a pragma: `10 REM! GOS TASKS 16`.

Yes, i meassured a lot the last days.
I like numbers.
I can trust numbers.

Multitasking is fragile to use !
- No stack overflow is checked !
- Again: its (kind of) "stable" but still wip !

---

### The X16

`--target=x16` or `--x16` changes these things:
	- CPU is WDC 65C02, no "normal" illegals are used, but WDC specific opcodes
	- full (?) X16 basic is supported
	- Code defines: __X16__ and __65C02__
	- VERA is supported
	- TSB optimized for VERA
	- GOS optimized for X16 (still only 256 bytes stack !)
	- Host exe for X16 is possible, with "native" X16 basic dialect as well as with TSB "improved" [not yet]

---

### "Host"

**c-compiled**
You can simply run all your basic programs on the host.
Very convenient for a quick and fast test.
Again, `abc foobar c` to create c source instead of a prg.
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
Something like this works on the host:
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
	abc.exe breath.bas --avr=atmega328p
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
**`abc.exe manzoo.bas`**
	creates: `manzoo.prg` is the compiled version of the bas file
**`abc.exe -a manzoo.bas`**
	creates also: `manzooT.prg` is the tokenized version of the bas file

> Compile manzoo.bas to manzoo.c (and then to host manzoo.exe)
**`abc.exe manzoo.bas c`**
	creates: `manzoo.c`
**`gcc -O2 manzoo.c -o manzoo.exe`**
	creates: `manzoo.exe` ... Wow, compare manzoo.prg and manzoo.exe how technology has developed !
or
**`oscar64 _O3 manzoo.c -o=manzooO.prg`**
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
If you create the tokenized prg version with -a you get the "preprocessed" and combined result.

**Easy project setup:**
	`--init <dir>` creates a starter-environment with all you need: A demo program, the needed runtime.h, Makefile, ...
	`--init` checks the current dir for missing files.

**(some) Commandline options**
	`--verbose`			see which optimizers fire (or not)
	`--version`			e.g. "abc 2026-08-20 11:58:47"
	`--init [<dir>]`	create/init a new project
	`--strict`			use strict cbm basic v2
	`--no-illegals`		the compiler uses normally "verified" illegals to speed up
	`--hiram`			done automatically if needed
	`--annoying-stuff`	create tokenized prg and sidecars
	`--tsb`				support Tunes Simons Basic
	`--gos`				support multitasking
	`--gos-fast[=N]`	preempt every jiffy (bare) or every Nth (def 10 Hz)
	`--gos-tasks=N`		task slots, 2-16 (def 4, 15 is already 'bad')
	`--gos-stack=N`		per-task stack, 16-128 (def 64)
	`--avr[=mcu]`		compile for AVR
	`--source`			"c64v2" for now (x16 basic in x16 mode)
	`--target`			"c64" and "x16" for now
	`--opt`				none, speed, size, c
	`--romfloat`		use C64 rom float functions and not faster (but bigger) own ones
	`--no-pack`			dont pack binaries
	`--help`			helps
	and the usual -o

**"pragmas"** kind of...
	`REM! STRICT`       `--strict`
	`REM! NOILLEGALS`   `--no-illegals`
	`REM! TSB`          `--tsb`
	`REM! GOS`          `--gos`
	`REM! GOS TASKS N`  `--gos-tasks=N`
	`REM! GOS STACK N`  `--gos-stack=N`
	`REM! AVR=`         `--avr`
	`REM! SOURCE=`      `--source`
	`REM! TARGET=`      `--target`
	`REM! +SPEED|+SIZE|+RESET` (see 'Regions' below)

**Regions**
If memory gets low, you can select where to spend the valuable ram for speed:
	`10 REM! +SPEED` enables speed optimizer
	`20 ...`         do speedy stuff here
	`50 REM! +SIZE`  enables size optimizer
	`60 ...`         do packed stuff here
	`70 REM! +RESET` resets to default form commandline

**Packer**
You can choose differnet packers:
`--pack=0`	or `--no-pack`	no packing
`--pack=1`					LZ
`--pack=2`					ZX
`--pack=3`					WIP
Dont get confused by the packer sizes.
A small binary does not automatically means it fits in ram!
Check with `--no-pack` the real in-ram size (or look what abc says while compiling).

---

## Files

- `runtime.h`
	Only needed for OPTIMIZE_C, when compiling the "resúlt c file" !
	3 modes:
		- `TARGET_C64`
			supports cc65, oscar64, llvm-mos, gate -> creates *.prg files
			here a x16 is somehow a c64
		- `TARGET_HOST`
			supports gcc, msvc, gate, ...          -> creates *.exe files
		- `TARGET_AVR`
			supports avr-gcc, gate, ...            -> creates *.bin files

- `tsb.h`
	Just in case you want to run TSB programs on your host with `--tsb --opt c` ...

- `x16.h`
	[wip] run x16 prgs on host.

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
	100 balls, each its own color (char 81), bounce via GOSUB, prints the frame time.
- `life.bas`
	Conway's game of life.
	22x22, double-buffered (screen + shadow at $6000), 100 gens, counts neighbors by raw PEEK sum, prints gen time + alive.
- `matrix.bas`
	Well, Matrix...
	40 columns, white head fading to green tail, each column its own speed and glyph, frame-timed.

#### GOS

With multitasking it takes a while to get a feeling, so i added the GOS ovrhead.

- `gos_hello.bas`
	4 tasks: HELLO, WORLD, ! and TI (TI homes the cursor first).
	**~3% GOS, no TYIELD so only the rare 10 Hz preemption fires.**
- `gos_idle.bas`
	2 tasks, no TYIELD, just GOTO loops.
	**Best case, ~1% overhead.**
- `gos_yield.bas`
	2 tasks, only TYIELD.
	**Worst case, ~98% overhead.**
- `gos_color.bas`
	2 tasks randomize border + background forever.
	**~77% GOS, TYIELD every loop + RND.**
- `gos_input.bas`
	One task prints dots, another does INPUT/echo.
	**~4% GOS, INPUT blocks the task so it barely switches.**
- `gos_14.bas`
	14 counter tasks (main + 13), each increments one screen cell.
	**~1% GOS, no TYIELD, int POKE only -- pure preemption is cheap at 10 Hz.**
- `gos_14y.bas`
	Same as gos_14 but every task yields.
	**~93% GOS -- the TYIELD storm is the cost, not the task count.**
- `gos_15.bas`
	15 tasks (main + 14, the real max), each yields.
	**~93% GOS, same storm as 14y.**
- `gos_16.bas`
	16 tasks. Deliberately over the 15-task max, refused at compile time (safety check).
- `gos_balls.bas`
	4 bouncing balls, each its own speed (1..4 yields) and color.
	**~60% GOS, the TYIELD count sets the speed.**
- `gos_bars.bas`
	4-bar equalizer, triangle wave, each bar at its own TYIELD rate.
	**~14% GOS, each bar does a column of POKEs per switch so the work amortizes it.**
- `gos_scroll.bas`
	4 independent scrolling glyph streams, one task per row.
	**~47% GOS.**
- `gos_rain.bas`
	Matrix digital rain, 4 columns, each its own speed, color and glyph (cycles 33..90).
	**~74% GOS, the 4-column TYIELD storm.**
- `gos_stars.bas`
	Parallax starfield, 6 stars in 3 depth layers scrolling left at 3 speeds.
	**~28% GOS, relies on preemption -- jumps to 68% with `--gos-fast`.**
- `gos_maze.bas`
	2 tasks: one prints maze chars (the `CHR$(205.5+RND)` trick), the other prints TI and sleeps.
	**~39% GOS, TSLEEP-heavy so every-jiffy is actually cheaper (-31%).**
- `gos_maze2.bas`
	Maze variant: the second task yields instead of TSLEEP. BROKEN -- the TASYNC is commented out, so it runs single-task (~0% GOS). Uncomment line 20 to actually see it.
- `gos_lissajous.bas`
	3 Lissajous curves at once, each its own frequency.
	**~15% GOS looks low, but it crawls: float SIN per point (RT_fsub 38%) dominates, not the scheduler. The horrible one.**
- `gos_odo.bas`
	Odometer, 3 digit-counter tasks cascading carries, int only.
	**~5% GOS, no TYIELD, relies on preemption (-> 25% with `--gos-fast`).**
	Compile with `-DTASKS8 --gos-tasks=8` for 7 digits.
- `gos_worms.bas`
	3 worms race for 1 food pellet.
	**~74% GOS, TLOCK + TYIELD every step.**
- `gos_catch.bas`
	Player (cursor keys) flees a follower, a score task prints TI + points.
	**~67% GOS, follower sleeps 7 so it barely adds switches.**
- `gos_catch_NOLOCK.bas`
	Catch without TLOCK: tasks set pending-move flags, the main loop applies them.
	**~77% GOS, no TSLEEP (pure TYIELD) + float coords -> pricier than catch.**

### X16

see 

### Host

- `manzoo.bas`
	A super simple Mandelbrot Zoom.
	As prg too slow, as exe too fast.
	Fixed-point (no float), zooms 0.95x per frame through 5 preset coords, color = iterations AND 15.
	Cool as prg, impressive as exe!
- `bt2.bas`
	`bt.bas` is a raytracer ("by marco64 14.06.2008") !!
	I just added animation (wasd/crsr to move).
	This is not prerendered, zoom out for even more fps !
	Not 30h/frame (like bt.bas on the C64), but 70+ frames/sec ...
- `mirrorball3.bas`
	I found `mirrorball.tsb` on a disk named "raytr c't 1/86".
	I just added animation (wasd/crsr to move, space to drop).
- `juggler.bas`
	My try of an Amiga juggler, hahaha.
- `boing.bas`
	A try of a "3d-pong".
	For sure the slowest "game" ever (as prg) !
	But to fast to play (as exe).
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
	Demo of integrated assembler and host simulator (demo for -DSYS6502).
	Never tested with --avr, why should the AVR simulate a 6502 ?

### Games

Some ultra short and mostly really bad demos only, sorry.
All char-mode only (its just easier to debug for me).
Maybe i should add hires, sprite and sound things, rela games. Maybe later...
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

Due to less interest (from my side and community) AVR is "on ice" now.

---

## Future

Not sure yet.
- performance increase [WIP]
- X16 full support [WIP]
- C16/C128/... support (V3.5 / V7)
- more source dialects
- more targets
- mega65 ?
- 65816 / more X16 ?
- funny things like AmigaBasic for C64 ?
- let me know what you are interested in
- automatic fastloader ?
