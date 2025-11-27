copy ..\..\..\Release\scrite.exe .
%CodeSignTool% sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 /n "%SCRITE_BUSINESS_NAME%" scrite.exe
copy C:\Qt\vcredist\vcredist_msvc2019_x86.exe vcredist_x86.exe
copy %SCRITE_OPENSSL_LIBS%\openssl-1.1\x86\bin\libcrypto-1_1.dll .
copy %SCRITE_OPENSSL_LIBS%\openssl-1.1\x86\bin\libssl-1_1.dll .
windeployqt --qmldir ..\..\qml --no-compiler-runtime --no-translations . --list relative > files.txt
copy %SCRITE_CRASHPAD_ROOT%\bin\crashpad_handler.exe .
fillnsi --installs-key WINDEPLOYQT_INSTALLS --uninstalls-key WINDEPLOYQT_UNINSTALLS --list files.txt --input installer-x86.nsi.in --output installer-x86.nsi
%MakeNSISTool% installer-x86.nsi
%CodeSignTool% sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 /n "%SCRITE_BUSINESS_NAME%" Scrite-1.9.9-32bit-Setup.exe
mkdir ..\..\..\Release\Deploy
copy Scrite-1.9.9-32bit-Setup.exe ..\..\..\Release\Deploy
copy ..\..\..\Release\scrite.pdb ..\..\..\Release\Deploy\Scrite-1.9.9-32bit.pdb
