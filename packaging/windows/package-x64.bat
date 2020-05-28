copy ..\..\..\Release\scrite.exe .
copy vcredist_x64 vcredist_x64.exe
windeployqt --qmldir ..\..\qml --no-compiler-runtime .
%MakeNSISTool% installer-x64.nsi
%CodeSignTool% sign /f %TERIFLIX_CSC% /p %TERIFLIX_CSC_PWORD% Scrite-*-Beta-64bit-Setup.exe
