/****************************************************************************
**
** Copyright (C) VCreate Logic Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth@scrite.io)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "crashpadmodule.h"

#include <QDir>
#include <QUrl>
#include <QFile>
#include <QDialog>
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>

#include "ui_CrashRecoveryDialog.h"

bool CrashpadModule::prepare()
{
    if (!CrashpadModule::isAvailable())
        return true;

    // Check if there are crash-dump files generated the last time the app was launched.
    const QString crashpadDataPath = CrashpadModule::dataPath();

    QDir crashpadDataDir(crashpadDataPath);
    if (!crashpadDataDir.exists() || !crashpadDataDir.cd("reports"))
        return true;

    const QFileInfoList dmpFiles = crashpadDataDir.entryInfoList(
            { "*.dmp" }, QDir::NoDotAndDotDot | QDir::Files, QDir::NoSort);

    // If no crash-dump files were found, then there is nothing to do.
    if (dmpFiles.empty())
        return true;

    // We move all the crash-dump files to the Desktop, show a message to the user
    // and also offer an option to optionally reset user login credentials before
    // continuing.
    int argc = 0;
    char **argv = nullptr;
    QApplication a(argc, argv);

    const QString desktopFolder = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QDir desktopDir(desktopFolder);
    [&desktopDir](const QString &subfolder) {
        if (!desktopDir.cd(subfolder))
            desktopDir.mkdir(subfolder);
        desktopDir.cd(subfolder);
    }("Scrite Crash Reports");

    for (const QFileInfo &dmpFile : dmpFiles) {
        QFile::copy(dmpFile.absoluteFilePath(), desktopDir.absoluteFilePath(dmpFile.fileName()));
        QFile::remove(dmpFile.absoluteFilePath());
    }

    QDialog messageBox(nullptr,
                       Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                               | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint
                               | Qt::WindowStaysOnTopHint);
    Ui::CrashRecoveryDialog messageBoxUi;
    messageBoxUi.setupUi(&messageBox);

    QObject::connect(messageBoxUi.joinDiscordButton, &QPushButton::clicked, &messageBox, []() {
        QDesktopServices::openUrl(QUrl("https://www.scrite.io/index.php/forum/"));
    });

    if (messageBox.exec() == QDialog::Accepted) {
        const QString settingsFile = crashpadDataPath + "/../settings.ini";

        if (messageBoxUi.factoryResetOption->isChecked()) {
            QFile::remove(settingsFile);
        } else if (messageBoxUi.resetLoginCredsOption->isChecked()) {
            QSettings settings(settingsFile, QSettings::IniFormat);
            settings.beginGroup("Registration");
            settings.setValue("loginToken", QVariant());
            settings.setValue("userInfo", QVariant());
            settings.setValue("devices", QVariant());
            settings.sync();
        }

        return true;
    }

    return false;
}

#ifndef CRASHPAD_AVAILABLE
bool CrashpadModule::isAvailable()
{
    return false;
}

bool CrashpadModule::initialize()
{
    return false;
}

QString CrashpadModule::handlerPath()
{
    return QString();
}

QString CrashpadModule::dataPath()
{
    return QString();
}
#endif
