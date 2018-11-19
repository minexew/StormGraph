@echo off

mkdir bin

cd ..
call ..\Config.bat

%Helium% storm/main.he -c -ostorm/bin/storm
