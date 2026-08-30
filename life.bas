 10 PRINT chr$(147)
 20 tt = TI
 30 w = 20
 40 g = 100
 50 l = 1113
 60 n = 24576
 70 d = 23463

 80 FOR r = 0 TO 21
 90   FOR c = 0 TO 21
100     POKE l + r * 40 + c,32
110     POKE n + r * 40 + c,32
120   NEXT c
130 NEXT r
140 z = RND(-246)
150 FOR r = 1 TO w
160   FOR c = 1 TO w
170     IF RND(1) < 0.3 THEN POKE l + r * 40 + c,81
180   NEXT c
190 NEXT r

200 FOR k = 1 TO g
210   t% = ti
220   FOR x = 1 TO w
230     bb = l + x * 40 + 1
240     FOR y = 1 TO w
250       c  = PEEK(bb - 41) + PEEK(bb - 40) + PEEK(bb - 39) + PEEK(bb - 1) + PEEK(bb + 1) + PEEK(bb + 39) + PEEK(bb + 40) + PEEK(bb + 41)
260       p  = PEEK(bb)
270       nb = bb + d
280       POKE nb,32
290       IF c = 403 OR (p = 81 AND c = 354) THEN POKE nb,81
300       bb = bb + 1
310     NEXT y
320   NEXT x
330   FOR r = 1 TO w
340     FOR c = 1 TO w
350       POKE l + r * 40 + c,PEEK(n + r * 40 + c)
360     NEXT c
370   NEXT r
380   POKE 1024,32
390   PRINT chr$(19);ti - t%
400 NEXT k

410 s = 0
420 FOR r = 1 TO w
430   FOR c = 1 TO w
440     s = s - (PEEK(l + r * 40 + c) = 81)
450   NEXT c
460 NEXT r

470 PRINT "GENS=";g
480 PRINT "ALIVE=";s
490 PRINT "TIME=";TI - tt
