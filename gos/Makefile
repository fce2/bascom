ABC = ../bin/win/abc.exe

NAMES = gos_14 gos_14y gos_catch gos_color gos_input gos_maze gos_odo gos_tsb_swarm

PRGS = $(addsuffix .prg,$(NAMES))

.PHONY: all clean

all: $(PRGS)

%.prg: %.bas
	$(ABC) $*.bas

clean:
	rm -f *.prg *.md
