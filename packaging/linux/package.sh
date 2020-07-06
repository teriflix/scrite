export PATH=/home/prashanthudupa/Qt5.13.2/5.13.2/gcc_64/bin:/home/prashanthudupa/QtPackaging/linuxdeployqt/bin:$PATH
export LD_LIBRARY_PATH=export LD_LIBRARY_PATH=/home/prashanthudupa/Qt5.13.2/5.13.2/gcc_64/lib:/home/prashanthudupa/Qt5.13.2/5.13.2/gcc_64/bin:$LD_LIBRARY_PATH
mkdir Scrite-0.4.6-beta.AppImage
cd Scrite-0.4.6-beta.AppImage
cp ../../../../Release/Scrite .
cp ../../../images/appicon.png Scrite_app_icon.png
cp ../Scrite.desktop .
cp ../install.sh install_Scrite.sh
chmod a+x install_Scrite.sh
linuxdeployqt Scrite -appimage -qmldir=../../../qml -verbose=2 -always-overwrite -no-translations -no-copy-copyright-files
cd ../
tar -czvf Scrite-0.4.6-beta.AppImage.tar.gz Scrite-0.4.6-beta.AppImage
