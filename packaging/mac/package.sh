$SCRITE_CRASHPAD_ROOT/bin/dump_syms ../../../Release/Scrite.app.dSYM/Contents/Resources/DWARF/Scrite > ../../../Release/Scrite.app.sym

cp -vaRf ../../../Release/Scrite.app .
codesign -s "$SCRITE_IDENT" ./Scrite.app/Contents/MacOS/crashpad_handler
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt/5.15.19/clang_64/bin/macdeployqt Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -hardened-runtime -codesign="$SCRITE_IDENT"
mkdir Scrite-2.0.1
mv Scrite.app Scrite-2.0.1
cp ../../images/dmgbackdrop.png dmgbackdrop.png
cp ../../appicon.icns Scrite.icns
sed "s/{{VERSION}}/Version 2.0.1/" dmgbackdrop.qml > dmgbackdropgen.qml
~/Qt/5.15.19/clang_64/bin/qmlscene dmgbackdropgen.qml
rm -f dmgbackdropgen.qml

# https://ss64.com/osx/sips.html
sips -s dpiWidth 144 -s dpiHeight 144 background.png

# https://github.com/create-dmg/create-dmg
~/Utils/create-dmg/create-dmg \
  --volicon "Scrite.icns" \
  --volname "Scrite-2.0.1" \
  --background "background.png" \
  --window-pos 272 136 \
  --window-size 896 660 \
  --icon-size 128 \
  --icon "Scrite.app" 256 300 \
  --hide-extension "Scrite.app" \
  --app-drop-link 620 300 \
  --hdiutil-verbose \
  "Scrite-2.0.1.dmg" \
  "Scrite-2.0.1/"
rm -f background.png
rm -f dmgbackdrop.png
rm -f Scrite.icns
mv Scrite-2.0.1/Scrite.app .
rm -fr Scrite-2.0.1

mv ../../../Release/Scrite.app.sym Scrite-2.0.1.app.sym
