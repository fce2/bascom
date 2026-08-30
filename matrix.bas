10 rem *** matrix digital rain - c64 ***
20 rem white head, light-green body, green tail

30 poke 53280,0:poke 53281,0
40 print chr$(147)

50 sc=1024:cr=55296
60 dim h(39):dim e(39):dim t(39):dim ch(39):dim d(39)

70 for c=0 to 39
80 h(c)=int(rnd(1)*25)
90 t(c)=5+int(rnd(1)*10)
100 e(c)=h(c)-t(c):if e(c)<0 then e(c)=e(c)+25
110 ch(c)=33+int(rnd(1)*58)
120 d(c)=1+int(rnd(1)*5)
130 next c

140 T%=TI
141 for c=0 to 39:gosub 1000:next c
142 PRINT CHR$(19);TI-T%
150 goto 140

1000 rem --- advance column c by 1 row ---
1001 p=h(c)*40+c
1002 poke sc+p,ch(c):poke cr+p,1
1003 ch(c)=ch(c)+d(c):if ch(c)>90 then ch(c)=ch(c)-58
1004 b=h(c)-1:if b<0 then b=24
1005 poke cr+b*40+c,13
1006 b=b-1:if b<0 then b=24
1007 poke cr+b*40+c,5
1008 poke sc+e(c)*40+c,32
1009 h(c)=h(c)+1:if h(c)>24 then h(c)=0:ch(c)=33+int(rnd(1)*58):d(c)=1+int(rnd(1)*5)
1010 e(c)=e(c)+1:if e(c)>24 then e(c)=0
1011 return
