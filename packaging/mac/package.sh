cp -vaRf ../../../Release/Scrite.app .
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt5.13.2/5.13.2/clang_64/bin/macdeployqt2 Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -dmg -codesign="$TERIFLIX_IDENT"
mv Scrite.dmg Scrite-0.4.6-beta.dmg
