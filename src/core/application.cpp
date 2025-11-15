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

#include "form.h"
#include "utils.h"
#include "scrite.h"
#include "hourglass.h"
#include "autoupdate.h"
#include "application.h"
#include "notification.h"
#include "localstorage.h"
#include "scritedocument.h"

#ifdef ENABLE_CRASHPAD_CRASH_TEST
#include "crashpadmodule.h"
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
#include <QNetworkConfigurationManager>

// #define ENABLE_SCRIPT_HOTKEY

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
    : QtApplicationClass(argc, argv), m_versionNumber(version)
{
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Rubik/Rubik-BoldItalic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Rubik/Rubik-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Rubik/Rubik-Italic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Rubik/Rubik-Bold.ttf"));
    this->setFont(QFont(QStringLiteral("Rubik")));

    connect(this, &QGuiApplication::fontChanged, this, &Application::applicationFontChanged);

    this->setWindowIcon(QIcon(":/images/appicon.png"));
    this->setBaseWindowTitle(Application::applicationName() + " "
                             + Application::applicationVersion());

    const QString settingsFile =
            QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                    .absoluteFilePath("settings.ini");
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

    if (LocalStorage::load("email").isNull()) {
        const QString email = m_settings->value("Registration/email").toString();
        if (!email.isEmpty())
            LocalStorage::store("email", email);
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

    QtConcurrent::run(&Application::systemFontInfo);

    this->setWindowIcon(QIcon(QStringLiteral(":/images/appicon.png")));

    m_customFontPointSize = [=]() -> int {
        const QVariant val = m_settings->value(QLatin1String("Application/customFontPointSize"));
        if (val.isValid() && !val.isNull() && val.canConvert(QMetaType::Int))
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
        const QString ret = m_settings->value(QStringLiteral("Application/theme")).toString();
        if (useSoftwareRenderer && ret == QStringLiteral("Material"))
            return QStringLiteral("Default");
        return Application::queryQtQuickStyleFor(ret);
    }();

    if (useSoftwareRenderer)
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);

#ifndef Q_OS_MAC
#ifdef Q_OS_UNIX
    const QString libPath = Application::applicationDirPath() + "/../lib";
    Application::addLibraryPath(libPath);
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

        return ret.join('.');
    }();

    if (qApp != nullptr)
        return applicationVersion;

    qInstallMessageHandler(ApplicationQtMessageHandler);

    Application::setApplicationName(QStringLiteral("Scrite"));
    Application::setOrganizationName(QStringLiteral("TERIFLIX"));
    Application::setOrganizationDomain(QStringLiteral("teriflix.com"));

    const QDir oldAppDataFolder =
            QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    Application::setOrganizationName(QStringLiteral("Scrite"));
    Application::setOrganizationDomain(QStringLiteral("scrite.io"));

    if (oldAppDataFolder.exists()) {
        const QString newAppDataPath =
                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(newAppDataPath);
        copyFilesRecursively(oldAppDataFolder, QDir(newAppDataPath));
        QDir(oldAppDataFolder).removeRecursively();
    }

#ifdef Q_OS_UNIX
    Application::setApplicationVersion(applicationVersionString + " (GNU Linux)");
#endif

#ifdef Q_OS_MAC
    Application::setApplicationVersion(applicationVersionString + QStringLiteral(" (macOS)"));
    if (QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSCatalina)
        qputenv("QT_MAC_WANTS_LAYER", QByteArrayLiteral("1"));
#endif

#ifdef Q_OS_WIN
    if (QSysInfo::WordSize == 32)
        Application::setApplicationVersion(applicationVersionString + " (Windows 32-bit)");
    else
        Application::setApplicationVersion(applicationVersionString + " (Windows 64-bit)");
#endif

#ifdef Q_OS_WIN
    Application::setAttribute(Qt::AA_UseDesktopOpenGL);

    // Maybe helps address https://www.github.com/teriflix/scrite/issues/247
    const QByteArray dpiMode = qgetenv("SCRITE_DPI_MODE").trimmed();
    if (dpiMode.isEmpty()) {
        const qreal uiScaleFactor =
                Utils::SystemEnvironment::get(QLatin1String("SCRITE_UI_SCALE_FACTOR"),
                                              QLatin1String("1.0"))
                        .toDouble();
        const QByteArray qtScaleFactor =
                QByteArray::number(qRound(qBound(0.1, uiScaleFactor, 10.0) * 100) / 100.0);
        Application::setAttribute(Qt::AA_UseHighDpiPixmaps);
        Application::setAttribute(Qt::AA_Use96Dpi);
        Application::setAttribute(Qt::AA_DisableHighDpiScaling);
        qputenv("QT_SCALE_FACTOR", qtScaleFactor);
    } else {
        if (dpiMode == QByteArrayLiteral("HIGH_DPI")) {
            Application::setAttribute(Qt::AA_EnableHighDpiScaling);
            Application::setAttribute(Qt::AA_UseHighDpiPixmaps);
        } else /*if (dpiMode == QByteArrayLiteral("96_DPI_ONLY"))*/ {
            Application::setAttribute(Qt::AA_Use96Dpi);
            Application::setAttribute(Qt::AA_DisableHighDpiScaling);
        }
    }
#endif

    QPalette palette = Application::palette();
    palette.setColor(QPalette::Active, QPalette::Highlight, QColor::fromRgbF(0, 0.4, 1));
    palette.setColor(QPalette::Active, QPalette::HighlightedText, QColor(Qt::white));
    palette.setColor(QPalette::Active, QPalette::Text, QColor(Qt::black));
    Application::setPalette(palette);

    return applicationVersion;
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
    return Scrite::ApplicationState((int)QtApplicationClass::applicationState());
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

QStringList Application::availableThemes()
{
    // return QQuickStyle::availableStyles();
    static QStringList themes({ QStringLiteral("Basic"), QStringLiteral("Fusion"),
                                QStringLiteral("Imagine"), QStringLiteral("Material"),
                                QStringLiteral("Universal") });
    return themes;
}

QString Application::queryQtQuickStyleFor(const QString &theme)
{
    const QString defaultStyle = QStringLiteral("Material");
    if (theme.isEmpty())
        return defaultStyle;

    const int idx = availableThemes().indexOf(theme);
    if (idx < 0)
        return defaultStyle;
    if (idx == 0)
        return QStringLiteral("Default");
    return theme;
}

bool Application::usingMaterialTheme()
{
    return QQuickStyle::name() == QStringLiteral("Material");
}

void Application::setBaseWindowTitle(const QString &val)
{
    if (m_baseWindowTitle == val)
        return;

    m_baseWindowTitle = val;
    emit baseWindowTitleChanged();
}

QFontDatabase &Application::fontDatabase()
{
    static QFontDatabase theGlobalFontDatabase;
    return theGlobalFontDatabase;
}

QJsonObject Application::systemFontInfo()
{
    HourGlass hourGlass;

    QFontDatabase &fontdb = Application::fontDatabase();

    static QJsonObject ret;

    static QMutex retLock;
    QMutexLocker locker(&retLock);

    if (ret.isEmpty()) {
        // Load all fonts, we will need it at some point anyway
        fontdb.families();

        const QStringList allFamilies = fontdb.families();
        QStringList families;
        std::copy_if(allFamilies.begin(), allFamilies.end(), std::back_inserter(families),
                     [fontdb](const QString &family) { return !fontdb.isPrivateFamily(family); });
        ret.insert("families", QJsonArray::fromStringList(families));

        QJsonArray sizes;
        const QList<int> stdSizes = fontdb.standardSizes();
        for (int stdSize : stdSizes)
            sizes.append(QJsonValue(stdSize));
        ret.insert("standardSizes", sizes);
    }

    return ret;
}

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
        param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
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
    if (fi.isFile()) {
        notification->setTitle(QStringLiteral("File available"));

#ifdef Q_OS_MAC
        notification->setText(QStringLiteral("Revealing <b>%1</b> in '<i>%2</i>'...")
                                      .arg(fi.fileName(), fi.absolutePath()));
#else
#ifdef Q_OS_WIN
        notification->setText(
                QStringLiteral("<b>%1</b> is available at '<i>%2</i>'. You can take a look at the "
                               "file by switching to the Explorer window which has been "
                               "opened in the background with this file selected.")
                        .arg(fi.fileName(), fi.absolutePath()));
#else
        notification->setText(
                QStringLiteral("<b>%1</b> is available. Please launch your file manager app "
                               "and navigate to '<i>%2</i>' to open the file.")
                        .arg(fi.fileName(), fi.absolutePath()));
#endif
#endif

    } else {
        notification->setTitle(QStringLiteral("Unable to open folder"));
        notification->setText(fi.absolutePath());
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
        return QtApplicationClass::notify(object, event);

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

    const bool ret = QtApplicationClass::notify(object, event);

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
            QtApplicationClass::notify(childObject, &parentChangeEvent);
        }
    }

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
        fontPointSize = 12;
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
    return QtApplicationClass::event(event);
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
#ifdef Q_OS_WIN
        QTimer::singleShot(100, window, &QWindow::showMaximized);
#else
        window->setGeometry(screenGeo);
#endif
        return true;
    }

    const QStringList geometry = geometryString.split(QStringLiteral(" "), Qt::SkipEmptyParts);
    if (geometry.length() != 4) {
        window->setGeometry(screenGeo);
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

    const int x = geometry.at(0).toInt() + geoDelta;
    const int y = geometry.at(1).toInt() + geoDelta;
    const int w = geometry.at(2).toInt() + geoDelta;
    const int h = geometry.at(3).toInt() + geoDelta;
    QRect geo(x, y, w, h);
    if (!screenGeo.contains(geo)) {
        if (w > screenGeo.width() || h > screenGeo.height()) {
            window->setGeometry(screenGeo);
            return false;
        }

        geo.moveCenter(screenGeo.center());
    }

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

    const QString sessionToken = LocalStorage::load("sessionToken").toString();
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
            window->showMaximized();
        else
            window->showNormal();
    } else {
        window->setProperty(propName, window->windowStates().testFlag(Qt::WindowMaximized));
        window->showFullScreen();
    }
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
