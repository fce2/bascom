100 REM  *** AMIGA BOING -- 3D RAYTRACED CHECKERED-BALL PONG ***
101 REM  built on Marco64's bt2 raytracer: 1 checkered sphere (the boing
102 REM  ball, shaded by the surface normal at the hit point) + 1 solid paddle
103 REM  sphere. Ball has gravity + wall bounces; cursor L/R moves the paddle.
104 REM  Hit the ball back before it gets past the paddle. GET paces + exits on Q.
105 REM  NOTE: bascom truncates scalar names to 2 chars, so ball pos = MX/MY/MZ
106 REM  (NOT BXB/BYB/BZB which collide with camera BX/BY/BZ) and paddle = PA
107 REM  (NOT PX which collides with the pixel-loop var).
200 DIM KX(10),KY(10),KZ(10),KR(10)
201 AK = 2 - 1
202 KX(0)= 0 : KY(0)= 8 : KZ(0)= 30 : KR(0)= 4
206 KX(1)= 0 : KY(1)= 4 : KZ(1)= 12 : KR(1)= 2.8
214 BX = 0
215 BY = 5
216 BZ = - 17
217 BE = 25
218 LX = - 50
219 LY = 300
220 LZ = 50
221 GB = 10
222 POKE 53272,11
223 POKE 53265,59
224 POKE 56576,148
250 FR = 0 : PA = 0 : VX = 0.3 : VY = 0 : VZ = -1.5 : MX = 0 : MY = 5 : MZ = 30
260 REM ---- physics: ball bounces with gravity, paddle L/R ----
261 MX = MX + VX : MY = MY + VY : MZ = MZ + VZ
262 VY = VY - 0.5
263 IF MY < KR(0) THEN MY = KR(0) : VY = - VY * 0.7
264 IF MX < - 5.5 THEN MX = - 5.5 : VX = - VX
265 IF MX > 5.5 THEN MX = 5.5 : VX = - VX
266 IF MZ > 55 THEN MZ = 55 : VZ = - VZ
267 IF MZ < 12 AND VZ < 0 AND ABS (MX-PA) < 4 THEN VZ = - VZ : VX = VX + (MX-PA)*0.2
268 IF MZ < - 10 THEN MX = 0 : MY = 5 : MZ = 30 : VX = 0.3 : VY = 0 : VZ = -1.5
270 KX(0)= MX : KY(0)= MY : KZ(0)= MZ
271 KX(1)= PA : KY(1)= 4 : KZ(1)= 12
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
708 IF K = 0 THEN 720
709 IF (KR(1)* .95 < D)THEN FA = 1:GOTO 900
710 FA = 1: GOTO 900
720 NX = (OX + T * RX - KX(0))/ KR(0)
721 NY = (OY + T * RY - KY(0))/ KR(0)
722 NZ = (OZ + T * RZ - KZ(0))/ KR(0)
723 FA = (INT (NX * 3)+ INT (NY * 3)+ INT (NZ * 3))AND 1
724 GOTO 900
900 PW = PW * 2 + FA
901 NEXT PX
902 POKE 57344 + PB,PW
903 NEXT PB
904 FR = FR + 1
905 GET A$
906 IF A$ = CHR$(29) THEN PA = PA + 2.5 : IF PA > 5.5 THEN PA = 5.5
907 IF A$ = CHR$(157) THEN PA = PA - 2.5 : IF PA < - 5.5 THEN PA = - 5.5
908 IF A$ = "Q" OR A$ = "q" THEN 920
909 GOTO 260
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