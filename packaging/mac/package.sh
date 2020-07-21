cp -vaRf ../../../Release/Scrite.app .
cp -vaf ../../Info.plist Scrite.app/Contents
~/Qt5.13.2/5.13.2/clang_64/bin/macdeployqt2 Scrite.app -qmldir=../../qml -verbose=1 -appstore-compliant -codesign="$TERIFLIX_IDENT"
mkdir Scrite-0.4.11-beta
mv Scrite.app Scrite-0.4.11-beta
cp ../../images/dmgbackdrop.png dmgbackdrop.png
~/Qt5.13.2/5.13.2/clang_64/bin/TextOverImage --file dmgbackdrop.png --text "Version 0.4.11 beta" --text-color lightgray --xpos 75 --ypos 160 --font-size 32 --dpr 2 --output background.png
~/Utils/create-dmg/create-dmg \
  --volname "Scrite-0.4.11-beta" \
  --background "background.png" \
  --window-pos 272 136 \
  --window-size 896 628 \
  --icon-size 128 \
  --icon "Scrite.app" 256 300 \
  --hide-extension "Scrite.app" \
  --app-drop-link 620 300 \
  --hdiutil-verbose \
  "Scrite-0.4.11-beta.dmg" \
  "Scrite-0.4.11-beta/"
rm -f background.png
rm -f dmgbackdrop.png
mv Scrite-0.4.11-beta/Scrite.app .
rm -fr Scrite-0.4.11-beta
