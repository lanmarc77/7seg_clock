@echo off
mode %1 BAUD=115200 PARITY=n DATA=8 RTS=OFF DTR=OFF TO=OFF
echo U > %1
