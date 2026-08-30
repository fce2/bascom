ABC = ../bin/win/abc.exe

.PHONY: all exes clean

all: flappy.prg frogger.prg invaders.prg ladder.prg landscape.prg meteors.prg minitetris.prg nethack.prg scramble.prg swarm.prg tetris.prg wolf.prg

exes: flappy.exe frogger.exe invaders.exe ladder.exe landscape.exe meteors.exe minitetris.exe nethack.exe scramble.exe swarm.exe tetris.exe wolf.exe

%.prg: %.bas
	$(ABC) $*.bas

%T.prg: %.bas
	$(ABC) $*.bas -a

%M.prg: %.bas
	$(ABC) $*.bas size -o $*M.prg

%.exe: %.bas ..\tsb.h ..\runtime.h
	$(ABC) $*.bas c
	gcc -O2 -static -s -I .. -DABC_AUTORUN $*.c -o $*.exe -mwindows -lgdi32 -luser32
	-upx -9 $*.exe

clean:
	rm -f *.prg *.exe *.m64 *.asm *.int *.lbl *.map *.disasm *.err *.txt *.log *.out bench.c *.xref *.c *.seq
