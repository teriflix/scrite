copy ..\..\..\Release\scrite.exe .
copy C:\Qt\vcredist\vcredist_msvc2019_x64.exe vcredist_x64.exe
copy %SCRITE_OPENSSL_LIBS%\openssl-1.1\x64\bin\libcrypto-1_1-x64.dll .
copy %SCRITE_OPENSSL_LIBS%\openssl-1.1\x64\bin\libssl-1_1-x64.dll .
windeployqt --qmldir ..\..\qml --no-compiler-runtime --no-translations . --list relative > files.txt
fillnsi --installs-key WINDEPLOYQT_INSTALLS --uninstalls-key WINDEPLOYQT_UNINSTALLS --list files.txt --input installer-x64.nsi.in --output installer-x64.nsi
%MakeNSISTool% installer-x64.nsi
%CodeSignTool% sign /f %SCRITE_CSC% /p %SCRITE_CSC_PWORD% Scrite-*-Beta-64bit-Setup.exe
