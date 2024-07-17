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
#include <QVersionNumber>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QOperatingSystemVersion>

#ifdef CRASHPAD_AVAILABLE
#include "ui_CrashRecoveryDialog.h"

#include "client/crashpad_client.h"
#endif

bool CrashpadModule::prepare()
{
#ifdef CRASHPAD_AVAILABLE
    /**
     * If a crash was detected the last time Scrite was launched, then we present a dialog
     * informing the user that a crash had occured, along with some options about what to
     * do next.
     *
     * Option #1: Send crash reports to us
     * Option #2: Reset login credentials and try starting the app
     * Option #3: Reset all settings to factory defaults, and try starting the app
     *
     * This implementation is common to all platforms, so its implemented here in this file.
     */
    if (!CrashpadModule::isAvailable())
        return true;

    // Check if there are crash-dump files generated the last time the app was launched.
    const QString pendingCrashReportsPath = CrashpadModule::pendingCrashReportsPath();
    if (pendingCrashReportsPath.isEmpty())
        return true;

    QDir crashpadDataDir(pendingCrashReportsPath);
    if (!crashpadDataDir.exists())
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
        const QString targetFileName = dmpFile.completeBaseName() + "-" + CrashpadModule::platform()
                + "-"
                + QVersionNumber(QOperatingSystemVersion::current().majorVersion(),
                                 QOperatingSystemVersion::current().minorVersion(),
                                 QOperatingSystemVersion::current().microVersion())
                          .toString()
                + "-Scrite-" + QStringLiteral(SCRITE_VERSION) + ".dmp";
        QFile::copy(dmpFile.absoluteFilePath(), desktopDir.absoluteFilePath(targetFileName));
        QFile::remove(dmpFile.absoluteFilePath());
    }

    QDialog messageBox(nullptr,
                       Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                               | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint
                               | Qt::WindowStaysOnTopHint);
    Ui::CrashRecoveryDialog messageBoxUi;
    messageBoxUi.setupUi(&messageBox);

    const QString settingsFile = CrashpadModule::dataPath() + "/../settings.ini";
    messageBoxUi.resetLoginCredsOption->setEnabled(QFile::exists(settingsFile));
    messageBoxUi.factoryResetOption->setEnabled(messageBoxUi.resetLoginCredsOption->isEnabled());

    QObject::connect(messageBoxUi.joinDiscordButton, &QPushButton::clicked, &messageBox, []() {
        QDesktopServices::openUrl(QUrl("https://www.scrite.io/index.php/forum/"));
    });

    if (messageBox.exec() == QDialog::Accepted) {
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
#endif

    return false;
}

void CrashpadModule::crash()
{
#ifdef ENABLE_CRASHPAD_CRASH_TEST
    if (CrashpadModule::initialize())
        *(volatile int *)0 = 0;
#endif
}

#ifdef CRASHPAD_AVAILABLE
namespace CrashpadModule {
#ifdef Q_OS_WINDOWS
std::wstring toPlatformString(const QString &string)
{
    return string.toStdWString();
}
#else
std::string toPlatformString(const QString &string)
{
    return string.toStdString();
}
#endif
}
#endif

bool CrashpadModule::initialize()
{
#ifdef CRASHPAD_AVAILABLE
    static bool invokedOnce = false;
    static bool initStatus = false;

    if (invokedOnce)
        return initStatus;

    invokedOnce = true;

    const QString crashpadHandler = QDir::toNativeSeparators(CrashpadModule::handlerPath());
    if (crashpadHandler.isEmpty())
        return false;

    const QString dataPath = QDir::toNativeSeparators(CrashpadModule::dataPath());

    const base::FilePath _handlerPath(CrashpadModule::toPlatformString(crashpadHandler));
    const base::FilePath _crashpadPath(CrashpadModule::toPlatformString(dataPath));
    const base::FilePath _attachmentPath(
            CrashpadModule::toPlatformString(dataPath + "\\attachment.txt"));
    const std::vector<base::FilePath> _attachments = { _attachmentPath };
    const std::map<std::string, std::string> _annotations = { { "format", "minidump" } };
    const std::vector<std::string> _args = { "--no-rate-limit" };
    const std::string _url;

    static crashpad::CrashpadClient client;
    initStatus = client.StartHandler(_handlerPath, _crashpadPath, _crashpadPath, _url, _annotations,
                                     _args, true, false, _attachments);

    return initStatus;
#else // CRASHPAD_AVAILABLE
    return false;
#endif // CRASHPAD_AVAILABLE
}

#ifndef CRASHPAD_AVAILABLE
bool CrashpadModule::isAvailable()
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

QString CrashpadModule::pendingCrashReportsPath()
{
    return QString();
}

QString CrashpadModule::platform()
{
    return QStringLiteral("Unknown");
}
#endif // CRASHPAD_AVAILABLE
