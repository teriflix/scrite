export PATH=~/Qt/5.15.19/gcc_64/bin:/usr/lib/x86_64-linux-gnu:$PATH
export LD_LIBRARY_PATH=~/Qt/5.15.19/gcc_64/lib:/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
export VERSION=2.0.10
mkdir Scrite-2.0.10.AppImage
cd Scrite-2.0.10.AppImage
mkdir bin
cp ../../../../Release/Scrite ./bin/
mkdir lib
cp /usr/lib/x86_64-linux-gnu/libssl.so.1.1 ./lib
cp /usr/lib/x86_64-linux-gnu/libcrypto.so.1.1 ./lib
cp -L /usr/lib/x86_64-linux-gnu/libibus-1.0.so ./lib
cp -L /usr/lib/x86_64-linux-gnu/libgio-2.0.so ./lib
cp -L /usr/lib/x86_64-linux-gnu/libgobject-2.0.so ./lib
cp -L /usr/lib/x86_64-linux-gnu/libglib-2.0.so ./lib
chmod a-x ./lib/*.so*
mkdir -p share/applications
cp ../Scrite.desktop share/applications/Scrite.desktop
mkdir -p share/icons/hicolor/512x512/apps/
cp ../../../images/appicon.png share/icons/hicolor/512x512/apps/Scrite.png
mkdir -p share/icons/hicolor/256x256/apps/
convert ../../../images/appicon.png -resize 256x256 share/icons/hicolor/256x256/apps/Scrite.png
cd ../
~/linuxdeployqt Scrite-2.0.10.AppImage/share/applications/Scrite.desktop -appimage -qmldir=../../qml -verbose=2 -no-translations -no-copy-copyright-files
cd ../
