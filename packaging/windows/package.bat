copy ..\..\..\Release\scrite.exe .
copy vcredist_x64 vcredist_x64.exe
windeployqt --qmldir ..\..\qml --no-compiler-runtime .
