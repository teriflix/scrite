cp -vaRf ../../../Release/Scrite.app .
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt5.13.2/5.13.2/clang_64/bin/macdeployqt Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -dmg
mv Scrite.dmg Scrite-0.4.5-beta.dmg
