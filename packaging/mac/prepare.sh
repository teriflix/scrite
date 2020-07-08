cp -vaRf ../../../Release/Scrite.app .
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt5.13.2/5.13.2/clang_64/bin/macdeployqt2 Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -codesign="$TERIFLIX_IDENT"

