/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "undoredo.h"
#include "hourglass.h"
#include "autoupdate.h"
#include "application.h"
#include "execlatertimer.h"
#include "scritedocument.h"

#include <QDir>
#include <QUuid>
#include <QtMath>
#include <QScreen>
#include <QtDebug>
#include <QCursor>
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
#include <QMessageBox>
#include <QJsonObject>
#include <QColorDialog>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QOperatingSystemVersion>
#include <QScreen>

#define ENABLE_SCRIPT_HOTKEY

bool QtApplicationEventNotificationCallback(void **cbdata);

Application *Application::instance()
{
    return qobject_cast<Application*>(qApp);
}

Application::Application(int &argc, char **argv, const QVersionNumber &version)
    : QtApplicationClass(argc, argv),
      m_versionNumber(version)
{
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Raleway/Raleway-BoldItalic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Raleway/Raleway-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Raleway/Raleway-Italic.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":font/Raleway/Raleway-Bold.ttf"));
    this->setFont( QFont("Raleway") );

    connect(m_undoGroup, &QUndoGroup::canUndoChanged, this, &Application::canUndoChanged);
    connect(m_undoGroup, &QUndoGroup::canRedoChanged, this, &Application::canRedoChanged);
    connect(m_undoGroup, &QUndoGroup::undoTextChanged, this, &Application::undoTextChanged);
    connect(m_undoGroup, &QUndoGroup::redoTextChanged, this, &Application::redoTextChanged);
    connect(this, &QGuiApplication::fontChanged, this, &Application::applicationFontChanged);

    this->setWindowIcon( QIcon(":/images/appicon.png") );
    this->setBaseWindowTitle(Application::applicationName() + " " + Application::applicationVersion());

    const QString settingsFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("settings.ini");
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
    this->installationId();
    this->installationTimestamp();
    m_settings->setValue( QStringLiteral("Installation/launchCount"), this->launchCounter()+1);

    if( m_settings->value( QStringLiteral("Installation/fileTypeRegistered") , false).toBool() == false )
    {
        const bool rft = this->registerFileTypes();
        m_settings->setValue( QStringLiteral("Installation/fileTypeRegistered"), rft );
        if(rft)
            m_settings->setValue( QStringLiteral("Installation/path"), this->applicationFilePath() );
    }

#ifndef QT_NO_DEBUG
    QInternal::registerCallback(QInternal::EventNotifyCallback, QtApplicationEventNotificationCallback);
#endif

    const QVersionNumber sversion = QVersionNumber::fromString( m_settings->value( QStringLiteral("Installation/version") ).toString() );
    if(sversion.isNull() || sversion == QVersionNumber(0,4,7))
    {
        // until we can fix https://github.com/teriflix/scrite/issues/138
        m_settings->setValue("Screenplay Editor/enableSpellCheck", false);
    }
    m_settings->setValue( QStringLiteral("Installation/version"), m_versionNumber.toString() );

    TransliterationEngine::instance(this);
    SystemTextInputManager::instance();
}

Application::~Application()
{
#ifndef QT_NO_DEBUG
    QInternal::unregisterCallback(QInternal::EventNotifyCallback, QtApplicationEventNotificationCallback);
#endif
}

QString Application::installationId() const
{
    QString clientID = m_settings->value("Installation/ClientID").toString();
    if(clientID.isEmpty())
    {
        clientID = QUuid::createUuid().toString();
        m_settings->setValue("Installation/ClientID", clientID);
    }

    return clientID;
}

QDateTime Application::installationTimestamp() const
{
    QString installTimestampStr = m_settings->value("Installation/timestamp").toString();
    QDateTime installTimestamp = QDateTime::fromString(installTimestampStr);
    if(installTimestampStr.isEmpty() || !installTimestamp.isValid())
    {
        installTimestamp = QDateTime::currentDateTime();
        installTimestampStr = installTimestamp.toString();
        m_settings->setValue("Installation/timestamp", installTimestampStr);
    }

    return installTimestamp;
}

int Application::launchCounter() const
{
    return m_settings->value("Installation/launchCount", 0).toInt();
}

QString Application::buildTimestamp() const
{
    return QString::fromLatin1(__TIMESTAMP__);
}

QUrl Application::toHttpUrl(const QUrl &url) const
{
    if(url.scheme() != QStringLiteral("https"))
        return url;

    QUrl url2 = url;
    url2.setScheme( QStringLiteral("http") );
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
    if(m_baseWindowTitle == val)
        return;

    m_baseWindowTitle = val;
    emit baseWindowTitleChanged();
}

QString Application::typeName(QObject *object) const
{
    if(object == nullptr)
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
    const QList<QUndoStack*> stacks = m_undoGroup->stacks();
    Q_FOREACH(QUndoStack *stack, stacks)
    {
        if(stack->objectName() == objectName)
        {
            UndoStack *ret = qobject_cast<UndoStack*>(stack);
            return ret;
        }
    }

    return nullptr;
}

QJsonObject Application::systemFontInfo() const
{
    HourGlass hourGlass;

    QFontDatabase fontdb;

    static QJsonObject ret;
    if(ret.isEmpty())
    {
        const QStringList allFamilies = fontdb.families( QFontDatabase::Latin );
        QStringList families;
        std::copy_if (allFamilies.begin(), allFamilies.end(),
                      std::back_inserter(families), [fontdb](const QString &family) {
            return !fontdb.isPrivateFamily(family);
        });
        ret.insert("families", QJsonArray::fromStringList(families));

        QJsonArray sizes;
        QList<int> stdSizes = fontdb.standardSizes();
        Q_FOREACH(int stdSize, stdSizes)
            sizes.append( QJsonValue(stdSize) );
        ret.insert("standardSizes", sizes);
    }

    return ret;
}

QColor Application::pickColor(const QColor &initial) const
{
    QColorDialog::ColorDialogOptions options =
            QColorDialog::ShowAlphaChannel|QColorDialog::DontUseNativeDialog;
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
    if (this->platform() == WindowsDesktop)
    {
        const QString explorer = QStandardPaths::findExecutable("explorer.exe");
        if (explorer.isEmpty())
        {
            m_errorReport->setErrorMessage("Could not find explorer.exe in path to launch Windows Explorer.");
            return;
        }

        QStringList param;
        if (!fileInfo.isDir())
            param += QLatin1String("/select,");
        param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
        QProcess::startDetached(explorer, param);
    }
    else if (this->platform() == MacOS)
    {
        QStringList scriptArgs;
        scriptArgs << QLatin1String("-e")
                   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                         .arg(fileInfo.canonicalFilePath());
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
        scriptArgs.clear();
        scriptArgs << QLatin1String("-e")
                   << QLatin1String("tell application \"Finder\" to activate");
        QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    }
    else
    {
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
}

QJsonArray enumerationModel(const QMetaObject *metaObject, const QString &enumName)
{
    QJsonArray ret;

    if( metaObject == nullptr || enumName.isEmpty() )
        return ret;

    const int enumIndex = metaObject->indexOfEnumerator( qPrintable(enumName) );
    if( enumIndex < 0 )
        return ret;

    const QMetaEnum enumInfo = metaObject->enumerator(enumIndex);
    if( !enumInfo.isValid() )
        return ret;

    for(int i=0; i<enumInfo.keyCount(); i++)
    {
        QJsonObject item;
        item.insert("key", QString::fromLatin1(enumInfo.key(i)));
        item.insert("value", enumInfo.value(i));
        ret.append(item);
    }

    return ret;
}

QJsonArray Application::enumerationModel(QObject *object, const QString &enumName) const
{
    const QMetaObject *mo = object ? object->metaObject() : nullptr;
    return ::enumerationModel(mo, enumName);
}

QJsonArray Application::enumerationModelForType(const QString &typeName, const QString &enumName) const
{
    const int typeId = QMetaType::type(qPrintable(typeName+"*"));
    const QMetaObject *mo = typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    return ::enumerationModel(mo, enumName);
}

QString enumerationKey(const QMetaObject *metaObject, const QString &enumName, int value)
{
    QString ret;

    if( metaObject == nullptr || enumName.isEmpty() )
        return ret;

    const int enumIndex = metaObject->indexOfEnumerator( qPrintable(enumName) );
    if( enumIndex < 0 )
        return ret;

    const QMetaEnum enumInfo = metaObject->enumerator(enumIndex);
    if( !enumInfo.isValid() )
        return ret;

    return QString::fromLatin1( enumInfo.valueToKey(value) );
}

QString Application::enumerationKey(QObject *object, const QString &enumName, int value) const
{
    return ::enumerationKey(object->metaObject(), enumName, value);
}

QString Application::enumerationKeyForType(const QString &typeName, const QString &enumName, int value) const
{
    const int typeId = QMetaType::type(qPrintable(typeName+"*"));
    const QMetaObject *mo = typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    return ::enumerationKey(mo, enumName, value);
}

QJsonObject Application::fileInfo(const QString &path) const
{
    QFileInfo fi(path);
    QJsonObject ret;
    ret.insert("exists", fi.exists());
    if(!fi.exists())
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
    if(item == nullptr)
        return pos;

    return item->mapFromGlobal(pos);
}

class ExecLater : public QObject
{
public:
    ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &arg, QObject *parent=nullptr);
    ~ExecLater();

    void timerEvent(QTimerEvent *event);

private:
    ExecLaterTimer m_timer;
    QJSValue m_function;
    QJSValueList m_arguments;
};

ExecLater::ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &args, QObject *parent)
    : QObject(parent), m_timer("ExecLater.m_timer"), m_function(function), m_arguments(args)
{
    howMuchLater = qBound(0, howMuchLater, 60*60*1000);
    m_timer.start(howMuchLater, this);
}

ExecLater::~ExecLater()
{
    m_timer.stop();
}

void ExecLater::timerEvent(QTimerEvent *event)
{
    if(m_timer.timerId() == event->timerId())
    {
        m_timer.stop();
        if(m_function.isCallable())
            m_function.call(m_arguments);
        GarbageCollector::instance()->add(this);
    }
}

void Application::execLater(QObject *context, int howMuchLater, const QJSValue &function, const QJSValueList &args)
{
    QObject *parent = context ? context : this;

#ifndef QT_NO_DEBUG
    qDebug() << "Registering Exec Later for " << context << " after " << howMuchLater;
#endif

    new ExecLater(howMuchLater, function, args, parent);
}

QColor Application::translucent(const QColor &input, qreal alpha) const
{
    QColor ret = input;
    ret.setAlphaF(qBound(0.0, ret.alphaF() * alpha, 1.0));
    return ret;
}

AutoUpdate *Application::autoUpdate() const
{
    return AutoUpdate::instance();
}

QJsonObject Application::objectConfigurationFormInfo(const QObject *object, const QMetaObject *from=nullptr) const
{
    QJsonObject ret;
    if(object == nullptr)
            return ret;

    if(from == nullptr)
        from = object->metaObject();

    const QMetaObject *mo = object->metaObject();
    auto queryClassInfo = [mo](const char *key) {
        const int ciIndex = mo->indexOfClassInfo(key);
        if(ciIndex < 0)
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

    auto addFieldToGroup = [&groupedFields,queryClassInfo](const QJsonObject &field) {
        const QString fieldGroup = field.value("group").toString();
        int index = -1;
        if(fieldGroup.isEmpty() && !groupedFields.isEmpty())
            index = 0;
        else {
            for(int i=0; i<groupedFields.size(); i++) {
                QJsonObject groupInfo = groupedFields.at(i).toObject();
                if(groupInfo.value("name").toString() == fieldGroup) {
                    index = i;
                    break;
                }
            }
        }

        QJsonObject groupInfo;
        if(index < 0) {
            const QString descKey = fieldGroup + QStringLiteral("_Description");
            groupInfo.insert("name", fieldGroup);
            groupInfo.insert("description", queryClassInfo(qPrintable(descKey)));
        } else {
            groupInfo = groupedFields.at(index).toObject();
        }

        QJsonArray fields = groupInfo.value("fields").toArray();
        fields.append(field);
        groupInfo.insert("fields", fields);
        if(index < 0)
            groupedFields.append(groupInfo);
        else
            groupedFields.replace(index, groupInfo);
    };

    for(int i=from->propertyOffset(); i<mo->propertyCount(); i++)
    {
        const QMetaProperty prop = mo->property(i);
        if(!prop.isWritable() || !prop.isStored())
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
        if( !fieldEnum.isEmpty() )
        {
            const int enumIndex = mo->indexOfEnumerator(qPrintable(fieldEnum));
            const QMetaEnum enumerator = mo->enumerator(enumIndex);

            QJsonArray choices;
            for(int j=0; j<enumerator.keyCount(); j++)
            {
                QJsonObject choice;
                choice.insert("key", QString::fromLatin1(enumerator.key(j)));
                choice.insert("value", enumerator.value(j));

                const QByteArray ciKey = QByteArray(enumerator.name()) + "_" + QByteArray(enumerator.key(j));
                const QString text = queryClassInfo(ciKey);
                if(!text.isEmpty())
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
#ifndef QT_NO_DEBUG
    QObject *object = reinterpret_cast<QObject*>(cbdata[0]);
    QEvent *event = reinterpret_cast<QEvent*>(cbdata[1]);
    bool *result = reinterpret_cast<bool*>(cbdata[2]);

    const bool ret = Application::instance()->notifyInternal(object, event);

    if(result)
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
    if(event->type() == QEvent::DeferredDelete)
        return QtApplicationClass::notify(object, event);

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if(ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_M)
        {
            emit minimizeWindowRequest();
            return true;
        }

        if(ke->modifiers() == Qt::ControlModifier && ke->key() == Qt::Key_Z)
        {
            m_undoGroup->undo();
            return true;
        }

        if( (ke->modifiers() == Qt::ControlModifier && ke->key() == Qt::Key_Y)
#ifdef Q_OS_MAC
           || (ke->modifiers()&Qt::ControlModifier && ke->modifiers()&Qt::ShiftModifier && ke->key() == Qt::Key_Z)
#endif
                )
        {
            m_undoGroup->redo();
            return true;
        }

        if( ke->modifiers()&Qt::ControlModifier && ke->modifiers()&Qt::ShiftModifier && ke->key() == Qt::Key_T )
        {
            if(this->loadScript())
                return true;
        }
    }

    const bool ret = QtApplicationClass::notify(object, event);

    // The only reason we reimplement the notify() method is because we sometimes want to
    // handle an event AFTER it is handled by the target object.

    if(event->type() == QEvent::ChildAdded)
    {
        QChildEvent *childEvent = reinterpret_cast<QChildEvent*>(event);
        QObject *childObject = childEvent->child();

        if(!childObject->isWidgetType() && !childObject->isWindowType())
        {
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
#ifndef QT_NO_DEBUG
    static QMap<QObject*,QString> objectNameMap;
    auto evaluateObjectName = [](QObject *object, QMap<QObject*,QString> &from) {
        QString objectName = from.value(object);
        if(objectName.isEmpty()) {
            QQuickItem *item = qobject_cast<QQuickItem*>(object);
            QObject* parent = item && item->parentItem() ? item->parentItem() : object->parent();
            QString parentName = parent ? from.value(parent) : "No Parent";
            if(parentName.isEmpty()) {
                parentName = QString("%1 [%2] (%3)")
                                    .arg(parent->metaObject()->className())
                                    .arg((unsigned long)((void*)parent),0,16)
                                    .arg(parent->objectName());
            }
            objectName = QString("%1 [%2] (%3) under %4")
                    .arg(object->metaObject()->className())
                    .arg((unsigned long)((void*)object),0,16)
                    .arg(object->objectName())
                    .arg(parentName);
            from[object] = objectName;
        }
        return objectName;
    };

    if(event->type() == QEvent::DeferredDelete)
    {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        qDebug() << "DeferredDelete: " << objectName;
    }
    else if(event->type() == QEvent::Timer)
    {
        const QString objectName = evaluateObjectName(object, objectNameMap);
        QTimerEvent *te = static_cast<QTimerEvent*>(event);
        ExecLaterTimer *timer = ExecLaterTimer::get(te->timerId());
        qDebug() << "TimerEventDespatch: " << te->timerId() << " on " << objectName << " is " << (timer ? qPrintable(timer->name()) : "Qt Timer.");
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
#ifndef Q_OS_MAC
    m_idealFontPointSize = 12;
#else
    const qreal minInch = 0.12; // Font should occupy atleast 0.12 inches on the screen
    const qreal nrPointsPerInch = qt_defaultDpi(); // These many dots make up one inch on the screen
    const qreal scale = this->primaryScreen()->physicalDotsPerInch() / nrPointsPerInch;
    const qreal dpr = this->primaryScreen()->devicePixelRatio();
    m_idealFontPointSize = qCeil(minInch * nrPointsPerInch * qMax(dpr,scale));
#endif
}

QString Application::painterPathToString(const QPainterPath &val) const
{
    QByteArray ret;
    {
        QDataStream ds(&ret, QIODevice::WriteOnly);
        ds << val;
    }

    return QString::fromLatin1(ret.toHex());
}

QPainterPath Application::stringToPainterPath(const QString &val) const
{
    const QByteArray bytes = QByteArray::fromHex(val.toLatin1());
    QDataStream ds(bytes);
    QPainterPath path;
    ds >> path;
    return path;
}

QString Application::sanitiseFileName(const QString &fileName) const
{
    const QFileInfo fi(fileName);

    QString baseName = fi.baseName();
    bool changed = false;
    for(int i=baseName.length()-1; i>=0; i--)
    {
        const QChar ch = baseName.at(i);
        if(ch.isLetterOrNumber())
            continue;

        static const QList<QChar> allowedChars = QList<QChar>() << QChar('_') << QChar('-') << QChar(' ');
        if(allowedChars.contains(ch))
            continue;

        baseName = baseName.remove(i, 1);
        changed = true;
    }

    if(changed)
        return fi.absoluteDir().absoluteFilePath( baseName + QStringLiteral(".") + fi.suffix().toLower() );

    return fileName;
}

bool Application::event(QEvent *event)
{
#ifdef Q_OS_MAC
    if(event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        if(m_handleFileOpenEvents)
            emit openFileRequest(openEvent->file());
        else
            m_fileToOpen = openEvent->file();
        return true;
    }
#endif
    return QtApplicationClass::event(event);
}

QColor Application::pickStandardColor(int counter) const
{
    const QVector<QColor> colors = this->standardColors();
    if(colors.isEmpty())
        return QColor("white");

    QColor ret = colors.at( counter%colors.size() );
    return ret;
}

QColor Application::textColorFor(const QColor &bgColor) const
{
    // https://stackoverflow.com/questions/1855884/determine-font-color-based-on-background-color/1855903#1855903
    const qreal luma = ((0.299 * bgColor.redF()) + (0.587 * bgColor.greenF()) + (0.114 * bgColor.blueF()));
    return luma > 0.5 ? Qt::black : Qt::white;
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

QRectF Application::adjustRectangle(const QRectF &rect, qreal left, qreal top, qreal right, qreal bottom) const
{
    return rect.adjusted(left, top, right, bottom);
}

bool Application::isRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect) const
{
    return bigRect.contains(smallRect);
}

QPointF Application::translationRequiredToBringRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect) const
{
    QPointF ret(0, 0);

    if(!bigRect.contains(smallRect))
    {
        if(smallRect.left() < bigRect.left())
            ret.setX( bigRect.left()-smallRect.left() );
        else if(smallRect.right() > bigRect.right())
            ret.setX( -(smallRect.right()-bigRect.right()) );

        if(smallRect.top() < bigRect.top())
            ret.setY( bigRect.top()-smallRect.top() );
        else if(smallRect.bottom() > bigRect.bottom())
            ret.setY( -(smallRect.bottom()-bigRect.bottom()) );
    }

    return ret;
}

qreal Application::distanceBetweenPoints(const QPointF &p1, const QPointF &p2) const
{
    return QLineF(p1, p2).length();
}

QRectF Application::querySubRectangle(const QRectF &in, const QRectF &around, const QSizeF &atBest) const
{
    if( in.width() < atBest.width() || in.height() < atBest.height() )
    {
        QRectF ret(0, 0, atBest.width(), atBest.height());
        ret.moveCenter(around.center());
        return ret;
    }

    QRectF around2;
    if(atBest.width() > in.width() || atBest.height() > in.height())
        around2 = QRectF(0, 0, qMin(atBest.width(), in.width()), qMin(atBest.height(), in.height()));
    else
        around2 = QRectF(0, 0, atBest.width(), atBest.height());
    around2.moveCenter(around.center());

    const QSizeF aroundSize = around2.size();

    around2 = in.intersected(around2);
    if(around2.width() == aroundSize.width() && around2.height() == aroundSize.height())
        return around2;

    around2.setSize(aroundSize);

    if(around2.left() < in.left())
        around2.moveLeft(in.left());
    else if(around2.right() > in.right())
        around2.moveRight(in.right());

    if(around2.top() < in.top())
        around2.moveTop(in.top());
    else if(around2.bottom() > in.bottom())
        around2.moveBottom(in.bottom());

    return around2;
}

QString Application::fileContents(const QString &fileName) const
{
    QFile file(fileName);
    if( !file.open(QFile::ReadOnly) )
        return QString();

    return QString::fromLatin1(file.readAll());
}

QString Application::fileName(const QString &path) const
{
    return QFileInfo(path).baseName();
}

QScreen *Application::windowScreen(QObject *window) const
{
    QWindow *qwindow = qobject_cast<QWindow*>(window);
    if(qwindow)
        return qwindow->screen();

    QWidget *qwidget = qobject_cast<QWidget*>(window);
    if(qwidget)
        return qwidget->window()->windowHandle()->screen();

    return nullptr;
}

QString Application::getEnvironmentVariable(const QString &name) const
{
    return QProcessEnvironment::systemEnvironment().value(name);
}

QPointF Application::globalMousePosition() const
{
    return QCursor::pos();
}

QString Application::camelCased(const QString &val) const
{
    if(TransliterationEngine::instance()->language() != TransliterationEngine::English)
        return val;

    QString val2 = val.toLower();

    bool capitalize = true;
    for(int i=0; i<val2.length(); i++)
    {
        QCharRef ch = val2[i];
        if(capitalize)
        {
            if(ch.isLetterOrNumber() && ch.script() == QChar::Script_Latin)
                ch = ch.toUpper();
            capitalize = false;
        }
        else
            capitalize = !ch.isLetterOrNumber();
    }

    return val2;
}

void Application::saveWindowGeometry(QWindow *window, const QString &group)
{
    if(window == nullptr)
        return;

    const QRect geometry = window->geometry();
    if(window->visibility() == QWindow::Windowed)
    {
        const QString geometryString = QString("%1 %2 %3 %4")
                .arg(geometry.x()).arg(geometry.y())
                .arg(geometry.width()).arg(geometry.height());
        m_settings->setValue( group + QStringLiteral("/windowGeometry"), geometryString );
    }
    else
        m_settings->setValue( group + QStringLiteral("/windowGeometry"), QStringLiteral("Maximized") );
}

bool Application::restoreWindowGeometry(QWindow *window, const QString &group)
{
    if(window == nullptr)
        return false;

    const QScreen *screen = window->screen();
    const QRect screenGeo = screen->availableGeometry();

    const QString geometryString = m_settings->value(group + QStringLiteral("/windowGeometry")).toString();
    if(geometryString == QStringLiteral("Maximized"))
    {
        window->setGeometry(screenGeo);
        return true;
    }

    const QStringList geometry = geometryString.split(QStringLiteral(" "), QString::SkipEmptyParts);
    if(geometry.length() != 4)
    {
        window->setGeometry(screenGeo);
        return false;
    }

    const int x = geometry.at(0).toInt();
    const int y = geometry.at(1).toInt();
    const int w = geometry.at(2).toInt();
    const int h = geometry.at(3).toInt();
    QRect geo(x, y, w, h);
    if(!screenGeo.contains(geo))
    {
        if(w > screenGeo.width() || h > screenGeo.height())
        {
            window->setGeometry(screenGeo);
            return false;
        }

        geo.moveCenter(screenGeo.center());
    }

    window->setGeometry(geo);
    return true;
}

void Application::initializeStandardColors(QQmlEngine *)
{
    if(!m_standardColors.isEmpty())
        return;

    const QVector<QColor> colors = this->standardColors();
    for(int i=0; i<colors.size(); i++)
        m_standardColors << QVariant::fromValue<QColor>(colors.at(i));

    emit standardColorsChanged();
}

QVector<QColor> Application::standardColors(const QVersionNumber &version)
{
    // Up-until version 0.2.17 Beta
    if( !version.isNull() && version <= QVersionNumber(0,2,17) )
        return QVector<QColor>() <<
            QColor("blue") << QColor("magenta") << QColor("darkgreen") <<
            QColor("purple") << QColor("yellow") << QColor("orange") <<
            QColor("red") << QColor("brown") << QColor("gray") << QColor("white");

    // New set of colors
    return QVector<QColor>() <<
        QColor("#2196f3") << QColor("#e91e63") << QColor("#009688") <<
        QColor("#9c27b0") << QColor("#ffeb3b") << QColor("#ff9800") <<
        QColor("#f44336") << QColor("#795548") << QColor("#9e9e9e") <<
        QColor("#fafafa") << QColor("#3f51b5") << QColor("#cddc39");
}

#ifdef ENABLE_SCRIPT_HOTKEY

#include <QJSEngine>
#include <QFileDialog>
#include "scritedocument.h"

bool Application::loadScript()
{
    QMessageBox::StandardButton answer = QMessageBox::question(nullptr, "Warning", "Executing scripts on a scrite project is an experimental feature. Are you sure you want to use it?", QMessageBox::Yes|QMessageBox::No);
    if(answer == QMessageBox::No)
        return true;

    ScriteDocument *document = ScriteDocument::instance();
    if(document->isReadOnly())
    {
        QMessageBox::information(nullptr, "Warning", "Cannot execute script on a readonly document.");
        return false;
    }

    QString scriptPath = QDir::homePath();
    if( !document->fileName().isEmpty() )
    {
        QFileInfo fi(document->fileName());
        scriptPath = fi.absolutePath();
    }

    const QString caption("Select a JavaScript file to load");
    const QString filter("JavaScript File (*.js)");
    const QString scriptFile = QFileDialog::getOpenFileName(nullptr, caption, scriptPath, filter);
    if(scriptFile.isEmpty())
        return true;

    auto loadProgram = [](const QString &fileName) {
        QFile file(fileName);
        if(!file.open(QFile::ReadOnly))
            return QString();
        return QString::fromLatin1(file.readAll());
    };
    const QString program = loadProgram(scriptFile);
    if(program.isEmpty())
    {
        QMessageBox::information(nullptr, "Script", "No code was found in the selected file.");
        return true;
    }

    QJSEngine jsEngine;
    QJSValue globalObject = jsEngine.globalObject();
    if(!document->isReadOnly())
        globalObject.setProperty("document", jsEngine.newQObject(document));

    qApp->setOverrideCursor(Qt::WaitCursor);
    const QJSValue result = jsEngine.evaluate(program, scriptFile);
    qApp->restoreOverrideCursor();

    if(result.isError())
    {
        const QString msg = "Uncaught exception at line " +
                result.property("lineNumber").toString() + ": " +
                result.toString();
        QMessageBox::warning(nullptr, "Script", msg);
    }

    return true;
}
#else
bool Application::loadScript() { return false; }
#endif

bool Application::registerFileTypes()
{
#ifdef Q_OS_WIN
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

