   10 DIM KX(13),KY(13),KZ(13),KR(13)
   20 AK = 14 - 1
   30 KX(0)= 0
   40 KY(0)= 19
   50 KZ(0)= 100
   60 KR(0)= 3.2
   70 KX(1)= 0
   80 KY(1)= 12
   90 KZ(1)= 100
  100 KR(1)= 5.5
  110 KX(2)= 0
  120 KY(2)= 4
  130 KZ(2)= 100
  140 KR(2)= 4.5
  150 KX(3)= -3.5
  160 KY(3)= -2
  170 KZ(3)= 100
  180 KR(3)= 3
  190 KX(4)= 3.5
  200 KY(4)= -2
  210 KZ(4)= 100
  220 KR(4)= 3
  230 BX = 0
  240 BY = 8
  250 BZ = -17
  260 BE = 25
  270 LX = -50
  280 LY = 300
  290 LZ = 50
  300 GB = 10
  310 POKE 53272,11
  320 POKE 53265,59
  330 POKE 56576,148
  340 FR = 0
  350 P = FR * 0.15
  360 TR = 0.5 + 0.5 * SIN(P)
  370 TL = 0.5 + 0.5 * SIN(P + 3.14159)
  380 SR = SIN(TR)
  390 CR = COS(TR)
  400 SL = SIN(TL)
  410 CL = COS(TL)
  420 KX(8)= 5 + 4 * SR
  430 KY(8)= 16 - 4 * CR
  440 KZ(8)= 100 - 0.6 * SR
  450 KR(8)= 2.5
  460 KX(9)= 5 + 12 * SR
  470 KY(9)= 16 - 12 * CR
  480 KZ(9)= 100 - 1.8 * SR
  490 KR(9)= 2.2
  500 KX(10)= 5 + 16 * SR
  510 KY(10)= 16 - 16 * CR
  520 KZ(10)= 100 - 2.4 * SR
  530 KR(10)= 2
  540 KX(5)= -5 - 4 * SL
  550 KY(5)= 16 - 4 * CL
  560 KZ(5)= 100 - 0.6 * SL
  570 KR(5)= 2.5
  580 KX(6)= -5 - 12 * SL
  590 KY(6)= 16 - 12 * CL
  600 KZ(6)= 100 - 1.8 * SL
  610 KR(6)= 2.2
  620 KX(7)= -5 - 16 * SL
  630 KY(7)= 16 - 16 * CL
  640 KZ(7)= 100 - 2.4 * SL
  650 KR(7)= 2
  660 B0 = P
  670 B1 = P + 2.0944
  680 B2 = P + 4.1888
  690 KX(11)= -7 * COS(B0)
  700 KY(11)= 6 + 10 * SIN(B0)
  710 KZ(11)= 96 - 2 * SIN(B0)
  720 KR(11)= 2
  730 KX(12)= -7 * COS(B1)
  740 KY(12)= 6 + 10 * SIN(B1)
  750 KZ(12)= 96 - 2 * SIN(B1)
  760 KR(12)= 2
  770 KX(13)= -7 * COS(B2)
  780 KY(13)= 6 + 10 * SIN(B2)
  790 KZ(13)= 96 - 2 * SIN(B2)
  800 KR(13)= 2
  810 IF CF = 0 THEN 830
  820 GOTO 880

  830 FOR I = 1024 TO 2023
  840   POKE 48128 + I,15
  850   POKE 54272 + I,0
  860 NEXT
  870 CF = 1

  880 FOR PB =  0 TO 8000
  890   ZW = PB AND 7
  900   Y = INT (PB / 320)* 8 + ZW
  910   ZW = INT (PB / 8)
  920   ZX = (ZW - INT (ZW / 40)* 40)* 8
  930   PW = 0
  940   FOR PX = 0 TO 7
  950     X = ZX + PX
  960     ZW = GB / 320
  970     RX = X * ZW - GB / 2
  980     RY = - Y * ZW + 200 / 2 * ZW
  990     RZ = BE
 1000     ZW = SQR (RX * RX + RY * RY + RZ * RZ)
 1010     RX = RX / ZW
 1020     RY = RY / ZW
 1030     RZ = RZ / ZW
 1040     OX = BX
 1050     OY = BY
 1060     OZ = BZ
 1070     RD = 0
 1080     GOSUB 10000
 1090     ON - ((OY + T * RY)> 0)GOTO 1260
 1100     LA = - OY / RY
 1110     SX = OX + LA * RX
 1120     SY = 0
 1130     SZ = OZ + LA * RZ
 1140     ZW = (INT (SX / 5)AND 1)+ (INT (SZ / 5)AND 1)
 1150     FA = (ZW = 1)+ 1
 1160     OX = SX
 1170     OY = SY
 1180     OZ = SZ
 1190     RX = OX + LX
 1200     RY = OY + LY
 1210     RZ = OZ + LZ
 1220     GOSUB 10000
 1230     ON - (K = - 1)GOTO 1530
 1240     FA = ((X AND 1)< > (Y AND 1))+ 1
 1250     GOTO 1530

 1260     IF K = - 1 THEN FA = 0:GOTO 1530
 1270     X1 = KX(K)* RX + KY(K)* RY + KZ(K)* RZ
 1280     X2 = OX * RX + OY * RY + OZ * RZ
 1290     X3 = RX * RX + RY * RY + RZ * RZ
 1300     LA = (X1 - X2)/ X3
 1310     F1 = (OX + LA * RX)- KX(K)
 1320     F2 = (OY + LA * RY)- KY(K)
 1330     F3 = (OZ + LA * RZ)- KZ(K)
 1340     D = SQR (F1 * F1 + F2 * F2 + F3 * F3)
 1350     IF (KR(K)* .95 < D)THEN FA = 1:GOTO 1530
 1360     RD = RD + 1
 1370     IF RD > 3 THEN FA = 1:GOTO 1530
 1380     OX = OX + T * RX
 1390     OY = OY + T * RY
 1400     OZ = OZ + T * RZ
 1410     NX = KX(K)- OX
 1420     NY = KY(K)- OY
 1430     NZ = KZ(K)- OZ
 1440     ZW = NX * RX + NY * RY + NZ * RZ
 1450     RX = RX - 2 * ZW * NX
 1460     RY = RY - 2 * ZW * NY
 1470     RZ = RZ - 2 * ZW * NZ
 1480     ZW = SQR (RX * RX + RY * RY + RZ * RZ)
 1490     RX = RX / ZW
 1500     RY = RY / ZW
 1510     RZ = RZ / ZW
 1520     GOTO 1080

 1530     PW = PW * 2 + FA
 1540   NEXT PX
 1550   POKE 57344 + PB,PW
 1560 NEXT PB
 1570 FR = FR + 1
 1580 GET A$
 1590 IF A$ < > "" THEN 1610
 1600 GOTO 350

 1610 END

10000 K = - 1
10005 T = 99999
10010 FOR I = 0 TO AK
10015   ZW = RX * RX + RY * RY + RZ * RZ
10020   P = (2 * (RX * (OX - KX(I))+ RY * (OY - KY(I))+ RZ * (OZ - KZ(I))))/ ZW
10025   Q = ((OX - KX(I))* (OX - KX(I))+ (OY - KY(I))* (OY - KY(I))+ (OZ - KZ(I))* (OZ - KZ(I))- KR(I)* KR(I))/ ZW
10030   Z1 = - P / 2
10035   Z2 = ((P * P)/ 4 - Q)
10040   IF Z2 < 0 THEN NEXT :RETURN
10045   WU = SQR (Z2)
10050   X1 = Z1 + WU
10055   X2 = Z1 - WU
10060   Z0 = - ((X1 > .001)AND (X1 < T))
10065   Z1 = - Z0 + 1
10070   K = Z0 * I + Z1 * K
10075   T = Z0 * X1 + Z1 * T
10080   Z0 = - ((X2 > .001)AND (X2 < T))
10085   Z1 = - Z0 + 1
10090   K = Z0 * I + Z1 * K
10095   T = Z0 * X2 + Z1 * T
10100 NEXT
10105 RETURN
