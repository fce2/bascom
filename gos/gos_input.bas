5 REM! GOS  stress: INPUT blocks for RETURN; under --gos does the scheduler stall?
10 TASYNC 100
20 PRINT ".";:TYIELD:GOTO 20
100 INPUT A$:PRINT A$:TYIELD:GOTO 100
