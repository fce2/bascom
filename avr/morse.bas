10 REM morse.bas -- flash PB5 LED in morse code with the C64 boot text
20 REM target: ATmega328p (Arduino Uno) @16MHz, on-board LED PB5 (pin 13)
30 REM dot length = N*H*G POKE iterations. Tune H,G (bigger=slower).

40 H = 250
50 G = 250
60 POKE DDRB, 32
70 DIM MC(91)

80 REM --- morse table: MC(ascii)=(len<<6)|pattern, dash=1 dot=0, MSB first ---
90 MC(42) = 316 : REM *  = ----   (custom: four dashes; not ITU standard)
100 MC(46) = 405 : REM .  = .-.-.-
110 MC(49) = 350 : REM 1  = .----
120 MC(50) = 334 : REM 2  = ..---
130 MC(51) = 326 : REM 3  = ...--
140 MC(52) = 322 : REM 4  = ....-
150 MC(54) = 352 : REM 6  = -....
160 MC(56) = 376 : REM 8  = ---..
170 MC(57) = 380 : REM 9  = ----.
180 MC(65) = 144 : REM A  = .-
190 MC(66) = 288 : REM B  = -...
200 MC(67) = 296 : REM C  = -.-.
210 MC(68) = 224 : REM D  = -..
220 MC(69) = 64  : REM E  = .
230 MC(70) = 264 : REM F  = ..-.
240 MC(73) = 128 : REM I  = ..
250 MC(75) = 232 : REM K  = -.-
260 MC(77) = 176 : REM M  = --
270 MC(79) = 248 : REM O  = ---
280 MC(82) = 208 : REM R  = .-.
290 MC(83) = 192 : REM S  = ...
300 MC(84) = 96  : REM T  = -
310 MC(86) = 258 : REM V  = ...-
320 MC(89) = 300 : REM Y  = -.--

330 REM --- message: C64 power-on screen, readable text, "" = end ---
340 DATA "     **** COMMODORE 64 BASIC V2 ****    "
350 DATA "                                        "
360 DATA " 64K RAM SYSTEM  38911 BASIC BYTES FREE "
370 DATA "                                        "
380 DATA "READY."
390 DATA ""

400 REM --- main: CLR pool each pass, READ text lines, morse each char ---
410 PS = 0
420 CLR
430 RESTORE
440 READ S$
450 IF LEN(S$) = 0 THEN GOTO 420
460 FOR K = 1 TO LEN(S$)
470 C = ASC(MID$(S$,K,1))
480 IF C <> 32 THEN GOTO 510
490 IF PS = 0 THEN LV = 0 : N = 4 : GOSUB 1000 : PRINT
500 PS = 1 : GOTO 650
510 PS = 0
520 V = MC(C)
530 IF V = 0 THEN GOTO 650
540 L = INT(V / 64)
550 P = V - L * 64
560 M = 32
570 FOR J = 1 TO L
580 D = 1
590 IF P AND M THEN D = 3
595 IF D = 1 THEN PRINT ".";
597 IF D = 3 THEN PRINT "-";
600 LV = 32 : N = D : GOSUB 1000
610 LV = 0 : N = 1 : GOSUB 1000
620 M = INT(M / 2)
630 NEXT J
640 LV = 0 : N = 2 : GOSUB 1000
645 PRINT " ";
650 NEXT K
660 GOTO 440

1000 REM DELAY N units; LED held at LV (N*H*G POKEs, each bound < 32768)
1001 FOR W = 1 TO N
1002 FOR Q = 1 TO H
1003 FOR R = 1 TO G
1004 POKE PORTB, LV
1005 NEXT R
1006 NEXT Q
1007 NEXT W
1008 RETURN
