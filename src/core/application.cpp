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
#include "undoredo.h"
#include "hourglass.h"
#include "autoupdate.h"
#include "application.h"
#include "timeprofiler.h"
#include "notification.h"
#include "execlatertimer.h"
#include "scritedocument.h"
#include "jsonhttprequest.h"

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
#include <QQuickItem>
#include <QJsonArray>
#include <QQuickStyle>
#include <QMessageBox>
#include <QJsonObject>
#include <QColorDialog>
#include <QQuickWindow>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QOperatingSystemVersion>
#include <QNetworkConfigurationManager>

#define ENABLE_SCRIPT_HOTKEY

bool QtApplicationEventNotificationCallback(void **cbdata);

void ApplicationQtMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                 const QString &message)
{
#ifdef QT_NO_DEBUG_OUTPUT
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

    connect(m_undoGroup, &QUndoGroup::canUndoChanged, this, &Application::canUndoChanged);
    connect(m_undoGroup, &QUndoGroup::canRedoChanged, this, &Application::canRedoChanged);
    connect(m_undoGroup, &QUndoGroup::undoTextChanged, this, &Application::undoTextChanged);
    connect(m_undoGroup, &QUndoGroup::redoTextChanged, this, &Application::redoTextChanged);
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

    TransliterationEngine::instance(this);
    SystemTextInputManager::instance();

    m_networkConfiguration = new QNetworkConfigurationManager(this);
    m_networkConfiguration->allConfigurations(QNetworkConfiguration::Active);
    connect(m_networkConfiguration, &QNetworkConfigurationManager::onlineStateChanged, this,
            &Application::internetAvailableChanged);

    QtConcurrent::run(&Application::systemFontInfo);

    this->setWindowIcon(QIcon(QStringLiteral(":/images/appicon.png")));

    m_customFontPointSize = [=]() -> int {
        const QVariant val = m_settings->value(QLatin1String("Application/customFontPointSize"));
        if (val.isValid() && !val.isNull() && val.canConvert(QMetaType::Int))
            return val.toInt();
        return 0;
    }();
    this->computeIdealFontPointSize();

    const bool useSoftwareRenderer = [=]() -> bool {
#ifdef Q_OS_WIN
        if (QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10)
            return true;
#endif
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

    QQuickStyle::setStyle(style);
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
    const QVersionNumber applicationVersion = QVersionNumber::fromString(QStringLiteral("0.9.2.8"));
    const QString applicationVersionString = [applicationVersion]() -> QString {
        const QVector<int> segments = applicationVersion.segments();

        QStringList ret;

        for (int i = 0; i < qMin(segments.size(), 3); i++)
            ret << QString::number(segments.at(i));

        for (int i = ret.size(); i < 3; i++)
            ret << QStringLiteral("0");

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
    }

#ifdef Q_OS_MAC
    Application::setApplicationVersion(applicationVersionString + QStringLiteral("-beta"));
    if (QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSCatalina)
        qputenv("QT_MAC_WANTS_LAYER", QByteArrayLiteral("1"));
#else
    if (QSysInfo::WordSize == 32)
        Application::setApplicationVersion(applicationVersionString + "-beta-x86");
    else
        Application::setApplicationVersion(applicationVersionString + "-beta-x64");
#endif

#ifdef Q_OS_WIN
    Application::setAttribute(Qt::AA_UseDesktopOpenGL);

    // Maybe helps address https://www.github.com/teriflix/scrite/issues/247
    const QByteArray dpiMode = qgetenv("SCRITE_DPI_MODE").trimmed();
    if (dpiMode.isEmpty()) {
        const qreal uiScaleFactor =
                getWindowsEnvironmentVariable(QLatin1String("SCRITE_UI_SCALE_FACTOR"),
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

QUrl Application::toHttpUrl(const QUrl &url) const
{
    if (url.scheme() != QStringLiteral("https"))
        return url;

    QUrl url2 = url;
    url2.setScheme(QStringLiteral("http"));
    return url2;
}

#ifdef Q_OS_MAC
Application::Platform Application::platform() const
{
    return Application::MacOS;
}
#else
#ifdef Q_OS_WIN
Application::Platform Application::platform() const
{
    return Application::WindowsDesktop;
}

bool Application::isNotWindows10() const
{
    return QOperatingSystemVersion::current() < QOperatingSystemVersion::Windows10;
}
#else
Application::Platform Application::platform() const
{
    return Application::LinuxDesktop;
}
#endif
#endif

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

bool Application::usingMaterialTheme() const
{
    return QQuickStyle::name() == QStringLiteral("Material");
}

bool Application::isInternetAvailable() const
{
    return m_networkConfiguration != nullptr && m_networkConfiguration->isOnline();
}

QString Application::controlKey() const
{
    return this->platform() == Application::MacOS ? "⌘" : "Ctrl";
}

QString Application::altKey() const
{
    return this->platform() == Application::MacOS ? "⌥" : "Alt";
}

QString Application::polishShortcutTextForDisplay(const QString &text) const
{
    QString text2 = text.trimmed();
    text2.replace(QStringLiteral("Ctrl"), this->controlKey(), Qt::CaseInsensitive);
    text2.replace(QStringLiteral("Alt"), this->altKey(), Qt::CaseInsensitive);
    return text2;
}

void Application::setBaseWindowTitle(const QString &val)
{
    if (m_baseWindowTitle == val)
        return;

    m_baseWindowTitle = val;
    emit baseWindowTitleChanged();
}

QString Application::typeName(QObject *object) const
{
    if (object == nullptr)
        return QString();

    return QString::fromLatin1(object->metaObject()->className());
}

bool Application::verifyType(QObject *object, const QString &name) const
{
    return object && object->inherits(qPrintable(name));
}

bool Application::isTextInputItem(QQuickItem *item) const
{
    return item && item->flags() & QQuickItem::ItemAcceptsInputMethod;
}

UndoStack *Application::findUndoStack(const QString &objectName) const
{
    const QList<QUndoStack *> stacks = m_undoGroup->stacks();
    for (QUndoStack *stack : stacks) {
        if (stack->objectName() == objectName) {
            UndoStack *ret = qobject_cast<UndoStack *>(stack);
            return ret;
        }
    }

    return nullptr;
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

        const QStringList allFamilies = fontdb.families(QFontDatabase::Latin);
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

QColor Application::pickColor(const QColor &initial) const
{
    QColorDialog::ColorDialogOptions options =
            QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog;
    return QColorDialog::getColor(initial, nullptr, "Select Color", options);
}

QRectF Application::textBoundingRect(const QString &text, const QFont &font) const
{
    return QFontMetricsF(font).boundingRect(text);
}

void Application::revealFileOnDesktop(const QString &pathIn)
{
    m_errorReport->clear();

    // The implementation of this function is inspired from QtCreator's
    // implementation of FileUtils::showInGraphicalShell() method
    const QFileInfo fileInfo(pathIn);

    // Mac, Windows support folder or file.
    if (this->platform() == WindowsDesktop) {
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
    } else if (this->platform() == MacOS) {
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

QJsonArray enumerationModel(const QMetaObject *metaObject, const QString &enumName)
{
    QJsonArray ret;

    if (metaObject == nullptr || enumName.isEmpty())
        return ret;

    const int enumIndex = metaObject->indexOfEnumerator(qPrintable(enumName));
    if (enumIndex < 0)
        return ret;

    const QMetaEnum enumInfo = metaObject->enumerator(enumIndex);
    if (!enumInfo.isValid())
        return ret;

    auto queryEnumIcon = [=](const char *key) {
        const QByteArray cikey =
                QByteArrayLiteral("enum_") + QByteArray(key) + QByteArrayLiteral("_icon");
        const int ciIndex = metaObject->indexOfClassInfo(cikey.constData());
        if (ciIndex < 0)
            return QString();

        const QMetaClassInfo ci = metaObject->classInfo(ciIndex);
        return QString::fromLatin1(ci.value());
    };

    for (int i = 0; i < enumInfo.keyCount(); i++) {
        QJsonObject item;
        item.insert(QStringLiteral("key"), QString::fromLatin1(enumInfo.key(i)));
        item.insert(QStringLiteral("value"), enumInfo.value(i));

        const QString icon = queryEnumIcon(enumInfo.key(i));
        if (!icon.isEmpty())
            item.insert(QStringLiteral("icon"), icon);
        ret.append(item);
    }

    return ret;
}

QJsonArray Application::enumerationModel(QObject *object, const QString &enumName) const
{
    const QMetaObject *mo = object ? object->metaObject() : nullptr;
    return ::enumerationModel(mo, enumName);
}

QJsonArray Application::enumerationModelForType(const QString &typeName,
                                                const QString &enumName) const
{
    const int typeId = QMetaType::type(qPrintable(typeName + "*"));
    const QMetaObject *mo =
            typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    return ::enumerationModel(mo, enumName);
}

QString enumerationKey(const QMetaObject *metaObject, const QString &enumName, int value)
{
    QString ret;

    if (metaObject == nullptr || enumName.isEmpty())
        return ret;

    const int enumIndex = metaObject->indexOfEnumerator(qPrintable(enumName));
    if (enumIndex < 0)
        return ret;

    const QMetaEnum enumInfo = metaObject->enumerator(enumIndex);
    if (!enumInfo.isValid())
        return ret;

    return QString::fromLatin1(enumInfo.valueToKey(value));
}

QString Application::enumerationKey(QObject *object, const QString &enumName, int value) const
{
    return ::enumerationKey(object->metaObject(), enumName, value);
}

QString Application::enumerationKeyForType(const QString &typeName, const QString &enumName,
                                           int value) const
{
    const int typeId = QMetaType::type(qPrintable(typeName + "*"));
    const QMetaObject *mo =
            typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    return ::enumerationKey(mo, enumName, value);
}

QJsonObject Application::fileInfo(const QString &path) const
{
    QFileInfo fi(path);
    QJsonObject ret;
    ret.insert("exists", fi.exists());
    if (!fi.exists())
        return ret;

    ret.insert("baseName", fi.baseName());
    ret.insert("absoluteFilePath", fi.absoluteFilePath());
    ret.insert("absolutePath", fi.absolutePath());
    ret.insert("suffix", fi.suffix());
    ret.insert("fileName", fi.fileName());
    return ret;
}

QString Application::settingsFilePath() const
{
    return m_settings->fileName();
}

QPointF Application::cursorPosition() const
{
    return QCursor::pos();
}

QPointF Application::mapGlobalPositionToItem(QQuickItem *item, const QPointF &pos) const
{
    if (item == nullptr)
        return pos;

    return item->mapFromGlobal(pos);
}

bool Application::isMouseOverItem(QQuickItem *item) const
{
    if (item == nullptr)
        return false;

    const QPointF pos = this->mapGlobalPositionToItem(item, QCursor::pos());
    return item->boundingRect().contains(pos);
}

class ExecLater : public QObject
{
public:
    explicit ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &arg,
                       QObject *parent = nullptr);
    ~ExecLater();

    void timerEvent(QTimerEvent *event);

private:
    ExecLaterTimer m_timer;
    QJSValue m_function;
    QJSValueList m_arguments;
};

ExecLater::ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &args,
                     QObject *parent)
    : QObject(parent), m_timer("ExecLater.m_timer"), m_function(function), m_arguments(args)
{
    howMuchLater = qBound(0, howMuchLater, 60 * 60 * 1000);
    m_timer.start(howMuchLater, this);
}

ExecLater::~ExecLater()
{
    m_timer.stop();
}

void ExecLater::timerEvent(QTimerEvent *event)
{
    if (m_timer.timerId() == event->timerId()) {
        m_timer.stop();
        if (m_function.isCallable())
            m_function.call(m_arguments);
        GarbageCollector::instance()->add(this);
    }
}

void Application::execLater(QObject *context, int howMuchLater, const QJSValue &function,
                            const QJSValueList &args)
{
    QObject *parent = context ? context : this;

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Registering Exec Later for " << context << " after " << howMuchLater;
#endif

    new ExecLater(howMuchLater, function, args, parent);
}

QColor Application::translucent(const QColor &input, qreal alpha)
{
    QColor ret = input;
    ret.setAlphaF(qBound(0.0, ret.alphaF() * alpha, 1.0));
    return ret;
}

AutoUpdate *Application::autoUpdate() const
{
    return AutoUpdate::instance();
}

QJsonObject Application::objectConfigurationFormInfo(const QObject *object,
                                                     const QMetaObject *from) const
{
    QJsonObject ret;
    if (object == nullptr)
        return ret;

    if (from == nullptr)
        from = object->metaObject();

    const QMetaObject *mo = object->metaObject();
    auto queryClassInfo = [mo](const char *key) {
        const int ciIndex = mo->indexOfClassInfo(key);
        if (ciIndex < 0)
            return QString();
        const QMetaClassInfo ci = mo->classInfo(ciIndex);
        return QString::fromLatin1(ci.value());
    };

    auto queryPropertyInfo = [queryClassInfo](const QMetaProperty &prop, const char *key) {
        const QString ciKey = QString::fromLatin1(prop.name()) + "_" + QString::fromLatin1(key);
        return queryClassInfo(qPrintable(ciKey));
    };

    ret.insert("title", queryClassInfo("Title"));
    ret.insert("description", queryClassInfo("Description"));

    QJsonArray fields;
    QJsonArray groupedFields;

    auto addFieldToGroup = [&groupedFields, queryClassInfo](const QJsonObject &field) {
        const QString fieldGroup = field.value("group").toString();
        int index = -1;
        if (fieldGroup.isEmpty() && !groupedFields.isEmpty())
            index = 0;
        else {
            for (int i = 0; i < groupedFields.size(); i++) {
                QJsonObject groupInfo = groupedFields.at(i).toObject();
                if (groupInfo.value("name").toString() == fieldGroup) {
                    index = i;
                    break;
                }
            }
        }

        QJsonObject groupInfo;
        if (index < 0) {
            const QString descKey = fieldGroup + QStringLiteral("_Description");
            groupInfo.insert("name", fieldGroup);
            groupInfo.insert("description", queryClassInfo(qPrintable(descKey)));
        } else {
            groupInfo = groupedFields.at(index).toObject();
        }

        QJsonArray fields = groupInfo.value("fields").toArray();
        fields.append(field);
        groupInfo.insert("fields", fields);
        if (index < 0)
            groupedFields.append(groupInfo);
        else
            groupedFields.replace(index, groupInfo);
    };

    for (int i = from->propertyOffset(); i < mo->propertyCount(); i++) {
        const QMetaProperty prop = mo->property(i);
        if (!prop.isWritable() || !prop.isStored())
            continue;

        QJsonObject field;
        field.insert("name", QString::fromLatin1(prop.name()));
        field.insert("label", queryPropertyInfo(prop, "FieldLabel"));
        field.insert("note", queryPropertyInfo(prop, "FieldNote"));
        field.insert("editor", queryPropertyInfo(prop, "FieldEditor"));
        field.insert("min", queryPropertyInfo(prop, "FieldMinValue"));
        field.insert("max", queryPropertyInfo(prop, "FieldMaxValue"));
        field.insert("ideal", queryPropertyInfo(prop, "FieldDefaultValue"));
        field.insert("group", queryPropertyInfo(prop, "FieldGroup"));

        const QString fieldEnum = queryPropertyInfo(prop, "FieldEnum");
        if (!fieldEnum.isEmpty()) {
            const int enumIndex = mo->indexOfEnumerator(qPrintable(fieldEnum));
            const QMetaEnum enumerator = mo->enumerator(enumIndex);

            QJsonArray choices;
            for (int j = 0; j < enumerator.keyCount(); j++) {
                QJsonObject choice;
                choice.insert("key", QString::fromLatin1(enumerator.key(j)));
                choice.insert("value", enumerator.value(j));

                const QByteArray ciKey =
                        QByteArray(enumerator.name()) + "_" + QByteArray(enumerator.key(j));
                const QString text = queryClassInfo(ciKey);
                if (!text.isEmpty())
                    choice.insert("key", text);

                choices.append(choice);
            }

            field.insert("choices", choices);
        }

        fields.append(field);
        addFieldToGroup(field);
    }

    ret.insert("fields", fields);
    ret.insert("groupedFields", groupedFields);

    return ret;
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

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_M) {
            emit minimizeWindowRequest();
            return true;
        }

        if (ke->modifiers() == Qt::ControlModifier && ke->key() == Qt::Key_Z) {
            if (!UndoHandler::handleUndo())
                m_undoGroup->undo();
            return true;
        }

        if ((ke->modifiers() == Qt::ControlModifier && ke->key() == Qt::Key_Y)
#ifdef Q_OS_MAC
            || (ke->modifiers() & Qt::ControlModifier && ke->modifiers() & Qt::ShiftModifier
                && ke->key() == Qt::Key_Z)
#endif
        ) {
            if (!UndoHandler::handleRedo())
                m_undoGroup->redo();
            return true;
        }

        if (ke->modifiers() & Qt::ControlModifier && ke->modifiers() & Qt::ShiftModifier
            && ke->key() == Qt::Key_T) {
            if (this->loadScript())
                return true;
        }
    }

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

QString Application::painterPathToString(const QPainterPath &val)
{
    QByteArray ret;
    {
        QDataStream ds(&ret, QIODevice::WriteOnly);
        ds << val;
    }

    return QString::fromLatin1(ret.toHex());
}

QPainterPath Application::stringToPainterPath(const QString &val)
{
    const QByteArray bytes = QByteArray::fromHex(val.toLatin1());
    QDataStream ds(bytes);
    QPainterPath path;
    ds >> path;
    return path;
}

QJsonObject Application::replaceCharacterName(const QString &from, const QString &to,
                                              const QJsonObject &delta, int *nrReplacements)
{
    const QString opsAttr = QStringLiteral("ops");
    const QString insertAttr = QStringLiteral("insert");

    QJsonArray ops = delta.value(opsAttr).toArray();

    int totalCount = 0;

    for (int i = 0; i < ops.size(); i++) {
        QJsonValueRef item = ops[i];
        QJsonObject op = item.toObject();

        QJsonValue insert = op.value(insertAttr);
        if (insert.isString()) {
            int count = 0;
            insert = replaceCharacterName(from, to, insert.toString(), &count);
            if (count > 0) {
                totalCount += count;
                op.insert(insertAttr, insert);
                item = op;
            }
        }
    }

    if (totalCount > 0) {
        if (nrReplacements)
            *nrReplacements = totalCount;

        QJsonObject ret = delta;
        ret.insert(opsAttr, ops);
        return ret;
    }

    return delta;
}

QString Application::replaceCharacterName(const QString &from, const QString &to, const QString &in,
                                          int *nrReplacements)
{
    QString text = in.trimmed();
    QList<int> replacePositions;

    int pos = 0; // search from last
    while (pos < text.length()) {
        pos = text.indexOf(from, pos, Qt::CaseInsensitive);
        if (pos < 0)
            break;

        if (pos > 0) {
            const QChar ch = text.at(pos - 1);
            if (!ch.isPunct() && !ch.isSpace()) {
                pos += from.length();
                continue;
            }
        }

        bool found = false;
        if (pos + from.length() < text.length()) {
            const QChar ch = text.at(pos + from.length());
            found = ch.isPunct() || ch.isSpace();
        } else
            found = (text.compare(from, Qt::CaseInsensitive) == 0);

        if (found)
            replacePositions << pos;

        pos += from.length();
    }

    if (!replacePositions.isEmpty()) {
        if (nrReplacements)
            *nrReplacements = replacePositions.size();

        for (int i = replacePositions.size() - 1; i >= 0; i--) {
            const int pos = replacePositions.at(i);
            const bool allCaps = [](const QString &val) {
                return val == val.toUpper();
            }(text.mid(pos, from.length()));
            text = text.replace(pos, from.length(), allCaps ? to.toUpper() : to);
        }

        return text;
    }

    return in;
}

QString Application::sanitiseFileName(const QString &fileName)
{
    const QFileInfo fi(fileName);

    QString baseName = fi.baseName();
    bool changed = false;
    for (int i = baseName.length() - 1; i >= 0; i--) {
        const QChar ch = baseName.at(i);
        if (ch.isLetterOrNumber())
            continue;

        static const QList<QChar> allowedChars = {
            '-', '_', '[', ']', '(', ')', '{', '}', '&', ' '
        };
        if (allowedChars.contains(ch))
            continue;

        baseName = baseName.remove(i, 1);
        changed = true;
    }

    if (changed)
        return fi.absoluteDir().absoluteFilePath(baseName + QStringLiteral(".")
                                                 + fi.suffix().toLower());

    return fileName;
}

void Application::log(const QString &message)
{
    fprintf(stdout, "%s\n", qPrintable(message));
    fflush(stdout);
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

/**
 * I dont expect the object registry to have too many objects at this point.
 * So, I dont want to overly optimise the code here for being super fast.
 */
QString Application::registerObject(QObject *object, const QString &name)
{
    if (object == nullptr)
        return QString();

    QString objName = name;
    if (objName.isEmpty())
        objName = object->objectName();
    if (objName.isEmpty())
        objName = QString::fromLatin1(object->metaObject()->className());

    int index = 0;
    QString finalObjName = objName;
    while (this->findRegisteredObject(finalObjName))
        finalObjName = objName + QString::number(index++);

    // We are not using setObjectName on purpose!!
    object->setProperty("#objectName", finalObjName);
    m_objectRegistry.append(object);

    return finalObjName;
}

void Application::unregisterObject(QObject *object)
{
    m_objectRegistry.removeAt(m_objectRegistry.indexOf(object));
}

QObject *Application::findRegisteredObject(const QString &name) const
{
    const QList<QObject *> &objects = m_objectRegistry.list();
    for (QObject *object : objects) {
        const QString objName = object->property("#objectName").toString();
        if (objName == name)
            return object;
    }

    return nullptr;
}

QColor Application::pickStandardColor(int counter) const
{
    const QVector<QColor> colors = this->standardColors();
    if (colors.isEmpty())
        return QColor("white");

    QColor ret = colors.at(qMax(counter, 0) % colors.size());
    return ret;
}

inline qreal evaluateLuminance(const QColor &color)
{
    return ((0.299 * color.redF()) + (0.587 * color.greenF()) + (0.114 * color.blueF()));
}

bool Application::isLightColor(const QColor &color)
{
    return evaluateLuminance(color) > 0.5;
}

bool Application::isVeryLightColor(const QColor &color)
{
    return evaluateLuminance(color) > 0.8;
}

QColor Application::textColorFor(const QColor &bgColor)
{
    // https://stackoverflow.com/questions/1855884/determine-font-color-based-on-background-color/1855903#1855903
    return evaluateLuminance(bgColor) > 0.5 ? Qt::black : Qt::white;
}

QRectF Application::largestBoundingRect(const QStringList &strings, const QFont &font) const
{
    if (strings.isEmpty())
        return QRectF();

    const QFontMetricsF fm(font);

    auto evalTextRect = [fm](const QString &item) -> QRectF { return fm.boundingRect(item); };

    auto pickLargestRect = [](QRectF &intermediate, const QRectF &rect) {
        if (intermediate.isEmpty() || intermediate.width() < rect.width())
            intermediate = rect;
    };

    // Turns out that using QtConcurrent takes more time than using a simple for loop
    QRectF ret;
    for (const QString &item : strings)
        pickLargestRect(ret, evalTextRect(item));

    return ret;
}

QRectF Application::boundingRect(const QString &text, const QFont &font) const
{
    const QFontMetricsF fm(font);
    return fm.boundingRect(text);
}

QRectF Application::intersectedRectangle(const QRectF &of, const QRectF &with) const
{
    return of.intersected(with);
}

bool Application::doRectanglesIntersect(const QRectF &r1, const QRectF &r2) const
{
    return r1.intersects(r2);
}

QSizeF Application::scaledSize(const QSizeF &of, const QSizeF &into) const
{
    return of.scaled(into, Qt::KeepAspectRatio);
}

QRectF Application::uniteRectangles(const QRectF &r1, const QRectF &r2) const
{
    return r1.united(r2);
}

QRectF Application::adjustRectangle(const QRectF &rect, qreal left, qreal top, qreal right,
                                    qreal bottom) const
{
    return rect.adjusted(left, top, right, bottom);
}

bool Application::isRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect) const
{
    return bigRect.contains(smallRect);
}

QPointF Application::translationRequiredToBringRectangleInRectangle(const QRectF &bigRect,
                                                                    const QRectF &smallRect) const
{
    QPointF ret(0, 0);

    if (!bigRect.contains(smallRect)) {
        if (smallRect.left() < bigRect.left())
            ret.setX(bigRect.left() - smallRect.left());
        else if (smallRect.right() > bigRect.right())
            ret.setX(-(smallRect.right() - bigRect.right()));

        if (smallRect.top() < bigRect.top())
            ret.setY(bigRect.top() - smallRect.top());
        else if (smallRect.bottom() > bigRect.bottom())
            ret.setY(-(smallRect.bottom() - bigRect.bottom()));
    }

    return ret;
}

qreal Application::distanceBetweenPoints(const QPointF &p1, const QPointF &p2) const
{
    return QLineF(p1, p2).length();
}

QRectF Application::querySubRectangle(const QRectF &in, const QRectF &around,
                                      const QSizeF &atBest) const
{
    if (in.width() < atBest.width() || in.height() < atBest.height()) {
        QRectF ret(0, 0, atBest.width(), atBest.height());
        ret.moveCenter(around.center());
        return ret;
    }

    QRectF around2;
    if (atBest.width() > in.width() || atBest.height() > in.height())
        around2 =
                QRectF(0, 0, qMin(atBest.width(), in.width()), qMin(atBest.height(), in.height()));
    else
        around2 = QRectF(0, 0, atBest.width(), atBest.height());
    around2.moveCenter(around.center());

    const QSizeF aroundSize = around2.size();

    around2 = in.intersected(around2);
    if (qFuzzyCompare(around2.width(), aroundSize.width())
        && qFuzzyCompare(around2.height(), aroundSize.height()))
        return around2;

    around2.setSize(aroundSize);

    if (around2.left() < in.left())
        around2.moveLeft(in.left());
    else if (around2.right() > in.right())
        around2.moveRight(in.right());

    if (around2.top() < in.top())
        around2.moveTop(in.top());
    else if (around2.bottom() > in.bottom())
        around2.moveBottom(in.bottom());

    return around2;
}

QString Application::copyFile(const QString &fromFilePath, const QString &toFolder)
{
    const QFileInfo fromFileInfo(fromFilePath);
    if (fromFileInfo.isDir() || !fromFileInfo.isReadable() || fromFileInfo.isSymbolicLink())
        return QString();

    const QFileInfo toInfo(toFolder);
    QString toFilePath = toInfo.isDir() ? toInfo.dir().absoluteFilePath(fromFileInfo.fileName())
                                        : toInfo.absoluteFilePath();
    int counter = 1;
    while (1) {
        if (QFile::exists(toFilePath)) {
            const QFileInfo toFileInfo(toFilePath);
            toFilePath = toFileInfo.absoluteDir().absoluteFilePath(
                    toFileInfo.baseName() + QStringLiteral(" ") + QString::number(counter++)
                    + QStringLiteral(".") + toFileInfo.suffix());
        } else
            break;
    }

    toFilePath = Application::sanitiseFileName(toFilePath);

    const bool success = QFile::copy(fromFileInfo.absoluteFilePath(), toFilePath);
    if (success)
        return toFilePath;

    return QString();
}

bool Application::writeToFile(const QString &fileName, const QString &fileContent)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        file.write(fileContent.toLatin1());
        return true;
    }

    return false;
}

QString Application::fileContents(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QString();

    return QString::fromLatin1(file.readAll());
}

QString Application::fileName(const QString &path)
{
    return QFileInfo(path).baseName();
}

QString Application::filePath(const QString &fileName)
{
    return QFileInfo(fileName).absolutePath();
}

QString Application::neighbouringFilePath(const QString &filePath, const QString &nfileName)
{
    const QFileInfo fi(filePath);
    return fi.absoluteDir().absoluteFilePath(nfileName);
}

QScreen *Application::windowScreen(QObject *window) const
{
    QWindow *qwindow = qobject_cast<QWindow *>(window);
    if (qwindow)
        return qwindow->screen();

    QWidget *qwidget = qobject_cast<QWidget *>(window);
    if (qwidget)
        return qwidget->window()->windowHandle()->screen();

    return nullptr;
}

QString Application::getEnvironmentVariable(const QString &name)
{
    return QProcessEnvironment::systemEnvironment().value(name);
}

#ifdef Q_OS_WIN
static QString windowsEnvironmentRegistryGroup()
{
    static const QString ret = QLatin1String("HKEY_CURRENT_USER\\Environment\\");
    return ret;
}

QString Application::getWindowsEnvironmentVariable(const QString &name, const QString &defaultValue)
{
    QSettings settings(::windowsEnvironmentRegistryGroup(), QSettings::NativeFormat);
    if (settings.contains(name))
        return settings.value(name).toString();
    return defaultValue;
}

void Application::changeWindowsEnvironmentVariable(const QString &name, const QString &value)
{
    QSettings settings(::windowsEnvironmentRegistryGroup(), QSettings::NativeFormat);
    settings.setValue(name, value);

    QProcessEnvironment::systemEnvironment().insert(name, value);
}

void Application::removeWindowsEnvironmentVariable(const QString &name)
{
    QSettings settings(::windowsEnvironmentRegistryGroup(), QSettings::NativeFormat);
    settings.remove(name);

    QProcessEnvironment::systemEnvironment().remove(name);
}
#else
QString Application::getWindowsEnvironmentVariable(const QString &name, const QString &defaultValue)
{
    Q_UNUSED(name)

    return defaultValue;
}
void Application::changeWindowsEnvironmentVariable(const QString &name, const QString &value)
{
    Q_UNUSED(name)
    Q_UNUSED(value)
}

void Application::removeWindowsEnvironmentVariable(const QString &name)
{
    Q_UNUSED(name)
}
#endif

QPointF Application::globalMousePosition() const
{
    return QCursor::pos();
}

QString Application::camelCased(const QString &val)
{
    //    if(TransliterationEngine::instance()->language() != TransliterationEngine::English)
    //        return val;

    QString val2 = val.toLower();
    if (val2.isEmpty())
        return val;

    bool capitalize = true;
    for (int i = 0; i < val2.length(); i++) {
        QCharRef ch = val2[i];
        if (ch.isLetter() && ch.script() != QChar::Script_Latin)
            return val;

        if (capitalize) {
            if (ch.isLetter() && ch.script() == QChar::Script_Latin) {
                ch = ch.toUpper();
                capitalize = false;
            }
        } else {
            const QList<QChar> exclude = QList<QChar>() << QChar('\'');
            capitalize = !ch.isLetter() && !exclude.contains(ch);
        }
    }

    return val2;
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
        ExecLaterTimer::call(
                "window.showMaximized", window, [=]() { window->showMaximized(); }, 100);
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
    this->launchNewInstanceAndOpenAnonymously(window, QString());
}

void Application::launchNewInstanceAndOpenAnonymously(QWindow *window, const QString &filePath)
{
    const QString appPath = this->applicationFilePath();

    QStringList args;
    if (!filePath.isEmpty() && QFile::exists(filePath))
        args = QStringList({ QStringLiteral("--openAnonymously"), filePath });

    if (window != nullptr) {
        const QRect geometry = window->geometry();
        args += { QStringLiteral("--windowGeometry"), QString::number(geometry.x() + 30),
                  QString::number(geometry.y() + 30), QString::number(geometry.width()),
                  QString::number(geometry.height()) };
    } else
        args += { QStringLiteral("--geodelta"), QStringLiteral("30") };

    if (!JsonHttpRequest::sessionToken().isEmpty())
        args += { QStringLiteral("--sessionToken"), JsonHttpRequest::sessionToken() };

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

bool Application::resetObjectProperty(QObject *object, const QString &propName)
{
    if (object == nullptr || propName.isEmpty())
        return false;

    const QMetaObject *mo = object->metaObject();
    const int propIndex = mo->indexOfProperty(qPrintable(propName));
    if (propIndex < 0)
        return false;

    const QMetaProperty prop = mo->property(propIndex);
    if (!prop.isResettable())
        return false;

    return prop.reset(object);
}

int Application::objectTreeSize(QObject *ptr) const
{
    return ptr->findChildren<QObject *>(QString(), Qt::FindChildrenRecursively).size() + 1;
}

QString Application::createUniqueId()
{
    return QUuid::createUuid().toString();
}

void Application::sleep(int ms)
{
    ms = qBound(0, ms, 2000);

    QEventLoop eventLoop;
    if (ms == 0) {
        eventLoop.processEvents(QEventLoop::ExcludeUserInputEvents);
        return;
    }

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < ms)
        eventLoop.processEvents(QEventLoop::ExcludeUserInputEvents);
}

QTime Application::secondsToTime(int seconds)
{
    if (seconds == 0)
        return QTime(0, 0, 0);
    const int s = seconds > 60 ? seconds % 60 : seconds;
    const int tm = seconds > 60 ? (seconds - s) / 60 : 0;
    const int m = tm > 60 ? tm % 60 : tm;
    const int h = seconds > 3600 ? (seconds - m * 60 - s) / (60 * 60) : 0;
    return QTime(h, m, s);
}

QString Application::relativeTime(const QDateTime &dt)
{
    if (!dt.isValid())
        return QStringLiteral("Unknown Time");

    const QDateTime now = QDateTime::currentDateTime();
    if (now.date() == dt.date()) {
        const int secsInMin = 60;
        const int secsInHour = secsInMin * 60;

        // Just say how many minutes or hours ago.
        const int nrSecs = dt.time().secsTo(now.time());
        const int nrHours = nrSecs > secsInHour ? qFloor(qreal(nrSecs) / qreal(secsInHour)) : 0;
        const int nrSecsRemaining = nrSecs - nrHours * secsInHour;
        const int nrMins =
                nrSecs > secsInMin ? qCeil(qreal(nrSecsRemaining) / qreal(secsInMin)) : 0;

        if (nrMins == 0)
            return QStringLiteral("Less than a minute ago");
        if (nrHours == 0)
            return QString::number(qCeil(qreal(nrSecs) / qreal(secsInMin)))
                    + QStringLiteral("m ago");

        return QString::number(nrHours) + QStringLiteral("h ") + QString::number(nrMins)
                + QStringLiteral("m ago");
    }

    const int nrDays = dt.date().daysTo(now.date());
    const QString time = dt.time().toString(QStringLiteral("h:mm A"));
    switch (nrDays) {
    case 1:
        return QStringLiteral("Yesterday @ ") + time;
    case 2:
        return QStringLiteral("Day before yesterday @ ") + time;
    case 3:
    case 4:
    case 5:
    case 6:
        return QString::number(nrDays) + QStringLiteral(" days ago @ ") + time;
    default:
        break;
    }

    if (nrDays >= 7 && nrDays < 14)
        return QStringLiteral("Last week ")
                + QLocale::system().standaloneDayName(dt.date().dayOfWeek()) + " @ " + time;

    if (nrDays >= 14 && nrDays < 21)
        return QStringLiteral("Two weeks ago");

    if (nrDays >= 21 && nrDays < 28)
        return QStringLiteral("Three weeks ago");

    if (nrDays >= 28 && nrDays < 60)
        return QStringLiteral("Little more than a month ago");

    return QStringLiteral("More than two months ago");
}

Forms *Application::forms() const
{
    return Forms::global();
}

void Application::initializeStandardColors(QQmlEngine *)
{
    if (!m_standardColors.isEmpty())
        return;

    const QVector<QColor> colors = this->standardColors();
    for (int i = 0; i < colors.size(); i++)
        m_standardColors << QVariant::fromValue<QColor>(colors.at(i));

    emit standardColorsChanged();
}

QVector<QColor> Application::standardColors(const QVersionNumber &version)
{
    // Up-until version 0.2.17 Beta
    if (!version.isNull() && version <= QVersionNumber(0, 2, 17))
        return QVector<QColor>() << QColor("blue") << QColor("magenta") << QColor("darkgreen")
                                 << QColor("purple") << QColor("yellow") << QColor("orange")
                                 << QColor("red") << QColor("brown") << QColor("gray")
                                 << QColor("white");

    // New set of colors
    return QVector<QColor>() << QColor("#2196f3") << QColor("#e91e63") << QColor("#009688")
                             << QColor("#9c27b0") << QColor("#ffeb3b") << QColor("#ff9800")
                             << QColor("#f44336") << QColor("#795548") << QColor("#9e9e9e")
                             << QColor("#fafafa") << QColor("#3f51b5") << QColor("#cddc39");
}

#ifdef ENABLE_SCRIPT_HOTKEY

#include <QJSEngine>
#include <QFileDialog>
#include "scritedocument.h"

bool Application::loadScript()
{
    QMessageBox::StandardButton answer = QMessageBox::question(
            nullptr, QStringLiteral("Warning"),
            QStringLiteral("Executing scripts on a scrite project is an experimental feature. Are "
                           "you sure you want to use it?"),
            QMessageBox::Yes | QMessageBox::No);
    if (answer == QMessageBox::No)
        return true;

    ScriteDocument *document = ScriteDocument::instance();
    if (document->isReadOnly()) {
        QMessageBox::information(nullptr, QStringLiteral("Warning"),
                                 QStringLiteral("Cannot execute script on a readonly document."));
        return false;
    }

    QString scriptPath = QDir::homePath();
    if (!document->fileName().isEmpty()) {
        QFileInfo fi(document->fileName());
        scriptPath = fi.absolutePath();
    }

    const QString caption = QStringLiteral("Select a JavaScript file to load");
    const QString filter = QStringLiteral("JavaScript File (*.js)");
    const QString scriptFile = QFileDialog::getOpenFileName(nullptr, caption, scriptPath, filter);
    if (scriptFile.isEmpty())
        return true;

    auto loadProgram = [](const QString &fileName) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly))
            return QString();
        return QString::fromLatin1(file.readAll());
    };
    const QString program = loadProgram(scriptFile);
    if (program.isEmpty()) {
        QMessageBox::information(nullptr, QStringLiteral("Script"),
                                 QStringLiteral("No code was found in the selected file."));
        return true;
    }

    QJSEngine jsEngine;

    QJSValue globalObject = jsEngine.globalObject();
    if (!document->isReadOnly())
        globalObject.setProperty(QStringLiteral("document"), jsEngine.newQObject(document));

    const QList<QObject *> objects = m_objectRegistry.list();
    for (QObject *object : qAsConst(objects)) {
        const QString objName = object->property("#objectName").toString();
        globalObject.setProperty(objName, jsEngine.newQObject(object));
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    const QJSValue result = jsEngine.evaluate(program, scriptFile);
    qApp->restoreOverrideCursor();

    if (result.isError()) {
        const QString msg = QStringLiteral("Uncaught exception at line ")
                + result.property("lineNumber").toString() + ": " + result.toString();
        QMessageBox::warning(nullptr, QStringLiteral("Script"), msg);
    }

    return true;
}
#else
bool Application::loadScript()
{
    return false;
}
#endif

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
