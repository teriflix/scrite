rm -f Scrite.desktop
echo "[Desktop Entry]" > Scrite.desktop
echo "Type=Application" >> Scrite.desktop
echo "Name=Scrite" >> Scrite.desktop
echo "Exec=`pwd`/AppRun %f" >> Scrite.desktop
echo "Icon=`pwd`/Scrite_app_icon.png" >> Scrite.desktop
echo "Comment=Multilingual Screenplay Writing App from TERIFLIX" >> Scrite.desktop
echo "Terminal=false" >> Scrite.desktop
echo "Categories=Office;WordProcessor;" >> Scrite.desktop
chmod a+x Scrite.desktop
sudo desktop-file-install Scrite.desktop
cp -f Scrite.desktop ~/Desktop
mkdir -p ~/Applications
cp -f Scrite.desktop ~/Applications
