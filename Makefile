ifeq ($(wildcard bin/win/abc.exe),)
ABC = bin/win/abcd.exe
else
ABC = bin/win/abc.exe
endif

.PHONY: all exes sub subexes clean

all: init sub \
	balls.prg life.prg matrix.prg \
	ballsM.prg lifeM.prg matrixM.prg

init:
	$(ABC) --init --tsb
	$(ABC) --init --avr
	$(ABC) --init --x16
	rm release.bas

sub:
	make -C host all
	make -C misc all
	make -C games all
	make -C tsb all
	make -C gos all

exes: subexes balls.exe life.exe matrix.exe

subexes:
	make -C host exes
	make -C misc exes
	make -C games exes

%.prg: %.bas
	$(ABC) -a $*.bas

%M.prg: %.bas
	$(ABC) -a $*.bas size -o $*M.prg

%.exe: %.bas tsb.h runtime.h
	$(ABC) -a $*.bas c
	gcc -O2 -static -s -I . $*.c -o $*.exe -mwindows -lgdi32 -luser32
	-upx -9 $*.exe

clean:
	make -C host clean
	make -C misc clean
	make -C games clean
	make -C tsb clean
	make -C gos clean
	rm -f balls.prg balls.exe life.prg life.exe matrix.prg matrix.exe *.prg *.bak runtime.h tsb.h x16.h avr-crt0.S README.md License
	rm -f chargen.901225-01.bin games\chargen.901225-01.bin misc\chargen.901225-01.bin gos\chargen.901225-01.bin tsb\chargen.901225-01.bin host\chargen.901225-01.bin
	rm -rf scratch bin
