ABC = ../bin/win/abc.exe

EXES = boing.exe bt2.exe juggler.exe manzoo.exe mirrorball3.exe povray.exe

.PHONY: all exes clean

all: $(EXES)

exes: $(EXES)

%.prg: %.bas
	$(ABC) $*.bas

%.exe: %.bas ..\tsb.h ..\runtime.h
	$(ABC) $*.bas c
	gcc -O2 -static -s -I .. $*.c -o $*.exe -mwindows -lgdi32 -luser32
	-upx -9 $*.exe

clean:
	rm -f *.prg *.exe *.m64 *.asm *.int *.lbl *.map *.disasm *.err *.txt *.log *.out bench.c *.xref *.c *.seq
