100 REM  *** MIRRORBALL3 -- 3D GRAVITY CHROME BALLS, RAYTRACE ENGINE (bt2) ***
101 REM  six reflective spheres drop, bounce on floor/walls and off each other
102 REM  physically: gravity + impulse elastic collisions, mass = r*r
103 REM  keys: SPACE = drop a new sphere ; R = reset ; Q = quit
200 DIM KX(60),KY(60),KZ(60),OX(60),OY(60),OZ(60),KR(60),VX(60),VY(60),VZ(60),M(60)
201 AK = 5
202 BX = 0 : BY = 24 : BZ = - 50 : BE = 25
203 LX = - 50 : LY = 300 : LZ = 50
204 GB = 26
205 G = 0.25 : RE = 0.99 : GR = 0.05
206 WL = 18 : ZL = 38 : ZR = 82 : CH = 50
207 POKE 53272,11 : POKE 53265,59 : POKE 56576,148
250 FR = 0 : CF = 0
255 GOSUB 1200
260 GOSUB 2000
262 CC = COS (CA) : SS = SIN (CA)
263 FOR I = 0 TO AK : KX(I)= OX(I)*CC - (OZ(I)-60)*SS : KZ(I)= 60 + OX(I)*SS + (OZ(I)-60)*CC : KY(I)= OY(I) : NEXT I
265 IF CF = 0 THEN 300
266 GOTO 304
300 FOR I = 1024 TO 2023
301 POKE 48128 + I,15
302 POKE 54272 + I,0
303 NEXT :CF = 1
304 FOR PB =  0 TO 8000
306 ZW = PB AND 7
307 Y = INT (PB / 320)* 8 + ZW
308 ZW = INT (PB / 8)
309 ZX = (ZW - INT (ZW / 40)* 40)* 8
310 PW = 0
400 FOR PX = 0 TO 7
401 X = ZX + PX
402 ZW = GB / 320
403 RX = X * ZW - GB / 2
404 RY = - Y * ZW + 200 / 2 * ZW
405 RZ = BE
406 ZW = SQR (RX * RX + RY * RY + RZ * RZ)
407 RX = RX / ZW
408 RY = RY / ZW
409 RZ = RZ / ZW
410 OX = BX
411 OY = BY
412 OZ = BZ
413 RD = 0
500 GOSUB 1000
501 ON - ((OY + T * RY)> 0)GOTO 600
502 LA = - OY / RY
503 SX = OX + LA * RX
504 SY = 0
505 SZ = OZ + LA * RZ
506 ZW = (INT (SX / 5)AND 1)+ (INT (SZ / 5)AND 1)
507 FA = (ZW = 1)+ 1
508 OX = SX
509 OY = SY
510 OZ = SZ
511 RX = OX + LX
512 RY = OY + LY
513 RZ = OZ + LZ
514 GOSUB 1000
515 ON - (K = - 1)GOTO 900
516 FA = ((X AND 1)< > (Y AND 1))+ 1
517 GOTO 900
600 IF K = - 1 THEN FA = 0:GOTO 900
700 X1 = KX(K)* RX + KY(K)* RY + KZ(K)* RZ
701 X2 = OX * RX + OY * RY + OZ * RZ
702 X3 = RX * RX + RY * RY + RZ * RZ
703 LA = (X1 - X2)/ X3
704 F1 = (OX + LA * RX)- KX(K)
705 F2 = (OY + LA * RY)- KY(K)
706 F3 = (OZ + LA * RZ)- KZ(K)
707 D = SQR (F1 * F1 + F2 * F2 + F3 * F3)
708 IF (KR(K)* .95 < D)THEN FA = 1:GOTO 900
710 RD = RD + 1: IF RD > 3 THEN FA = 1:GOTO 900
800 OX = OX + T * RX
801 OY = OY + T * RY
802 OZ = OZ + T * RZ
803 NX = KX(K)- OX
804 NY = KY(K)- OY
805 NZ = KZ(K)- OZ
806 ZW = NX * RX + NY * RY + NZ * RZ
807 RX = RX - 2 * ZW * NX
808 RY = RY - 2 * ZW * NY
809 RZ = RZ - 2 * ZW * NZ
810 ZW = SQR (RX * RX + RY * RY + RZ * RZ)
811 RX = RX / ZW
812 RY = RY / ZW
813 RZ = RZ / ZW
814 GOTO 500
900 PW = PW * 2 + FA
901 NEXT PX
902 POKE 57344 + PB,PW
903 NEXT PB
904 FR = FR + 1
905 GET A$
906 IF A$ = " " THEN GOSUB 1400
907 IF A$ = "R" OR A$ = "r" THEN GOSUB 1200
908 IF A$ = "Q" OR A$ = "q" THEN 920
909 IF A$ = CHR$(29) THEN CA = CA + 0.1
910 IF A$ = CHR$(157) THEN CA = CA - 0.1
911 IF A$ = CHR$(145) THEN BY = BY + 1
912 IF A$ = CHR$(17) THEN BY = BY - 1
913 IF A$ = "w" OR A$ = "W" THEN BZ = BZ + 1
914 IF A$ = "s" OR A$ = "S" THEN BZ = BZ - 1
915 IF A$ = "a" OR A$ = "A" THEN BX = BX - 1
916 IF A$ = "d" OR A$ = "D" THEN BX = BX + 1
917 GOTO 260
920 END
1000 K = - 1
1001 T = 99999
1002 FOR I = 0 TO AK
1003 ZW = RX * RX + RY * RY + RZ * RZ
1004 P = (2 * (RX * (OX - KX(I))+ RY * (OY - KY(I))+ RZ * (OZ - KZ(I))))/ ZW
1005 Q = ((OX - KX(I))* (OX - KX(I))+ (OY - KY(I))* (OY - KY(I))+ (OZ - KZ(I))* (OZ - KZ(I))- KR(I)* KR(I))/ ZW
1006 Z1 = - P / 2
1007 Z2 = ((P * P)/ 4 - Q)
1008 IF Z2 < 0 THEN NEXT :RETURN
1100 WU = SQR (Z2)
1101 X1 = Z1 + WU
1102 X2 = Z1 - WU
1103 Z0 = - ((X1 > .001)AND (X1 < T))
1104 Z1 = - Z0 + 1
1105 K = Z0 * I + Z1 * K
1106 T = Z0 * X1 + Z1 * T
1107 Z0 = - ((X2 > .001)AND (X2 < T))
1108 Z1 = - Z0 + 1
1109 K = Z0 * I + Z1 * K
1110 T = Z0 * X2 + Z1 * T
1111 NEXT
1112 RETURN
1200 REM ---------- init 1 sphere ----------
1210 AK = 0
1220 FOR I = 0 TO AK
1230   KR(I) = 3 + INT (RND (1)* 5)
1240   OX(I) = - 16 + RND (1)* 32
1250   OZ(I) = 40 + RND (1)* 40
1260   OY(I) = 35 + RND (1)* 10
1270   VX(I) = (RND (1)* 2 - 1)* 2.5
1280   VY(I) = 0
1290   VZ(I) = (RND (1)* 2 - 1)* 2.5
1300   M(I) = KR(I)* KR(I)
1310 NEXT I
1320 CF = 0
1330 RETURN
1400 REM ---------- drop a new sphere ----------
1410 IF AK >= 59 THEN RETURN
1420 AK = AK + 1 : I = AK
1430 KR(I) = 3 + INT (RND (1)* 5)
1440 OX(I) = - 16 + RND (1)* 32
1450 OZ(I) = 40 + RND (1)* 40
1460 OY(I) = 50
1470 VX(I) = (RND (1)* 2 - 1)* 2.5 : VY(I) = 0 : VZ(I) = (RND (1)* 2 - 1)* 2.5
1480 M(I) = KR(I)* KR(I)
1490 RETURN
2000 REM ---------- 3D physics: gravity + integrate + walls + floor ----------
2010 FOR I = 0 TO AK
2020   VY(I) = VY(I) - G
2030   OX(I) = OX(I) + VX(I)
2040   OY(I) = OY(I) + VY(I)
2050   OZ(I) = OZ(I) + VZ(I)
2060   IF OX(I) < - WL THEN OX(I) = - WL : VX(I) = - VX(I)* RE
2070   IF OX(I) >  WL THEN OX(I) =  WL : VX(I) = - VX(I)* RE
2080   IF OZ(I) <  ZL THEN OZ(I) =  ZL : VZ(I) = - VZ(I)* RE
2090   IF OZ(I) >  ZR THEN OZ(I) =  ZR : VZ(I) = - VZ(I)* RE
2100   IF OY(I) < KR(I) THEN OY(I) = KR(I) : VY(I) = - VY(I)* RE
2110   IF OY(I) >= KR(I) - .5 AND ABS (VY(I)) < GR THEN VY(I) = 0 : VX(I) = VX(I)* .997 : VZ(I) = VZ(I)* .997
2120 NEXT I
2200 REM ---------- ball-ball 3D elastic collisions (impulse) ----------
2210 FOR I = 0 TO AK
2220   FOR J = I + 1 TO AK
2230     DX = OX(J)- OX(I) : DY = OY(J)- OY(I) : DZ = OZ(J)- OZ(I)
2240     D = SQR (DX * DX + DY * DY + DZ * DZ)
2250     RS = KR(I)+ KR(J)
2260     IF D >= RS THEN 2360
2270     IF D < .01 THEN D = .01 : DX = .01 : DY = 0 : DZ = 0
2280     NX = DX / D : NY = DY / D : NZ = DZ / D
2290     OV = RS - D
2295     MT = M(I) + M(J) : FI = M(J) / MT : FJ = M(I) / MT
2300     OX(I) = OX(I)- NX * OV * FI : OY(I) = OY(I)- NY * OV * FI : OZ(I) = OZ(I)- NZ * OV * FI
2310     OX(J) = OX(J)+ NX * OV * FJ : OY(J) = OY(J)+ NY * OV * FJ : OZ(J) = OZ(J)+ NZ * OV * FJ
2320     RX = VX(J)- VX(I) : RY = VY(J)- VY(I) : RZ = VZ(J)- VZ(I)
2330     VN = RX * NX + RY * NY + RZ * NZ
2340     IF VN >= 0 THEN 2360
2350     IM = - (1 + RE)* VN / (1 / M(I)+ 1 / M(J))
2352     VX(I) = VX(I)- IM * NX / M(I) : VY(I) = VY(I)- IM * NY / M(I) : VZ(I) = VZ(I)- IM * NZ / M(I)
2354     VX(J) = VX(J)+ IM * NX / M(J) : VY(J) = VY(J)+ IM * NY / M(J) : VZ(J) = VZ(J)+ IM * NZ / M(J)
2360   NEXT J
2370 NEXT I
2380 RETURN