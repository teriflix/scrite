cp -vaRf ../../../Release/Scrite.app .
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt/5.15.8/clang_64/bin/macdeployqt Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -hardened-runtime -codesign="$TERIFLIX_IDENT"
mkdir Scrite-0.8.7-beta
mv Scrite.app Scrite-0.8.7-beta
cp ../../images/dmgbackdrop.png dmgbackdrop.png
sed "s/{{VERSION}}/Version 0.8.7 Beta/" dmgbackdrop.qml > dmgbackdropgen.qml
~/Qt/5.15.8/clang_64/bin/qmlscene dmgbackdropgen.qml
rm -f dmgbackdropgen.qml

# https://ss64.com/osx/sips.html
sips -s dpiWidth 144 -s dpiHeight 144 background.png

# https://github.com/create-dmg/create-dmg
~/Utils/create-dmg/create-dmg \
  --volname "Scrite-0.8.7-beta" \
  --background "background.png" \
  --window-pos 272 136 \
  --window-size 896 660 \
  --icon-size 128 \
  --icon "Scrite.app" 256 300 \
  --hide-extension "Scrite.app" \
  --app-drop-link 620 300 \
  --hdiutil-verbose \
  "Scrite-0.8.7-beta.dmg" \
  "Scrite-0.8.7-beta/"
rm -f background.png
rm -f dmgbackdrop.png
mv Scrite-0.8.7-beta/Scrite.app .
rm -fr Scrite-0.8.7-beta
