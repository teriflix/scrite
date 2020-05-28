copy ..\..\..\Release\scrite.exe .
copy vcredist_x86 vcredist_x86.exe
windeployqt --qmldir ..\..\qml --no-compiler-runtime .
%MakeNSISTool% installer-x86.nsi
%CodeSignTool% sign /f %TERIFLIX_CSC% /p %TERIFLIX_CSC_PWORD% Scrite-*-Beta-32bit-Setup.exe
