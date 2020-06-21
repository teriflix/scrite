export PATH=/home/prashanthudupa/Qt5.13.2/5.13.2/gcc_64/bin:/home/prashanthudupa/QtPackaging/linuxdeployqt/bin:$PATH
export LD_LIBRARY_PATH=export LD_LIBRARY_PATH=/home/prashanthudupa/Qt5.13.2/5.13.2/gcc_64/lib:/home/prashanthudupa/Qt5.13.2/5.13.2/gcc_64/bin:$LD_LIBRARY_PATH
mkdir scrite-0.4.2-beta.AppImage
cd scrite-0.4.2-beta.AppImage
cp ../../../../Release/scrite .
cp ../../../images/appicon.png scrite_app_icon.png
cp ../scrite.desktop .
cp ../install.sh install_scrite.sh
chmod a+x install_scrite.sh
linuxdeployqt scrite -appimage -qmldir=../../../qml -verbose=2 -always-overwrite -no-translations -no-copy-copyright-files
cd ../
tar -czvf scrite-0.4.2-beta.AppImage.tar.gz scrite-0.4.2-beta.AppImage
