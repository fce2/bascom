10 rem === landscape: top-down 3d heightmap, endless valleys, border scroll ===
11 rem interactive: w/a/s/d move, q quit.  terrain = sum of 3 sines via int
12 rem LUT (no rnd, no sin rom), so interpreter and compiled show the identical
13 rem map.  the view scrolls (pure-bascom memmove of screen+color) only when
14 rem the player reaches a border deadzone (cols 6..33, rows 6..18).  endless.
15 print chr$(147)
20 dim sl%(63):gosub 9600:poke 650,128:dm%=0
25 rem (sl% dim moved to line 20, before gosub 9600)
28 gosub 8300
30 ox%=0:oy%=0:sx%=20:sy%=12
40 gosub 1000
45 gosub 2100
46 gosub 8500
70 wait 53265,128:K%=PEEK(197):POKE 198,0
72 if K%<>64 then dm%=0
75 if K%=62 then sys 64738
78 if dm%=1 then gosub 8400
80 dx%=0:dy%=0
85 if K%=18 then dx%=1
90 if K%=10 then dx%=-1
95 if K%=13 then dy%=1
100 if K%=9 then dy%=-1
105 if dx%=0 and dy%=0 then 70
110 gosub 2000
115 sx%=sx%+dx%:sy%=sy%+dy%
120 gosub 3000
125 gosub 2100
127 gosub 8500
130 rem
135 goto 70
1000 rem === full render: direct LUT per cell (24 rows x 40 cols; row 24=HUD) ===
1005 for r=0 to 23
1008 iy%=oy%+r:ra=r*40
1015 for c=0 to 39
1020 ix%=ox%+c
1025 gosub 8000:gosub 7000
1030 poke 1024+ra+c,ch%:poke 55296+ra+c,cl%
1035 next c
1040 next r
1045 return
2000 rem === erase player: restore saved terrain at (sx%,sy%) ===
2005 poke 1024+sy%*40+sx%,uc%:poke 55296+sy%*40+sx%,ul%
2010 return
2100 rem === draw player at (sx%,sy%): save cell, then draw marker ===
2105 uc%=peek(1024+sy%*40+sx%):ul%=peek(55296+sy%*40+sx%)
2110 poke 1024+sy%*40+sx%,88:poke 55296+sy%*40+sx%,10
2115 return
3000 rem === scroll handler: deadzone border scroll (pure bascom memmove) ===
3005 if sx%>31 then gosub 3100:ox%=ox%+1:sx%=sx%-1:cc=39:wx%=ox%+39:gosub 5000
3010 if sx%<8 then gosub 3200:ox%=ox%-1:sx%=sx%+1:cc=0:wx%=ox%:gosub 5000
3015 if sy%>16 then gosub 3300:oy%=oy%+1:sy%=sy%-1:rr=23:wy%=oy%+23:gosub 6000
3020 if sy%<8 then gosub 3400:oy%=oy%-1:sy%=sy%+1:rr=0:wy%=oy%:gosub 6000
3025 return
3100 rem scroll-left: shift screen+color cols 1..39 -> 0..38, expose col 39
3105 for r=0 to 23
3110 ra=r*40
3115 for c=0 to 38
3120 poke 1024+ra+c,peek(1024+ra+c+1)
3125 poke 55296+ra+c,peek(55296+ra+c+1)
3130 next c
3135 next r
3140 return
3200 rem scroll-right: shift screen+color cols 0..38 -> 1..39, expose col 0
3205 for r=0 to 23
3210 ra=r*40
3215 for c=38 to 0 step -1
3220 poke 1024+ra+c+1,peek(1024+ra+c)
3225 poke 55296+ra+c+1,peek(55296+ra+c)
3230 next c
3235 next r
3240 return
3300 rem scroll-up: shift screen+color rows 1..23 -> 0..22, expose row 23
3305 for r=0 to 22
3310 ra=r*40:rb=(r+1)*40
3315 for c=0 to 39
3320 poke 1024+ra+c,peek(1024+rb+c)
3325 poke 55296+ra+c,peek(55296+rb+c)
3330 next c
3335 next r
3340 return
3400 rem scroll-down: shift screen+color rows 0..22 -> 1..23, expose row 0
3405 for r=23 to 1 step -1
3410 ra=r*40:rb=(r-1)*40
3415 for c=0 to 39
3420 poke 1024+ra+c,peek(1024+rb+c)
3425 poke 55296+ra+c,peek(55296+rb+c)
3430 next c
3435 next r
3440 return
5000 rem === drawcol: screen col cc, world x=wx% (24 rows, direct LUT) ===
5005 ix%=wx%
5010 for r=0 to 23
5015 iy%=oy%+r:ra=r*40
5020 gosub 8000:gosub 7000
5025 poke 1024+ra+cc,ch%:poke 55296+ra+cc,cl%
5030 next r
5035 return
6000 rem === drawrow: screen row rr, world y=wy% (40 cols, direct LUT) ===
6005 iy%=wy%:ra=rr*40
6010 for c=0 to 39
6015 ix%=ox%+c
6020 gosub 8000:gosub 7000
6025 poke 1024+ra+c,ch%:poke 55296+ra+c,cl%
6030 next c
6035 return
7000 rem === band: ht%(0..60) -> ch%, cl% (7 height bands) ===
7005 ch%=35:cl%=1
7010 if ht%<10 then ch%=32:cl%=6:return
7015 if ht%<20 then ch%=46:cl%=14:return
7020 if ht%<30 then ch%=58:cl%=5:return
7025 if ht%<40 then ch%=45:cl%=13:return
7030 if ht%<50 then ch%=43:cl%=8:return
7035 if ht%<60 then ch%=42:cl%=7:return
7040 return
8000 rem === gh: height at (ix%,iy%) via sine LUT -> ht% (0..60) ===
8005 ht%=sl%((ix%*3) and 63)+sl%((iy%*4) and 63)+sl%(((ix%+iy%)*2) and 63)+30
8010 return
8300 rem === title screen ===
8305 print chr$(147)
8310 print "                LANDSCAPE"
8315 print:print "        W/A/S/D PAN  Q=QUIT"
8320 print:print "      EXPLORE ENDLESS TERRAIN"
8325 print:print "      SUM OF 3 SINES  NO RND"
8330 print:print "        SPACE TO PLAY  D=DEMO"
8335 for gd%=1 to 200
8337 wait 53265,128:wait 53265,128,128
8340 K%=PEEK(197):POKE 198,0
8345 if K%=18 then dm%=1:goto 8390
8350 if K%<>64 then dm%=0:goto 8390
8355 next gd%
8360 dm%=1
8390 print chr$(147)
8395 return
8400 rem === demo ai -> K% (auto-pan random) ===
8405 K%=64
8410 rr%=int(rnd(1)*4)
8415 if rr%=0 then K%=9:return
8420 if rr%=1 then K%=10:return
8425 if rr%=2 then K%=13:return
8430 K%=18:return
8500 rem === hud row 24 ===
8505 sa%=1024+24*40:sb%=55296+24*40
8510 for i=0 to 39:poke sa%+i,32:poke sb+i,14:next i
8515 tx%=0:ty%=24:t$="WASD PAN Q=QUIT":gosub 8600
8520 tx%=16:t$="X=":gosub 8600:tx%=tx%+2:vv%=ox%:gosub 8700:tx%=tx%+5
8525 tx%=tx%+1:t$="Y=":gosub 8600:tx%=tx%+2:vv%=oy%:gosub 8700
8530 if dm%=1 then tx%=32:t$="DEMO":gosub 8600
8535 return
8600 rem === poke t$ at (tx%,ty%) as screen codes ===
8605 if t$="" then return
8610 sa%=1024+ty%*40+tx%
8615 for i=1 to len(t$)
8620 qz%=asc(mid$(t$,i,1))
8625 if qz%>=64 then if qz%<=95 then qz%=qz%-64
8630 poke sa%,qz%:sa%=sa%+1
8635 next i
8640 return
8700 rem === poke vv% as sign+4 digits (5 cols) at tx%,ty% row 24 ===
8705 sg%=0:if vv%<0 then sg%=1:vv%=0-vv%
8710 d4%=vv%/1000:d3%=(vv%-d4%*1000)/100:d2%=(vv%-d4%*1000-d3%*100)/10:d1%=vv%-d4%*1000-d3%*100-d2%*10
8715 sa%=1024+24*40+tx%:if sg%=1 then poke sa%,45:poke sa%+1,d4%+48:poke sa%+2,d3%+48:poke sa%+3,d2%+48:poke sa%+4,d1%+48:return
8720 poke sa%,32:poke sa%+1,d4%+48:poke sa%+2,d3%+48:poke sa%+3,d2%+48:poke sa%+4,d1%+48
8725 return
9600 rem === sine LUT init (64 entries = sin(2*pi*i/64)*10, period 64) ===
9605 for i=0 to 63:read u0%:sl%(i)=u0%:next i
9610 return
9620 data 0,1,2,3,4,5,6,6,7,8,8,9,9,10,10,10
9625 data 10,10,10,10,9,9,8,8,7,6,6,5,4,3,2,1
9630 data 0,-1,-2,-3,-4,-5,-6,-6,-7,-8,-8,-9,-9,-10,-10,-10
9635 data -10,-10,-10,-10,-9,-9,-8,-8,-7,-6,-6,-5,-4,-3,-2,-1