@echo off
rem low level CRT-Less compiler flags
gcc iocplab1.c -o iocplab1.exe -nostdlib -e entry -lkernel32 -luser32 -lws2_32 -s -O
pause