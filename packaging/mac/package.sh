$SCRITE_CRASHPAD_ROOT/bin/dump_syms ../../../Release/Scrite.app.dSYM/Contents/Resources/DWARF/Scrite > ../../../Release/Scrite.app.sym

cp -vaRf ../../../Release/Scrite.app .
codesign -s "$SCRITE_IDENT" ./Scrite.app/Contents/MacOS/crashpad_handler
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt/5.15.19/macos/bin/macdeployqt Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -hardened-runtime -codesign="$SCRITE_IDENT"
mkdir Scrite-2.0.20
mv Scrite.app Scrite-2.0.20
cp ../../images/dmgbackdrop.png dmgbackdrop.png
cp ../../appicon.icns Scrite.icns
sed "s/{{VERSION}}/Version 2.0.20/" dmgbackdrop.qml > dmgbackdropgen.qml
~/Qt/5.15.19/macos/bin/qmlscene dmgbackdropgen.qml
rm -f dmgbackdropgen.qml

# https://ss64.com/osx/sips.html
sips -s dpiWidth 144 -s dpiHeight 144 background.png

sed "s/{{VERSION}}/2.0.20/" dmg_settings_tmpl.py > dmg_settings.py

dmgbuild -s dmg_settings.py "Scrite-2.0.20" Scrite-2.0.20.dmg

rm -f background.png
rm -f dmgbackdrop.png
rm -f Scrite.icns
mv Scrite-2.0.20/Scrite.app .
rm -fr Scrite-2.0.20

mv ../../../Release/Scrite.app.sym Scrite-2.0.20.app.sym
