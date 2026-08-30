   10 REM  *** POV-RAY -- SCENE-DESCRIPTION RAYTRACER (bt2 engine) ***
   20 REM  set up a scene POV-Ray style: camera, light_source, plane,
   30 REM  spheres, boxes. reflective + matte materials, shadows,
   40 REM  recursive reflections. scenes are DATA -- edit lines 11000+.
   50 REM  keys: pick a scene at menu ; L/R turn ; U/D up/down ; WASD walk ; Q quit
   60 REM  ============================================================
   70 REM  >>> EDIT THE SCENE: lines 11000+ (end of file)        <<<
   80 REM  >>>   DATA "scene","NAME" ... keyword, params ... "end"  <<<
   85 REM  >>>   DATA "animate",N,spd + N x DATA "kf",x,y,z = camera dive <<<
   90 REM  ============================================================
  100 REM  CBM vars are 2 chars -- array names use distinct 2nd letters.
  110 REM  --- scenes are DATA (POV-Ray style): see lines 11000+ ---

  120 BE = 25
  130 GB = 26
  140 DIM SX(47),SY(47),SZ(47),SR(47),SM(47)
  145 DIM KX(63),KY(63),KZ(63)
  146 DIM SN$(15)
  150 DIM UX(199),UY(199),UZ(199),VX(199),VY(199),VZ(199)
  160 NS = - 1
  170 NB = - 1
  175 KC = 0 : NK = - 1 : AM = 0 : AP = 0 : SP = 0 : SC = 0
  180 GOSUB 10900
  190 POKE 53272,11
  200 POKE 53265,59
  210 POKE 56576,148
  220 FR = 0
  230 CF = 0

  240 IF AM = 0 THEN 250
  242 AP = AP + SP : IF AP >= KC THEN AP = AP - KC
  244 GOSUB 10940
  250 CC = COS (CA) : SS = SIN (CA)
  260 IF CF = 0 THEN 280
  270 GOTO 330

  280 FOR I = 1024 TO 2023
  290   POKE 48128 + I,15
  300   POKE 54272 + I,0
  310 NEXT
  320 CF = 1

  330 FOR PB =  0 TO 8000
  340   ZW = PB AND 7
  350   Y = INT (PB / 320)* 8 + ZW
  360   ZW = INT (PB / 8)
  370   ZX = (ZW - INT (ZW / 40)* 40)* 8
  380   PW = 0
  390   FOR PX = 0 TO 7
  400     X = ZX + PX
  410     ZW = GB / 320
  420     RX = X * ZW - GB / 2
  430     RY = - Y * ZW + 200 / 2 * ZW
  440     RZ = BE
  450     ZW = SQR (RX * RX + RY * RY + RZ * RZ)
  460     RX = RX / ZW
  470     RY = RY / ZW
  480     RZ = RZ / ZW
  490     RW = RX * CC + RZ * SS
  500     RZ = - RX * SS + RZ * CC
  510     RX = RW
  520     OX = EX
  530     OY = EY
  540     OZ = EZ
  550     RD = 0
  560     GOSUB 10000
  570     GOSUB 10200
  580     TF = 99999
  590     IF RY < 0 THEN TF = (GY - OY) / RY
  600     HT = TF
  610     HM = 0
  620     IF T < HT THEN HT = T : HM = 1
  630     IF BT < HT THEN HT = BT : HM = 2
  640     IF HT >= 99999 THEN FA = 0 : GOTO 1280
  650     IF HM = 0 THEN 680
  660     IF HM = 1 THEN 790
  670     GOTO 10720

  680     REM  ---- plane: checker + shadow ----
  690     FX = OX + TF * RX
  700     FZ = OZ + TF * RZ
  710     ZW = (INT (FX / CS) AND 1) + (INT (FZ / CS) AND 1)
  720     FA = (ZW = 1) + 1
  730     OX = FX
  740     OY = GY
  750     OZ = FZ
  760     GOSUB 10500
  770     IF SH = 1 THEN FA = 0
  780     GOTO 1280

  790     REM  ---- sphere hit ----
  800     HX = OX + T * RX
  810     HY = OY + T * RY
  820     HZ = OZ + T * RZ
  830     NX = (HX - SX(K)) / SR(K)
  840     NY = (HY - SY(K)) / SR(K)
  850     NZ = (HZ - SZ(K)) / SR(K)
  860     IF SM(K) = 0 THEN 1110
  870     REM  reflective: rim fade then reflect
  880     X1 = SX(K)* RX + SY(K)* RY + SZ(K)* RZ
  890     X2 = OX * RX + OY * RY + OZ * RZ
  900     X3 = RX * RX + RY * RY + RZ * RZ
  910     LA = (X1 - X2)/ X3
  920     F1 = (OX + LA * RX)- SX(K)
  930     F2 = (OY + LA * RY)- SY(K)
  940     F3 = (OZ + LA * RZ)- SZ(K)
  950     D = SQR (F1 * F1 + F2 * F2 + F3 * F3)
  960     IF (SR(K)* .95 < D)THEN FA = 1:GOTO 1280
  970     RD = RD + 1
  980     IF RD > 3 THEN FA = 1:GOTO 1280
  990     ZW = NX * RX + NY * RY + NZ * RZ
 1000     RX = RX - 2 * ZW * NX
 1010     RY = RY - 2 * ZW * NY
 1020     RZ = RZ - 2 * ZW * NZ
 1030     ZW = SQR (RX * RX + RY * RY + RZ * RZ)
 1040     RX = RX / ZW
 1050     RY = RY / ZW
 1060     RZ = RZ / ZW
 1070     OX = HX
 1080     OY = HY
 1090     OZ = HZ
 1100     GOTO 560

 1110     REM  matte: diffuse lit by light_source (with shadow)
 1120     RX = LX - HX
 1130     RY = LY - HY
 1140     RZ = LZ - HZ
 1150     ZW = SQR (RX * RX + RY * RY + RZ * RZ)
 1160     RX = RX / ZW
 1170     RY = RY / ZW
 1180     RZ = RZ / ZW
 1190     NL = NX * RX + NY * RY + NZ * RZ
 1200     IF NL <= 0 THEN FA = 0 : GOTO 1280
 1210     OX = HX
 1220     OY = HY
 1230     OZ = HZ
 1240     GOSUB 10500
 1250     IF SH = 1 THEN FA = 0 : GOTO 1280
 1260     FA = 1
 1270     GOTO 1280

 1280     PW = PW * 2 + FA
 1290   NEXT PX
 1300   POKE 57344 + PB,PW
 1310 NEXT PB
 1320 FR = FR + 1
 1330 GET A$
 1332 IF A$ = "q" OR A$ = "Q" THEN 1440
 1334 IF AM = 1 THEN 240
 1340 IF A$ = CHR$(29) THEN CA = CA + 0.1 : GOTO 240
 1350 IF A$ = CHR$(157) THEN CA = CA - 0.1 : GOTO 240
 1360 IF A$ = CHR$(145) THEN EY = EY + 1 : GOTO 240
 1370 IF A$ = CHR$(17) THEN EY = EY - 1 : GOTO 240
 1380 IF A$ = "w" OR A$ = "W" THEN EX = EX + SS : EZ = EZ + CC : GOTO 240
 1390 IF A$ = "s" OR A$ = "S" THEN EX = EX - SS : EZ = EZ - CC : GOTO 240
 1400 IF A$ = "a" OR A$ = "A" THEN EX = EX - CC : EZ = EZ + SS : GOTO 240
 1410 IF A$ = "d" OR A$ = "D" THEN EX = EX + CC : EZ = EZ - SS : GOTO 240
 1420 IF A$ = "q" OR A$ = "Q" THEN 1440
 1430 GOTO 240

 1440 END

10000 REM  ---- sphere intersection (nearest) -> K,T ----
10005 K = - 1
10010 T = 99999
10015 FOR I = 0 TO NS
10020   ZW = RX * RX + RY * RY + RZ * RZ
10025   P = (2 * (RX * (OX - SX(I))+ RY * (OY - SY(I))+ RZ * (OZ - SZ(I))))/ ZW
10030   Q = ((OX - SX(I))* (OX - SX(I))+ (OY - SY(I))* (OY - SY(I))+ (OZ - SZ(I))* (OZ - SZ(I))- SR(I)* SR(I))/ ZW
10035   Z1 = - P / 2
10040   Z2 = ((P * P)/ 4 - Q)
10045   IF Z2 < 0 THEN NEXT :RETURN
10050   WU = SQR (Z2)
10055   X1 = Z1 + WU
10060   X2 = Z1 - WU
10065   Z0 = - ((X1 > .001)AND (X1 < T))
10070   Z1 = - Z0 + 1
10075   K = Z0 * I + Z1 * K
10080   T = Z0 * X1 + Z1 * T
10085   Z0 = - ((X2 > .001)AND (X2 < T))
10090   Z1 = - Z0 + 1
10095   K = Z0 * I + Z1 * K
10100   T = Z0 * X2 + Z1 * T
10105 NEXT
10110 RETURN

10200 REM  ---- box intersection (axis-aligned slab) -> BK,BT ----
10205 BK = - 1
10210 BT = 99999
10215 FOR I = 0 TO NB
10220   TM = - 99999
10225   TX = 99999
10230   REM  X slab (parallel => origin must be inside X range)
10235   IF ABS (RX) >= .0001 THEN 10250
10240   IF OX < UX(I) OR OX > VX(I) THEN 10380
10245   GOTO 10275

10250   A1 = (UX(I) - OX)/ RX
10255   A2 = (VX(I) - OX)/ RX
10260   IF A1 > A2 THEN ZW = A1 : A1 = A2 : A2 = ZW
10265   IF A1 > TM THEN TM = A1
10270   IF A2 < TX THEN TX = A2
10275   REM  Y slab
10280   IF ABS (RY) >= .0001 THEN 10295
10285   IF OY < UY(I) OR OY > VY(I) THEN 10380
10290   GOTO 10320

10295   B1 = (UY(I) - OY)/ RY
10300   B2 = (VY(I) - OY)/ RY
10305   IF B1 > B2 THEN ZW = B1 : B1 = B2 : B2 = ZW
10310   IF B1 > TM THEN TM = B1
10315   IF B2 < TX THEN TX = B2
10320   REM  Z slab
10325   IF ABS (RZ) >= .0001 THEN 10340
10330   IF OZ < UZ(I) OR OZ > VZ(I) THEN 10380
10335   GOTO 10365

10340   C1 = (UZ(I) - OZ)/ RZ
10345   C2 = (VZ(I) - OZ)/ RZ
10350   IF C1 > C2 THEN ZW = C1 : C1 = C2 : C2 = ZW
10355   IF C1 > TM THEN TM = C1
10360   IF C2 < TX THEN TX = C2
10365   IF TM > TX OR TX < .001 THEN 10380
10370   IF TM < .001 THEN TM = TX
10375   IF TM < BT THEN BT = TM : BK = I
10380 NEXT I
10385 RETURN

10400 REM  ---- read scene from DATA (POV-Ray keyword tags) ----
10405 READ OB$
10410 IF OB$ = "END" THEN RETURN
10415 IF OB$ = "CAMERA" THEN READ EX, EY, EZ, DX, DZ : CA = ATN ( (DX - EX) / (DZ - EZ + .0001) ) : GOTO 10405
10420 IF OB$ = "LIGHT" THEN READ LX, LY, LZ : GOTO 10405
10425 IF OB$ = "PLANE" THEN READ GY, CS : GOTO 10405
10430 IF OB$ = "SPHERE" THEN GOSUB 10600 : GOTO 10405
10435 IF OB$ = "BOX" THEN GOSUB 10700 : GOTO 10405
10436 IF OB$ = "ANIMATE" THEN READ KC, SP : AM = 1 : NK = - 1 : GOTO 10405
10437 IF OB$ = "KF" THEN GOSUB 10895 : GOTO 10405
10440 GOTO 10405

10450 REM  ---- skip params for keyword in OB$ (scene scan) ----
10455 IF OB$ = "CAMERA" THEN READ DA, DB, DC, DD, DE : RETURN
10460 IF OB$ = "LIGHT" THEN READ DA, DB, DC : RETURN
10465 IF OB$ = "PLANE" THEN READ DA, DB : RETURN
10470 IF OB$ = "SPHERE" THEN READ DA, DB, DC, DD, DE : RETURN
10475 IF OB$ = "BOX" THEN READ DA, DB, DC, DD, DE, DF : RETURN
10480 IF OB$ = "ANIMATE" THEN READ DA, DB : RETURN
10485 IF OB$ = "KF" THEN READ DA, DB, DC : RETURN
10490 RETURN

10500 REM  ---- shadow test: is OX,OY,OZ shadowed from light? -> SH ----
10505 SH = 0
10510 RX = LX - OX
10515 RY = LY - OY
10520 RZ = LZ - OZ
10525 ZW = SQR (RX * RX + RY * RY + RZ * RZ)
10530 RX = RX / ZW
10535 RY = RY / ZW
10540 RZ = RZ / ZW
10545 GOSUB 10000
10550 IF K >= 0 AND T < ZW THEN SH = 1
10555 GOSUB 10200
10560 IF BK >= 0 AND BT < ZW THEN SH = 1
10565 RETURN

10600 REM  ---- read one sphere (params after "sphere" tag) ----
10605 NS = NS + 1
10610 READ SX(NS), SY(NS), SZ(NS), SR(NS), SM(NS)
10615 RETURN

10700 REM  ---- read one box (params after "box" tag) ----
10705 NB = NB + 1
10710 READ UX(NB), UY(NB), UZ(NB), VX(NB), VY(NB), VZ(NB)
10715 RETURN

10720 REM  ---- box hit (HM=2): face normal + diffuse + shadow ----
10725 HX = OX + BT * RX
10730 HY = OY + BT * RY
10735 HZ = OZ + BT * RZ
10740 NX = 0
10745 NY = 0
10750 NZ = 0
10755 BN = 1
10760 BS = - 1
10765 ZW = ABS (HX - UX(BK))
10770 IF ABS (HX - VX(BK)) < ZW THEN ZW = ABS (HX - VX(BK)) : BN = 1 : BS = 1
10775 IF ABS (HY - UY(BK)) < ZW THEN ZW = ABS (HY - UY(BK)) : BN = 2 : BS = - 1
10780 IF ABS (HY - VY(BK)) < ZW THEN ZW = ABS (HY - VY(BK)) : BN = 2 : BS = 1
10785 IF ABS (HZ - UZ(BK)) < ZW THEN ZW = ABS (HZ - UZ(BK)) : BN = 3 : BS = - 1
10790 IF ABS (HZ - VZ(BK)) < ZW THEN BN = 3 : BS = 1
10795 IF BN = 1 THEN NX = BS : GOTO 10810
10800 IF BN = 2 THEN NY = BS : GOTO 10810
10805 NZ = BS
10810 REM  diffuse lit by light_source (with shadow) -- same as sphere matte
10815 RX = LX - HX
10820 RY = LY - HY
10825 RZ = LZ - HZ
10830 ZW = SQR (RX * RX + RY * RY + RZ * RZ)
10835 RX = RX / ZW
10840 RY = RY / ZW
10845 RZ = RZ / ZW
10850 NL = NX * RX + NY * RY + NZ * RZ
10855 IF NL <= 0 THEN FA = 0 : GOTO 1280
10860 OX = HX
10865 OY = HY
10870 OZ = HZ
10875 GOSUB 10500
10880 IF SH = 1 THEN FA = 0 : GOTO 1280
10885 FA = 1
10890 GOTO 1280

10895 REM  ---- read one keyframe -> KX(NK),KY(NK),KZ(NK) ----
10896 NK = NK + 1 : READ KX(NK), KY(NK), KZ(NK) : RETURN

10900 REM  ---- scene menu: scan names, pick, skip, load ----
10901 SC = 0 : RESTORE
10903 READ OB$
10904 IF OB$ = "STOP" THEN 10915
10905 IF OB$ = "SCENE" THEN SC = SC + 1 : READ NM$ : SN$(SC) = NM$ : GOTO 10903
10906 GOSUB 10450 : GOTO 10903
10915 PRINT CHR$(147)
10916 PRINT "POV-RAY SCENES:"
10917 FOR I = 1 TO SC : PRINT I; " "; SN$(I) : NEXT
10918 PRINT "PRESS 1-"; SC; " :"
10919 GET A$ : IF A$ = "" THEN 10919
10920 SG = VAL(A$)
10921 IF SG < 1 OR SG > SC THEN 10919
10922 SG = SG - 1 : RESTORE
10923 READ OB$
10924 IF OB$ = "SCENE" THEN 10928
10925 GOSUB 10450 : GOTO 10923
10928 IF SG = 0 THEN READ NM$ : GOSUB 10400 : GOTO 10937
10930 SG = SG - 1 : READ NM$ : GOTO 10923
10937 RETURN

10940 REM  ---- animate camera along keyframes (interpolate + heading) ----
10941 P1 = INT(AP)
10942 P2 = P1 + 1
10943 IF P2 >= KC THEN P2 = 0
10944 FT = AP - P1
10945 EX = KX(P1) + FT * (KX(P2) - KX(P1))
10946 EY = KY(P1) + FT * (KY(P2) - KY(P1))
10947 EZ = KZ(P1) + FT * (KZ(P2) - KZ(P1))
10948 AX = KX(P2) - EX : AZ = KZ(P2) - EZ + .0001
10949 CA = ATN(AX / AZ)
10950 IF AZ < 0 THEN CA = CA + 3.141593
10951 RETURN

11000 REM  ============================================================
11001 REM  ===== SCENE 1: CITY -- 3x3 block grid, drive around the block =====
11002 REM  streets: west x-10..-4 / east x6..10 ; cross1 z-10..-4 / cross2 z4..10
11003 REM  loop: south -> east -> south -> west -> north, corner-cut keyframes
11004 REM  ------------------------------------------------------------
11010 DATA "SCENE", "CITY"
11020 DATA "CAMERA", - 7, 2.8, - 28, - 7, 3
11030 DATA "LIGHT", - 20, 40, - 20
11040 DATA "PLANE", - 0.2, 2
11050 REM  -- 9 building blocks (varied heights) --
11060 DATA "BOX", - 20, 0, - 20, - 10, 7, - 10
11061 DATA "BOX", - 4, 0, - 20, 6, 10, - 10
11062 DATA "BOX", 10, 0, - 20, 20, 8, - 10
11063 DATA "BOX", - 20, 0, - 4, - 10, 6, 4
11064 DATA "BOX", - 4, 0, - 4, 6, 12, 4
11065 DATA "BOX", 10, 0, - 4, 20, 9, 4
11066 DATA "BOX", - 20, 0, 10, - 10, 5, 20
11067 DATA "BOX", - 4, 0, 10, 6, 8, 20
11068 DATA "BOX", 10, 0, 10, 20, 7, 20
11070 REM  -- cars parked on the streets (low boxes) --
11071 DATA "BOX", - 8, 0, - 16, - 6, 0.7, - 14
11072 DATA "BOX", - 1, 0, - 8, 1, 0.7, - 6
11073 DATA "BOX", 7, 0, 2, 9, 0.7, 4
11074 DATA "BOX", - 3, 0, 6, - 1, 0.7, 8
11100 REM  -- animation: drive a smooth loop around the center block --
11110 DATA "ANIMATE", 10, 0.04
11120 DATA "KF", - 7, 2.8, - 28
11121 DATA "KF", - 7, 2.8, - 11
11122 DATA "KF", - 3, 2.8, - 7
11123 DATA "KF", 4, 2.8, - 7
11124 DATA "KF", 8, 2.8, - 3
11125 DATA "KF", 8, 2.8, 3
11126 DATA "KF", 4, 2.8, 7
11127 DATA "KF", - 3, 2.8, 7
11128 DATA "KF", - 7, 2.8, 3
11129 DATA "KF", - 7, 2.8, - 20
11130 DATA "END"

11200 REM  ============================================================
11201 REM  ===== SCENE 2: STILL -- three spheres + box (manual) =====
11210 DATA "SCENE", "STILL"
11220 DATA "CAMERA", 0, 2.5, - 8, 0, 2
11230 DATA "LIGHT", - 20, 30, - 20
11240 DATA "PLANE", - 1, 4
11250 DATA "SPHERE", - 2, 0, 2, 1.0, 0
11251 DATA "SPHERE",   0, 0, 2, 1.5, 1
11252 DATA "SPHERE",   2, 0, 2, 0.5, 1
11260 DATA "BOX", 4, - 1, 1, 6, 2, 3
11270 DATA "END"

11300 DATA "STOP"
