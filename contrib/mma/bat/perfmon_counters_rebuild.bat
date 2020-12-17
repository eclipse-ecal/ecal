@echo off
REM This batch file will rebuild the performance counters on this PC.
cd C:\windows\syswow64\
lodctr /r
pause