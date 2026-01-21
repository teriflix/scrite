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

#include "systemrequirements.h"
#include "restapicall.h"
#include "utils.h"

#include <QGuiApplication>
#include <QMessageBox>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOperatingSystemVersion>
#include <QProgressDialog>
#include <QScreen>
#include <QStorageInfo>
#include <QThread>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QTimer>
#include <QLabel>
#include <QDesktopServices>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#elif defined(Q_OS_MACOS)
#include <sys/sysctl.h>
#endif

namespace {
qint64 getTotalSystemRam()
{
#ifdef Q_OS_WIN
    ULONGLONG totalMemoryInKilobytes = 0;
    if (GetPhysicallyInstalledSystemMemory(&totalMemoryInKilobytes)) {
        return totalMemoryInKilobytes * 1024;
    }

    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#elif defined(Q_OS_LINUX)
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return info.totalram * info.mem_unit;
    }
    return 0;
#elif defined(Q_OS_MACOS)
    int mib[2];
    size_t len;
    uint64_t memsize;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    len = sizeof(memsize);
    if (sysctl(mib, 2, &memsize, &len, NULL, 0) == 0) {
        return memsize;
    }
    return 0;
#else
    return 0; // Not supported on this OS
#endif
}

bool isNetworkAvailable()
{
    const auto allNetworkInterfaces = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &iface, allNetworkInterfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp)
            && iface.flags().testFlag(QNetworkInterface::IsRunning)
            && !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            foreach (const QHostAddress &addr, iface.allAddresses()) {
                if (addr.protocol() == QAbstractSocket::IPv4Protocol
                    && addr != QHostAddress(QHostAddress::LocalHost)) {
                    return true;
                }
            }
        }
    }
    return false;
}
} // namespace

QString SystemRequirements::describe(const QList<Aspect> aspects)
{
    QStringList descriptions;
    for (Aspect aspect : aspects) {
        switch (aspect) {
        case CpuCore:
            descriptions << QObject::tr("At least 2 CPU Cores");
            break;
        case ScreenResolution:
            descriptions << QObject::tr("Screen resolution of at least 1366x768");
            break;
        case RAM:
            descriptions << QObject::tr("At least 8 GB of RAM");
            break;
        case DiskSpace:
            descriptions << QObject::tr("At least 10 GB of free disk space");
            break;
        case OpenGLSupport:
            descriptions << QObject::tr("OpenGL 2.1 or higher support");
            break;
        case MinimumOSVersion:
#if defined(Q_OS_WIN)
            descriptions << QObject::tr("Windows 10 or later");
#elif defined(Q_OS_MACOS)
            descriptions << QObject::tr("macOS 10.15 (Catalina) or later");
#elif defined(Q_OS_LINUX)
            descriptions << QObject::tr(
                    "A modern Linux distribution (like Ubuntu 20.04+, CentOS/RHEL 8+, Fedora 30+, "
                    "Mint 20+, openSuSE 15.2+)");
#else
            descriptions << QObject::tr("A supported operating system");
#endif
            break;
        case SupportedScriteVersion:
            descriptions << QObject::tr("Support for Scrite version ")
                            + QString::fromLatin1(SCRITE_VERSION);
        default:
            break;
        }
    }
    return descriptions.join("\n");
}

QList<SystemRequirements::Aspect> SystemRequirements::check(const QList<Aspect> aspects,
                                                            QProgressDialog *progressDialog)
{
    QList<Aspect> failedChecks;
    int checksToPerform = aspects.isEmpty() ? AspectCount : aspects.count();
    int checksPerformed = 0;

    if (progressDialog) {
        progressDialog->setRange(0, checksToPerform);
        progressDialog->setValue(0);
    }

    if (aspects.isEmpty() || aspects.contains(CpuCore)) {
        if (!hasMinimumCpuCores()) {
            failedChecks << CpuCore;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }
    if (aspects.isEmpty() || aspects.contains(ScreenResolution)) {
        if (!hasMinimumScreenResolution()) {
            failedChecks << ScreenResolution;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }
    if (aspects.isEmpty() || aspects.contains(RAM)) {
        if (!hasMinimumRam()) {
            failedChecks << RAM;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }
    if (aspects.isEmpty() || aspects.contains(DiskSpace)) {
        if (!hasMinimumFreeDiskSpace()) {
            failedChecks << DiskSpace;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }
    if (aspects.isEmpty() || aspects.contains(OpenGLSupport)) {
        if (!hasOpenGlSupport()) {
            failedChecks << OpenGLSupport;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }
    if (aspects.isEmpty() || aspects.contains(MinimumOSVersion)) {
        if (!hasMinimumOSVersion()) {
            failedChecks << MinimumOSVersion;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }
    if (aspects.isEmpty() || aspects.contains(SupportedScriteVersion)) {
        if (!hasSupportedScriteVersion()) {
            failedChecks << SupportedScriteVersion;
        }
        if (progressDialog)
            progressDialog->setValue(++checksPerformed);
    }

    return failedChecks;
}

bool SystemRequirements::checkAndReport(const QList<Aspect> aspects)
{
    QList<Aspect> aspectsToCheck = aspects;
    if (aspectsToCheck.isEmpty()) {
        for (int i = 0; i < AspectCount; i++) {
            aspectsToCheck << Aspect(i);
        }
    }

    QString labelText;
    QTextStream ts(&labelText);
    ts << "<ul>";
    for (int i = 0; i < AspectCount; i++)
        ts << "<li>" << describe({ Aspect(i) }) << "</li>";
    ts << "</ul>";
    ts.flush();

    QProgressDialog msgBox(nullptr, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    msgBox.setWindowTitle(QObject::tr("Scrite is Checking System Requirements .."));
    msgBox.setLabelText(labelText);
    QLabel *label = msgBox.findChild<QLabel *>();
    if (label) {
        label->setAlignment(Qt::AlignLeft);
        label->setMargin(15);
    }
    msgBox.setCancelButton(nullptr);
    msgBox.show();

#if 0
    QEventLoop eventLoop;
    QTimer::singleShot(2000, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
#endif

    const QList<Aspect> mandatoryAspects({ Aspect::CpuCore, Aspect::SupportedScriteVersion });
    const QList<Aspect> failedChecks = check(aspectsToCheck, &msgBox);

    bool allowWithWarning = [=]() -> bool {
        for (Aspect aspect : failedChecks) {
            if (mandatoryAspects.contains(aspect))
                return false;
        }
        return true;
    }();

    if (!failedChecks.isEmpty()) {
        const QString errorString = describe(failedChecks);

        if (allowWithWarning) {
            QMessageBox::StandardButton answer = QMessageBox::question(
                    nullptr, QObject::tr("System Requirements Not Met"),
                    QObject::tr("The following requirements were not met:\n\n") + errorString
                            + QObject::tr("\n\nYou may notice issues with performance and "
                                          "function. Do you want to "
                                          "continue using Scrite anyway?"),
                    QMessageBox::Yes | QMessageBox::No);
            return answer == QMessageBox::Yes;
        } else {
            aspectsToCheck.removeOne(Aspect::SupportedScriteVersion);

            QString message = QObject::tr("The following mandatory requirements were not met:\n\n")
                    + errorString
                    + QObject::tr("\n\nScrite cannot be used on this device. Please use a "
                                  "device that supports these requirements:\n\n")
                    + describe(aspectsToCheck);

            if (failedChecks.contains(Aspect::SupportedScriteVersion)) {
                message += QObject::tr("\n\nAdditionally, please download a supported version of "
                                       "Scrite, since version ")
                        + QString::fromLatin1(SCRITE_VERSION)
                        + QObject::tr(" is not supported anymore. Click Ok to visit the downloads "
                                      "page on the Scrite "
                                      "website to download the latest supported version.");
            }

            QMessageBox::information(nullptr, QObject::tr("System Requirements Not Met"), message);

            if (failedChecks.contains(Aspect::SupportedScriteVersion)) {
#ifdef Q_OS_WIN
                QDesktopServices::openUrl(QUrl("https://www.scrite.io/download-windows/"));
#elif defined(Q_OS_MACOS)
                QDesktopServices::openUrl(QUrl("https://www.scrite.io/download-macOS/"));
#elif defined(Q_OS_LINUX)
                QDesktopServices::openUrl(QUrl("https://www.scrite.io/download-linux/"));
#else
                QDesktopServices::openUrl(QUrl("https://www.scrite.io/downloads/"));
#endif
            }
        }

        return false;
    }

    return true;
}

bool SystemRequirements::hasMinimumCpuCores(int minCores)
{
    return QThread::idealThreadCount() >= minCores;
}

bool SystemRequirements::hasMinimumScreenResolution(int minWidth, int minHeight)
{
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        QRect availableGeometry = primaryScreen->availableGeometry();
        return availableGeometry.width() >= minWidth && availableGeometry.height() >= minHeight;
    }
    return false;
}

bool SystemRequirements::hasMinimumRam(qint64 minRamGb)
{
    return (getTotalSystemRam() / (1024 * 1024 * 1024)) >= minRamGb;
}

bool SystemRequirements::hasMinimumFreeDiskSpace(qint64 minSpaceGb)
{
    QStorageInfo root = QStorageInfo::root();
    return (root.bytesAvailable() / (1024 * 1024 * 1024)) >= minSpaceGb;
}

bool SystemRequirements::hasMinimumVram(qint64 minVramMb)
{
    // This is a placeholder. Getting VRAM is complex and platform-specific.
    // Qt does not provide a direct way to query VRAM.
    Q_UNUSED(minVramMb);
    return true;
}

bool SystemRequirements::hasOpenGlSupport()
{
    QSurfaceFormat format;
    format.setVersion(2, 1);

    QOffscreenSurface surface;
    surface.setFormat(format);
    surface.create();

    if (!surface.isValid())
        return false;

    QOpenGLContext context;
    context.setFormat(format);
    if (!context.create())
        return false;

    return true;
}

bool SystemRequirements::hasMinimumOSVersion()
{
#ifdef Q_OS_WIN
    // Qt 5.15 requires Windows 7 or later.
    // but we recommend Windows 10 or later.
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;
#elif defined(Q_OS_MACOS)
    // Qt 5.15 requires macOS 10.13 (High Sierra) or later.
    // But we recommend macOS 10.15 (Catalina) or later.
    return QOperatingSystemVersion::current() >= QOperatingSystemVersion::MacOSCatalina;
#elif defined(Q_OS_LINUX)
    // For Linux, dependency versions (like glibc) are more critical than the
    // kernel or distribution version. If the application is running, it's
    // very likely the dependencies are met. A specific version check is
    // complex and may not be reliable across all distributions.
    // We will assume the OS is sufficient if the app is running.
    return true;
#else
    // For other OSes, assume it's supported.
    return true;
#endif
}

bool SystemRequirements::hasSupportedScriteVersion()
{
    bool success = true;

    if (isNetworkAvailable()) {
        AppMinimumVersionRestApiCall call;
        QEventLoop eventLoop;

        QObject::connect(&call, &AppMinimumVersionRestApiCall::finished, &eventLoop,
                         &QEventLoop::quit);

        call.setReportNetworkErrors(true);
        call.setAutoDelete(false);
        call.call();

        eventLoop.exec();

        success = call.hasError() ? true : (call.hasResponse() && call.isVersionSupported());
    }

    return success;
}
