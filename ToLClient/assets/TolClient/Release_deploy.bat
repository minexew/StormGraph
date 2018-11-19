@echo off

call ..\..\Config.bat
%Helium% ../../utils/deploy.he Release

..\..\utils\mkftpscript.exe Release -hREDACTED -uREDACTED -pREDACTED -dlanthaia.net/web/update/tolcl2 -oFtp
