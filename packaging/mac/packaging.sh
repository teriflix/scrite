cp -vaRf ../../../Release/scrite.app .
~/Qt5.13.2/5.13.2/clang_64/bin/macdeployqt scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -dmg
mv scrite.dmg scrite-0.3.8-beta.dmg
