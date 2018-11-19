@echo off

call Config.bat
%Helium% %HeliumDir%\AutoBind\AutoBind-v1.3.he src/Binding/StormGraph_Binding.cfx2 -I%HeliumDir%/

pause