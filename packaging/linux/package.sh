export PATH=/home/prashanthudupa/Qt/5.15.18/gcc_64/bin:/usr/lib/x86_64-linux-gnu:$PATH
export LD_LIBRARY_PATH=/home/prashanthudupa/Qt/5.15.18/gcc_64/lib:/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
export VERSION=1.0
mkdir Scrite-1.0.AppImage
cd Scrite-1.0.AppImage
mkdir bin
cp ../../../../Release/Scrite ./bin/
mkdir -p share/applications
cp ../Scrite.desktop share/applications/Scrite.desktop
mkdir -p share/icons/hicolor/512x512/apps/
cp ../../../images/appicon.png share/icons/hicolor/512x512/apps/Scrite.png
mkdir -p share/icons/hicolor/256x256/apps/
convert ../../../images/appicon.png -resize 256x256 share/icons/hicolor/256x256/apps/Scrite.png
cd ../
linuxdeployqt Scrite-1.0.AppImage/share/applications/Scrite.desktop -appimage -qmldir=../../qml -verbose=2 -no-translations -no-copy-copyright-files
cd ../
