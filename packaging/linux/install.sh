rm -f scrite.desktop
echo "[Desktop Entry]" > scrite.desktop
echo "Type=Application" >> scrite.desktop
echo "Name=Scrite" >> scrite.desktop
echo "Exec=`pwd`/AppRun %f" >> scrite.desktop
echo "Icon=`pwd`/scrite_app_icon.png" >> scrite.desktop
echo "Comment=Multilingual Screenplay Writing App from TERIFLIX" >> scrite.desktop
echo "Terminal=false" >> scrite.desktop
echo "Categories=Office;WordProcessor;" >> scrite.desktop
chmod a+x scrite.desktop
sudo desktop-file-install scrite.desktop
cp -f scrite.desktop ~/Desktop
mkdir -p ~/Applications
cp -f scrite.desktop ~/Applications
