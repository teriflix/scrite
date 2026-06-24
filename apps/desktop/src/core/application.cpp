/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "form.h"
#include "utils.h"
#include "scrite.h"
#include "undoredo.h"
#include "hourglass.h"
#include "autoupdate.h"
#include "appwindow.h"
#include "application.h"
#include "notification.h"
#include "localstorage.h"
#include "scritedocument.h"

#ifdef ENABLE_CRASHPAD_CRASH_TEST
#include "crashpadmodule.h"
#endif

// Needed for MSIX AppData path translation (production Windows builds only).
#if defined(Q_OS_WIN) && defined(SCRITE_PRODUCTION_BUILD)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <appmodel.h>
#include <ShlObj.h>
#endif

#include <QDir>
#include <QUuid>
#include <QtMath>
#include <QScreen>
#include <QtDebug>
#include <QCursor>
#include <QScreen>
#include <QWindow>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QKeyEvent>
#include <QDateTime>
#include <QMetaEnum>
#include <QHostInfo>
#include <QQmlEngine>
#include <QQuickItem>
#include <QJsonArray>
#include <QClipboard>
#include <QQmlEngine>
#include <QSslSocket>
#include <QQuickStyle>
#include <QMessageBox>
#include <QJsonObject>
#include <QQmlContext>
#include <QColorDialog>
#include <QQuickWindow>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QStandardPaths>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QOperatingSystemVersion>

bool QtApplicationEventNotificationCallback(void **cbdata);

void ApplicationQtMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                 const QString &message)
{
#if QT_NO_DEBUG_OUTPUT
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(message)
#else
    QString logMessage;

    QTextStream ts(&logMessage, QIODevice::WriteOnly);
    switch (type) {
    case QtDebugMsg:
        ts << "Debug: ";
        break;
    case QtWarningMsg:
        ts << "Warning: ";
        break;
    case QtCriticalMsg:
        ts << "Critical: ";
        break;
    case QtFatalMsg:
        ts << "Fatal: ";
        break;
    case QtInfoMsg:
        ts << "Info: ";
        break;
    }

    const char *where = context.function ? context.function : context.file;
    static const char *somewhere = "Somewhere";
    if (where == nullptr)
        where = somewhere;

    ts << "[" << where << " / " << context.line << "] - ";
    ts << message;
    ts.flush();

    fprintf(stderr, "%s\n", qPrintable(logMessage));
#endif
}

Application *Application::instance()
{
    return qobject_cast<Application *>(qApp);
}

Application::Application(int &argc, char **argv, const QVersionNumber &version)
    : QApplication(argc, argv), m_versionNumber(version)
{
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Rubik/Rubik-BoldItalic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Rubik/Rubik-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Rubik/Rubik-Italic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Rubik/Rubik-Bold.ttf"));
    this->setFont(QFont(QStringLiteral("Rubik")));

    this->setWindowIcon(QIcon(":/images/appicon.png"));
    this->setBaseWindowTitle(Application::applicationName() + " "
                             + Application::applicationVersion());

    const QString settingsFile =
            QDir(Application::appDataLocation()).absoluteFilePath("settings.ini");
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
    this->installationId();
    this->installationTimestamp();
    m_settings->setValue(QStringLiteral("Installation/launchCount"), this->launchCounter() + 1);

    if (m_settings->value(QStringLiteral("Installation/fileTypeRegistered"), false).toBool()
        == false) {
        const bool rft = this->registerFileTypes();
        m_settings->setValue(QStringLiteral("Installation/fileTypeRegistered"), rft);
        if (rft)
            m_settings->setValue(QStringLiteral("Installation/path"), this->applicationFilePath());
    }

    if (LocalStorage::load(LocalStorage::email).isNull()) {
        const QString email = m_settings->value("Registration/email").toString();
        if (!email.isEmpty())
            LocalStorage::store(LocalStorage::email, email);
    }

#ifndef QT_NO_DEBUG_OUTPUT
    QInternal::registerCallback(QInternal::EventNotifyCallback,
                                QtApplicationEventNotificationCallback);
#endif

    const QVersionNumber sversion = QVersionNumber::fromString(
            m_settings->value(QStringLiteral("Installation/version")).toString());
    if (sversion.isNull() || sversion == QVersionNumber(0, 4, 7)) {
        // until we can fix https://github.com/teriflix/scrite/issues/138
        m_settings->setValue("Screenplay Editor/enableSpellCheck", false);
    }
    m_settings->setValue(QStringLiteral("Installation/version"), m_versionNumber.toString());

    if (sversion.isNull() || sversion <= QVersionNumber(0, 5, 3)) {
        const QString customResKey = QStringLiteral("ScreenplayPageLayout/customResolution");
        const QString forceCustomResKey =
                QStringLiteral("ScreenplayPageLayout/forceCustomResolution");
        const bool customResAlreadySet = m_settings->value(customResKey, 0).toDouble() > 0;
        const bool forceCustomRes =
                !customResAlreadySet && m_settings->value(forceCustomResKey, true).toBool();
        if (forceCustomRes) {
#ifdef Q_OS_MAC
            m_settings->setValue(customResKey, 72);
#endif
#ifdef Q_OS_WIN
            m_settings->setValue(customResKey, 96);
#endif
        }

        m_settings->setValue(forceCustomResKey, false);
    }

    m_settings->sync();

    auto systemFonts = QtConcurrent::run(&Application::systemFontInfo);
    Q_UNUSED(systemFonts)

    this->setWindowIcon(QIcon(QStringLiteral(":/images/appicon.png")));

    m_customFontPointSize = [=]() -> int {
        const QVariant val = m_settings->value(QLatin1String("Application/customFontPointSize"));
        if (val.isValid() && !val.isNull() && val.canConvert(QMetaType(QMetaType::Int)))
            return val.toInt();
        return 0;
    }();
    this->computeIdealFontPointSize();

    QSurfaceFormat surfaceFormat = QSurfaceFormat::defaultFormat();
    const QByteArray envOpenGLMultisampling =
            qgetenv("SCRITE_OPENGL_MULTISAMPLING").toUpper().trimmed();
    if (envOpenGLMultisampling == QByteArrayLiteral("FULL"))
        surfaceFormat.setSamples(4);
    else if (envOpenGLMultisampling == QByteArrayLiteral("EXTREME"))
        surfaceFormat.setSamples(8);
    else if (envOpenGLMultisampling == QByteArrayLiteral("HALF"))
        surfaceFormat.setSamples(2);
    else
        surfaceFormat.setSamples(-1); // default
    QSurfaceFormat::setDefaultFormat(surfaceFormat);

    const bool useSoftwareRenderer = [=]() -> bool {
#ifdef Q_OS_WIN
        if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10)
            return true;
#endif

        QOpenGLContext context;
        context.setFormat(surfaceFormat);
        if (!context.create())
            return true;

        return m_settings->value(QStringLiteral("Application/useSoftwareRenderer"), false).toBool();
    }();
    const QString style = [=]() -> QString {
        const QString ret = m_settings->value(QStringLiteral("Application/uiTheme")).toString();
        const QStringList themes = Application::availableThemes();
        const QString resolved = themes.contains(ret) ? ret : themes.first();
        if (useSoftwareRenderer && resolved != themes.last())
            return themes.last();
        return resolved;
    }();

    if (useSoftwareRenderer)
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);

#ifndef Q_OS_MAC
#ifdef Q_OS_UNIX
    const QString libPath = Application::applicationDirPath() + "/../lib";
    Application::addLibraryPath(libPath);
    Application::addLibraryPath(Application::applicationDirPath());
#endif
#endif

    QQuickStyle::setStyle(style);

    connect(this, SIGNAL(applicationStateChanged(Qt::ApplicationState)), this,
            SIGNAL(appStateChanged()));
}

static void copyFilesRecursively(const QDir &from, const QDir &to)
{
    const QFileInfoList fromList =
            from.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &fromFile : fromList) {
        if (fromFile.isFile())
            QFile::copy(fromFile.absoluteFilePath(), to.absoluteFilePath(fromFile.fileName()));
        else if (fromFile.isDir()) {
            QDir fromDir = from;
            fromDir.cd(fromFile.fileName());

            QDir toDir = to;
            toDir.mkdir(fromFile.fileName());
            toDir.cd(fromFile.fileName());
            copyFilesRecursively(fromDir, toDir);
        }
    }
}

static const QString legacyDataMovedKey = QStringLiteral("Migration/legacyDataMoved");

QVersionNumber Application::prepare()
{
    const QVersionNumber applicationVersion =
            QVersionNumber::fromString(QStringLiteral(SCRITE_VERSION));
    const QString applicationVersionString = [applicationVersion]() -> QString {
        QStringList ret = { QString::number(applicationVersion.majorVersion()),
                            QString::number(applicationVersion.minorVersion()) };

        if (applicationVersion.microVersion() > 0)
            ret << QString::number(applicationVersion.microVersion());

        const QVector<int> segments = applicationVersion.segments();
        for (int i = 3; i < segments.size(); i++) {
            QString field;
            int segment = qMax(0, segments.at(i));
            while (--segment >= 0) {
                field = QChar('a' + segment % 26) + field;
                segment /= 26;
            }
            ret.last() += field;
        }

        QString verStr = ret.join('.');

        const QString verType = QStringLiteral(SCRITE_VERSION_TYPE);
        if (!verType.isEmpty())
            verStr += "-" + verType;

        return verStr;
    }();

    if (qApp != nullptr)
        return applicationVersion;

    qInstallMessageHandler(ApplicationQtMessageHandler);

    Application::setApplicationName(QStringLiteral("Scrite"));

    // SCRITE_PRODUCTION_BUILD is set via -DSCRITE_PRODUCTION_BUILD=ON at CMake
    // configure time by the production build system (external to this repo).
    // Production builds use "IEDN Technologies" as the org name and
    // migrate data from every prior org (oldest first so newest data wins).
    // Open-source builds leave this flag unset and keep "Scrite" as the org name.
    struct LegacyOrg
    {
        const char *name;
        const char *domain;
        bool remove = false;
    };
#ifdef SCRITE_PRODUCTION_BUILD
    // Org history: TERIFLIX → VCreate Logic Pvt. Ltd. → Scrite → IEDN Technologies
    const LegacyOrg legacyOrgs[] = {
        { "TERIFLIX", "teriflix.com", true },
        { "VCreate Logic Pvt. Ltd.", "vcreatelogic.com", true },
        { "Scrite", "scrite.io", false },
    };
    Application::setOrganizationName(QStringLiteral("IEDN Technologies"));
    Application::setOrganizationDomain(QStringLiteral("scrite.io"));
#else
    // Org history: TERIFLIX → VCreate Logic Pvt. Ltd. → Scrite (current)
    const LegacyOrg legacyOrgs[] = {
        { "TERIFLIX", "teriflix.com", true },
        { "VCreate Logic Pvt. Ltd.", "vcreatelogic.com", true },
    };
    Application::setOrganizationName(QStringLiteral("Scrite"));
    Application::setOrganizationDomain(QStringLiteral("scrite.io"));
#endif

    const QString targetAppDataPath = Application::appDataLocation();
    bool legacyDataMigrated = false;

    if (!QDir(targetAppDataPath).exists("settings.ini")) {

        for (const LegacyOrg &legacy : legacyOrgs) {
            Application::setOrganizationName(QLatin1String(legacy.name));
            Application::setOrganizationDomain(QLatin1String(legacy.domain));
            const QDir legacyDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
            if (legacyDir.exists()) {
                QDir().mkpath(targetAppDataPath);
                copyFilesRecursively(legacyDir, QDir(targetAppDataPath));
                if (legacy.remove)
                    QDir(legacyDir).removeRecursively();
                legacyDataMigrated = true;
            }
        }
    }

    // Restore the target org name after the migration loop changed it.
#ifdef SCRITE_PRODUCTION_BUILD
    Application::setOrganizationName(QStringLiteral("IEDN Technologies"));
    Application::setOrganizationDomain(QStringLiteral("scrite.io"));
#else
    Application::setOrganizationName(QStringLiteral("Scrite"));
    Application::setOrganizationDomain(QStringLiteral("scrite.io"));
#endif

#ifdef Q_OS_UNIX
    Application::setApplicationVersion(applicationVersionString + " (GNU Linux)");
#endif

#ifdef Q_OS_MAC
    Application::setApplicationVersion(applicationVersionString + QStringLiteral(" (macOS)"));
    if (QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSCatalina)
        qputenv("QT_MAC_WANTS_LAYER", QByteArrayLiteral("1"));
#endif

#ifdef Q_OS_WIN
    Application::setApplicationVersion(applicationVersionString + " (Windows 64-bit)");
#endif

    /*QPalette palette = Application::palette();
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor::fromRgbF(0, 0.4, 1));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(Qt::white));
    palette.setColor(QPalette::Active, QPalette::Text, QColor(Qt::black));
    Application::setPalette(palette);*/

    if (legacyDataMigrated) {
        // Note, this doesnt go into settings.ini. And that's okay.
        QSettings settings;
        settings.setValue(legacyDataMovedKey, true);
    }

    return applicationVersion;
}

QString Application::appDataLocation()
{
    // On Windows, production builds are distributed as MSIX packages which virtualize writes to
    // AppData\Roaming and AppData\Local, redirecting them into a package-private store that is
    // deleted on uninstall. A dot-folder directly under the user's home directory sits outside
    // MSIX's VFS redirect list entirely, so data written there survives uninstall/reinstall.
#if defined(Q_OS_WIN) && defined(SCRITE_PRODUCTION_BUILD)
    const QString location = QDir::homePath() + QLatin1String("/.scrite");
    QDir().mkpath(location);
    return location;
#endif
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

Application::~Application()
{
#ifndef QT_NO_DEBUG_OUTPUT
    QInternal::unregisterCallback(QInternal::EventNotifyCallback,
                                  QtApplicationEventNotificationCallback);
#endif
}

QString Application::deviceId() const
{
    static QString ret;
    if (ret.isEmpty()) {
        ret = QString::fromLatin1(QSysInfo::machineUniqueId().toHex());
        if (!ret.isEmpty())
            return ret;

        const QString deviceIdKey = QStringLiteral("deviceId");
        ret = LocalStorage::load(deviceIdKey).toString();
        if (!ret.isEmpty())
            return ret;

        ret = QUuid::createUuid().toString();
        LocalStorage::store(deviceIdKey, ret);
    }

    return ret;
}

QString Application::installationId() const
{
    QString clientID = m_settings->value("Installation/ClientID").toString();
    if (clientID.isEmpty()) {
        clientID = QUuid::createUuid().toString();
        m_settings->setValue("Installation/ClientID", clientID);
    }

    return clientID;
}

QDateTime Application::installationTimestamp() const
{
    QString installTimestampStr =
            m_settings->value(QLatin1String("Installation/timestamp")).toString();
    QDateTime installTimestamp = QDateTime::fromString(installTimestampStr);
    if (installTimestampStr.isEmpty() || !installTimestamp.isValid()) {
        installTimestamp = QDateTime::currentDateTime();
        installTimestampStr = installTimestamp.toString();
        m_settings->setValue(QLatin1String("Installation/timestamp"), installTimestampStr);
    }

    return installTimestamp;
}

int Application::appState() const
{
    return Scrite::ApplicationState((int)QApplication::applicationState());
}

int Application::launchCounter() const
{
    return m_settings->value("Installation/launchCount", 0).toInt();
}

void Application::setCustomFontPointSize(int val)
{
    if (m_customFontPointSize == val || val < 0 || val >= 100)
        return;

    m_customFontPointSize = val;
    m_settings->setValue(QLatin1String("Application/customFontPointSize"), val);

    emit customFontPointSizeChanged();

    this->computeIdealFontPointSize();
}

const QString Application::versionType = QStringLiteral(SCRITE_VERSION_TYPE);

QStringList Application::availableThemes()
{
    // ORDERING RULE: first entry = best style for this platform (used as the default when no
    // preference is stored); last entry = software-rendering fallback (must require no GPU).
    // The Application constructor relies on first() and last() — never break this contract.
    static QStringList themes;
    if (themes.isEmpty()) {
#ifdef Q_OS_WIN
        if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11)
            themes << QStringLiteral("FluentWinUI3");
        themes << QStringLiteral("Material");
#elif defined(Q_OS_MACOS)
        themes << QStringLiteral("Material") << QStringLiteral("macOS");
#else
        themes << QStringLiteral("Material");
#endif
        themes << QStringLiteral("Basic");
    }

    return themes;
}

bool Application::usingGpuAcceleratedTheme()
{
    const QString name = QQuickStyle::name();
    return name == QStringLiteral("Material") || name == QStringLiteral("FluentWinUI3");
}

void Application::setBaseWindowTitle(const QString &val)
{
    if (m_baseWindowTitle == val)
        return;

    m_baseWindowTitle = val;
    emit baseWindowTitleChanged();
}

QJsonObject Application::systemFontInfo()
{
    HourGlass hourGlass;

    static QJsonObject ret;

    static QMutex retLock;
    QMutexLocker locker(&retLock);

    if (ret.isEmpty()) {
        // Load all fonts, we will need it at some point anyway
        QFontDatabase::families();

        const QStringList allFamilies = QFontDatabase::families();
        QStringList families;
        std::copy_if(allFamilies.begin(), allFamilies.end(), std::back_inserter(families),
                     [](const QString &family) { return !QFontDatabase::isPrivateFamily(family); });
        ret.insert("families", QJsonArray::fromStringList(families));

        QJsonArray sizes;
        const QList<int> stdSizes = QFontDatabase::standardSizes();
        for (int stdSize : stdSizes)
            sizes.append(QJsonValue(stdSize));
        ret.insert("standardSizes", sizes);
    }

    return ret;
}

#if defined(Q_OS_WIN) && defined(SCRITE_PRODUCTION_BUILD)
// Translates a virtual AppData\Roaming path (as seen by the app inside the MSIX
// container) to the physical package-specific path that Explorer, running outside
// the container, can actually open. Returns the original path unchanged for paths
// outside AppData\Roaming or when the app is not running as an MSIX package.
static QString msixPhysicalPath(const QString &virtualPath)
{
    struct MsixPaths
    {
        QString virtualRoaming;
        QString physicalRoaming;
    };
    static const MsixPaths s = []() -> MsixPaths {
        UINT32 len = 0;
        if (GetPackageFamilyName(GetCurrentProcess(), &len, nullptr) != ERROR_INSUFFICIENT_BUFFER)
            return {}; // not running as an MSIX package

        QVector<wchar_t> familyName(len);
        if (GetPackageFamilyName(GetCurrentProcess(), &len, familyName.data()) != ERROR_SUCCESS)
            return {};

        PWSTR pRoaming = nullptr;
        PWSTR pLocal = nullptr;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_NO_PACKAGE_REDIRECTION, nullptr,
                             &pRoaming);
        SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_NO_PACKAGE_REDIRECTION, nullptr,
                             &pLocal);

        if (!pRoaming || !pLocal) {
            if (pRoaming)
                CoTaskMemFree(pRoaming);
            if (pLocal)
                CoTaskMemFree(pLocal);
            return {};
        }

        MsixPaths result;
        result.virtualRoaming = QDir::fromNativeSeparators(QString::fromWCharArray(pRoaming));
        result.physicalRoaming = QDir::fromNativeSeparators(QString::fromWCharArray(pLocal))
                + QLatin1String("/Packages/") + QString::fromWCharArray(familyName.data())
                + QLatin1String("/LocalCache/Roaming");
        CoTaskMemFree(pRoaming);
        CoTaskMemFree(pLocal);
        return result;
    }();

    if (s.virtualRoaming.isEmpty())
        return virtualPath;

    const QString normalized = QDir::fromNativeSeparators(virtualPath);
    if (normalized.startsWith(s.virtualRoaming, Qt::CaseInsensitive))
        return s.physicalRoaming + normalized.mid(s.virtualRoaming.length());

    return virtualPath;
}
#endif // Q_OS_WIN && SCRITE_PRODUCTION_BUILD

void Application::revealFileOnDesktop(const QString &pathIn)
{
    m_errorReport->clear();

    // The implementation of this function is inspired from QtCreator's
    // implementation of FileUtils::showInGraphicalShell() method
    const QFileInfo fileInfo(pathIn);

    // Mac, Windows support folder or file.
    if (Utils::Platform::isWindowsDesktop()) {
        const QString explorer = QStandardPaths::findExecutable("explorer.exe");
        if (explorer.isEmpty()) {
            m_errorReport->setErrorMessage(
                    "Could not find explorer.exe in path to launch Windows Explorer.");
            return;
        }

        QStringList param;
        if (!fileInfo.isDir())
            param += QLatin1String("/select,");
#if defined(Q_OS_WIN) && defined(SCRITE_PRODUCTION_BUILD)
        param += QDir::toNativeSeparators(msixPhysicalPath(fileInfo.canonicalFilePath()));
#else
        param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
#endif
        QProcess::startDetached(explorer, param);
    } else if (Utils::Platform::isMacOSDesktop()) {
        QStringList scriptArgs;
        scriptArgs << QLatin1String("-e")
                   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                              .arg(fileInfo.canonicalFilePath());
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
        scriptArgs.clear();
        scriptArgs << QLatin1String("-e")
                   << QLatin1String("tell application \"Finder\" to activate");
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    } else {
#if 0 // TODO
      // we cannot select a file here, because no file browser really supports it...
        const QString folder = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
        const QString app = UnixUtils::fileBrowser(ICore::settings());
        QProcess browserProc;
        const QString browserArgs = UnixUtils::substituteFileBrowserParameters(app, folder);
        bool success = browserProc.startDetached(browserArgs);
        const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
        success = success && error.isEmpty();
        if (!success)
            showGraphicalShellError(parent, app, error);
#endif
    }

    const QFileInfo fi(pathIn);

    Notification *notification = new Notification(this);
    connect(notification, &Notification::dismissed, &Notification::deleteLater);
    if (fi.exists()) {
        notification->setTitle(QStringLiteral("File available"));

#ifdef Q_OS_MAC
        notification->setText(QStringLiteral("Revealing <b>%1</b> in '<i>%2</i>'...")
                                      .arg(fi.fileName(), fi.absolutePath()));
#else
#ifdef Q_OS_WIN
#ifdef SCRITE_PRODUCTION_BUILD
        const QString notificationPath = msixPhysicalPath(fi.absolutePath());
#else
        const QString notificationPath = fi.absolutePath();
#endif
        notification->setText(
                QStringLiteral("<b>%1</b> is available at '<i>%2</i>'. You can take a look at the "
                               "file by switching to the Explorer window which has been "
                               "opened in the background with this file selected.")
                        .arg(fi.fileName(), notificationPath));
#else
        notification->setText(
                QStringLiteral("<b>%1</b> is available. Please launch your file manager app "
                               "and navigate to '<i>%2</i>' to open the file.")
                        .arg(fi.fileName(), fi.absolutePath()));
#endif
#endif

    } else {
        notification->setTitle(QStringLiteral("Unable to show path."));
        notification->setText(fi.absoluteFilePath());
    }
#ifdef Q_OS_MAC
    notification->setAutoClose(true);
#else
    notification->setAutoClose(false);
#endif
    notification->setActive(true);
}

QString Application::settingsFilePath() const
{
    return m_settings->fileName();
}

AutoUpdate *Application::autoUpdate() const
{
    return AutoUpdate::instance();
}

bool QtApplicationEventNotificationCallback(void **cbdata)
{
#ifndef QT_NO_DEBUG_OUTPUT
    QObject *object = reinterpret_cast<QObject *>(cbdata[0]);
    QEvent *event = reinterpret_cast<QEvent *>(cbdata[1]);
    bool *result = reinterpret_cast<bool *>(cbdata[2]);

    const bool ret = Application::instance()->notifyInternal(object, event);

    if (result)
        *result |= ret;

    return ret;
#else
    Q_UNUSED(cbdata)
    return false;
#endif
}

bool Application::notify(QObject *object, QEvent *event)
{
    // Note that notifyInternal() will be called first before we get here.
    if (event->type() == QEvent::DeferredDelete)
        return QApplication::notify(object, event);

#ifdef ENABLE_CRASHPAD_CRASH_TEST
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->modifiers() & Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier
            && ke->key() == Qt::Key_R) {
            CrashpadModule::crash();
            return true;
        }
    }
#endif

    const bool ret = QApplication::notify(object, event);

    // The only reason we reimplement the notify() method is because we sometimes want to
    // handle an event AFTER it is handled by the target object.

    if (event->type() == QEvent::ChildAdded) {
        QChildEvent *childEvent = reinterpret_cast<QChildEvent *>(event);
        QObject *childObject = childEvent->child();

        if (!childObject->isWidgetType() && !childObject->isWindowType()) {
            /**
             * For whatever reason, ParentChange event is only sent
             * if the child is a widget or window or declarative-item.
             * I was not aware of this up until now. Classes like
             * StructureElement, SceneElement etc assume that ParentChange
             * event will be sent when they are inserted into the document
             * object tree, so that they can evaluate a pointer to the
             * parent object in the tree. Since these classes are subclassed
             * from QObject, we will need the following lines to explicitly
             * despatch ParentChange events.
             */
            QEvent parentChangeEvent(QEvent::ParentChange);
            QApplication::notify(childObject, &parentChangeEvent);
        }
    }

    if (event->type() == QEvent::ApplicationFontChange)
        this->applicationFontChanged();

    return ret;
}

bool Application::notifyInternal(QObject *object, QEvent *event)
{
#ifndef QT_NO_DEBUG_OUTPUT
    static QHash<QObject *, QString> objectNameMap;
    auto evaluateObjectName = [](QObject *object, QHash<QObject *, QString> &from) {
        QString objectName = from.value(object);
        if (objectName.isEmpty()) {
            QQuickItem *item = qobject_cast<QQuickItem *>(object);
            QObject *parent = item && item->parentItem() ? item->parentItem() : object->parent();
            QString parentName = parent ? from.value(parent) : "No Parent";
            if (parentName.isEmpty()) {
                parentName =
                        QString("%1 [%2] (%3)")
                                .arg(parent ? parent->metaObject()->className() : "Unknown Parent")
                                .arg((unsigned long)((void *)parent), 0, 16)
                                .arg(parent->objectName());
            }
            objectName = QString("%1 [%2] (%3) under %4")
                                 .arg(object->metaObject()->className())
                                 .arg((unsigned long)((void *)object), 0, 16)
                                 .arg(object->objectName())
                                 .arg(parentName);
            from[object] = objectName;
        }
        return objectName;
    };

    if (event->type() == QEvent::DeferredDelete) {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        qDebug() << "DeferredDelete: " << objectName;
    } else if (event->type() == QEvent::Timer) {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        QTimerEvent *te = static_cast<QTimerEvent *>(event);
        ExecLaterTimer *timer = ExecLaterTimer::get(te->timerId());
        qDebug() << "TimerEventDespatch: " << te->timerId() << " on " << objectName << " is "
                 << (timer ? qPrintable(timer->name()) : "Qt Timer.");
    } else if (event->type() == QEvent::Shortcut) {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        QShortcutEvent *se = static_cast<QShortcutEvent *>(event);
        qDebug() << "ShortcutEvent: " << objectName << "-" << se->key().toString();
    }
#else
    Q_UNUSED(object)
    Q_UNUSED(event)
#endif

    return false;
}

Q_DECL_IMPORT int qt_defaultDpi();

void Application::computeIdealFontPointSize()
{
    int fontPointSize = 0;

    if (m_customFontPointSize > 0)
        fontPointSize = m_customFontPointSize;
    else {
#ifndef Q_OS_MAC
        fontPointSize = 10;
#else
        const qreal minInch = 0.12; // Font should occupy atleast 0.12 inches on the screen
        const qreal nrPointsPerInch =
                qt_defaultDpi(); // These many dots make up one inch on the screen
        const qreal scale = this->primaryScreen()->physicalDotsPerInch() / nrPointsPerInch;
        const qreal dpr = this->primaryScreen()->devicePixelRatio();
        fontPointSize = qCeil(minInch * nrPointsPerInch * qMax(dpr, scale));
#endif
    }

    if (m_idealFontPointSize != fontPointSize) {
        m_idealFontPointSize = fontPointSize;
        emit idealFontPointSizeChanged();
    }
}

bool Application::event(QEvent *event)
{
#ifdef Q_OS_MAC
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        if (m_handleFileOpenEvents)
            emit openFileRequest(openEvent->file());
        else
            m_fileToOpen = openEvent->file();
        return true;
    }
#endif
    return QApplication::event(event);
}

QScreen *Application::windowScreen(QObject *window)
{
    QWindow *qwindow = qobject_cast<QWindow *>(window);
    if (qwindow)
        return qwindow->screen();

    QWidget *qwidget = qobject_cast<QWidget *>(window);
    if (qwidget)
        return qwidget->window()->windowHandle()->screen();

    return nullptr;
}

void Application::saveWindowGeometry(QWindow *window, const QString &group)
{
    if (window == nullptr)
        return;

    const QRect geometry = window->geometry();
    if (window->visibility() == QWindow::Windowed) {
        const QString geometryString = QString("%1 %2 %3 %4")
                                               .arg(geometry.x())
                                               .arg(geometry.y())
                                               .arg(geometry.width())
                                               .arg(geometry.height());
        m_settings->setValue(group + QStringLiteral("/windowGeometry"), geometryString);
    } else
        m_settings->setValue(group + QStringLiteral("/windowGeometry"),
                             QStringLiteral("Maximized"));
}

bool Application::restoreWindowGeometry(QWindow *window, const QString &group)
{
    if (window == nullptr)
        return false;

    const QString geometryArg = QStringLiteral("--windowGeometry");
    const int geometryArgPos = this->arguments().indexOf(geometryArg);
    if (geometryArgPos >= 0 && this->arguments().size() >= geometryArgPos + 5) {
        const int x = this->arguments().at(geometryArgPos + 1).toInt();
        const int y = this->arguments().at(geometryArgPos + 2).toInt();
        const int w = this->arguments().at(geometryArgPos + 3).toInt();
        const int h = this->arguments().at(geometryArgPos + 4).toInt();
        window->setGeometry(x, y, w, h);
        return true;
    }

    const QScreen *screen = window->screen();
    const QRect screenGeo = screen->availableGeometry();

    const QString geometryString =
            m_settings->value(group + QStringLiteral("/windowGeometry")).toString();
    if (geometryString == QStringLiteral("Maximized")) {
        QTimer::singleShot(100, window, &QWindow::showMaximized);
        return true;
    }

    const QStringList geometry = geometryString.split(QStringLiteral(" "), Qt::SkipEmptyParts);
    if (geometry.length() != 4) {
        // No saved geometry — first launch. Use showMaximized() so the OS/window manager
        // places the window correctly, including title bar within the available area.
        QTimer::singleShot(100, window, &QWindow::showMaximized);
        return false;
    }

    const QString geoDeltaArg = QStringLiteral("--geodelta");
    const int geoDeltaArgPos = this->arguments().indexOf(geoDeltaArg);
    const int geoDelta =
            qBound(0,
                   geoDeltaArgPos >= 0 && this->arguments().size() >= geoDeltaArgPos + 2
                           ? this->arguments().at(geoDeltaArgPos + 1).toInt()
                           : 0,
                   100);

    const int x = qMax(geometry.at(0).toInt() + geoDelta, 10);
    const int y = qMax(geometry.at(1).toInt() + geoDelta, 50);
    const int w = qMax(geometry.at(2).toInt() + geoDelta, AppWindow::minimumWindowWidth);
    const int h = qMax(geometry.at(3).toInt() + geoDelta, AppWindow::minimumWindowHeight);
    QRect geo(x, y, w, h);
    if (!screenGeo.contains(geo)) {
        if (w >= screenGeo.width() || h >= screenGeo.height()) {
            QTimer::singleShot(100, window, &QWindow::showMaximized);
            return false;
        }

        geo.moveCenter(screenGeo.center());
    }

    window->showNormal();
    window->setGeometry(geo);
    return true;
}

void Application::launchNewInstance(QWindow *window)
{
    startNewInstance(window, QString(), false);
}

void Application::launchNewInstanceAndOpenAnonymously(QWindow *window, const QString &filePath)
{
    startNewInstance(window, filePath, true);
}

void Application::launchNewInstanceAndOpen(QWindow *window, const QString &filePath)
{
    startNewInstance(window, filePath, false);
}

void Application::startNewInstance(QWindow *window, const QString &filePath, bool anonymously)
{
    const QString appPath = this->applicationFilePath();

    QStringList args;
    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        args = QStringList({ filePath });
        if (anonymously)
            args.prepend(QStringLiteral("--openAnonymously"));
    }

    if (window != nullptr) {
        const QRect geometry = window->geometry();
        args += { QStringLiteral("--windowGeometry"), QString::number(geometry.x() + 30),
                  QString::number(geometry.y() + 30), QString::number(geometry.width()),
                  QString::number(geometry.height()) };
    } else
        args += { QStringLiteral("--geodelta"), QStringLiteral("30") };

    const QString sessionToken = LocalStorage::load(LocalStorage::sessionToken).toString();
    if (!sessionToken.isEmpty())
        args += { QStringLiteral("--sessionToken"), sessionToken };

    QProcess::startDetached(appPath, args);
}

bool Application::maybeOpenAnonymously()
{
    const QStringList args = this->arguments();
    const int oaIndex = args.indexOf(QStringLiteral("--openAnonymously"));
    if (oaIndex < 0 || oaIndex >= args.size() - 1)
        return false;

    const QString filePath = args.at(oaIndex + 1);
    if (filePath.isEmpty() || !QFile::exists(filePath))
        return false;

    ScriteDocument::instance()->openAnonymously(filePath);
    return true;
}

void Application::toggleFullscreen(QWindow *window)
{
    const char *propName = "#previouslyMaximised";
    if (window->windowStates() & Qt::WindowFullScreen) {
        const bool waxMaxed = window->property(propName).toBool();
        if (waxMaxed)
            window->setVisibility(QWindow::Maximized);
        else
            window->setVisibility(QWindow::Windowed);
    } else {
        window->setProperty(propName, window->visibility() == QWindow::FullScreen);
        window->setVisibility(QWindow::FullScreen);
    }
}

bool Application::hasLegacyDataMovedRecently() const
{
    QSettings settings;
    return settings.value(legacyDataMovedKey, false).toBool();
}

void Application::acknowledgeLegacyDataMigration()
{
    QSettings settings;
    settings.remove(legacyDataMovedKey);
}

bool Application::hasActiveFocus(QQuickWindow *window, QQuickItem *item)
{
    if (window == nullptr || item == nullptr)
        return false;

    QQuickItem *focusItem = window->activeFocusItem();
    if (focusItem == nullptr)
        return false;

    QQuickItem *i = focusItem;
    while (i != nullptr) {
        if (i == item)
            return true;
        i = i->parentItem();
    }

    return false;
}

Forms *Application::forms() const
{
    return Forms::global();
}

bool Application::registerFileTypes()
{
#ifdef Q_OS_WIN
#if 0 // Registration is done during setup
    const QString appFilePath = this->applicationFilePath();
    QSettings classes(QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\CLASSES"), QSettings::NativeFormat);

    const QString ns = QStringLiteral("com.teriflix.scrite");
    const QString root = QStringLiteral("/.");
    const QString shell = QStringLiteral("/shell/open/command/.");

    auto registerFileExtension = [&](const QString &extension, const QString &description, const QString &cmdLineOption) {
        classes.setValue(extension + root, ns + extension);
        classes.setValue(ns + extension + root, description);
        classes.setValue(ns + extension + shell, QDir::toNativeSeparators(appFilePath) + QStringLiteral(" ") + cmdLineOption + QStringLiteral(" \"%1\""));
        const bool makeDefault = (extension == QStringLiteral(".scrite"));
        if(makeDefault)
            classes.setValue(extension + QStringLiteral("/DefaultIcon/."), QDir::toNativeSeparators(appFilePath));
    };
    registerFileExtension(".scrite", "Scrite Screenplay Document", QString());
#endif // 0
    return true;
#endif

#ifdef Q_OS_MAC
    // Registration happens via Info.plist file.
    return true;
#else
#ifdef Q_OS_UNIX
    // Registration happens via .desktop file
    return true;
#endif
#endif
}

#include "qimageitem.h"
#include "structure.h"
#include "languageengine.h"
#include "colorimageprovider.h"
#include "themediconprovider.h"
#include "basicfileiconprovider.h"

void Application::initialize(QQmlEngine *engine)
{
    QObject::connect(engine, &QQmlEngine::quit, this, &Application::quit);

    // Force registration of QML types in io.scrite.components
    extern void qml_register_types_io_scrite_components();
    qml_register_types_io_scrite_components();

    // Init modules
    const char *uri = SCRITE_QML_URI;
    UndoHub::init(uri, engine);
    LanguageEngine::init(uri, engine);

    // Register image providers
    engine->addImageProvider(ImageIconProvider::name(), new ImageIconProvider);
    engine->addImageProvider(ColorImageProvider::name(), new ColorImageProvider);
    engine->addImageProvider(ThemedIconProvider::name(), new ThemedIconProvider);
    engine->addImageProvider(BasicFileIconProvider::name(), new BasicFileIconProvider);
    engine->addImageProvider(AnnotationImageProvider::name(), new AnnotationImageProvider);
}
