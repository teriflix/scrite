copy ..\..\..\Release\scrite.exe .
copy C:\Qt\Qt5.13.2\vcredist\vcredist_msvc2017_x64.exe vcredist_x64.exe
windeployqt --qmldir ..\..\qml --no-compiler-runtime --no-translations . --list relative > files.txt
fillnsi --installs-key WINDEPLOYQT_INSTALLS --uninstalls-key WINDEPLOYQT_UNINSTALLS --list files.txt --input installer-x64.nsi.in --output installer-x64.nsi
%MakeNSISTool% installer-x64.nsi
%CodeSignTool% sign /f %TERIFLIX_CSC% /p %TERIFLIX_CSC_PWORD% Scrite-*-Beta-64bit-Setup.exe
