/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "application.h"

#include <QDir>
#include <QtDebug>
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMetaEnum>
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QColorDialog>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QQuickItem>

Application *Application::instance()
{
    return qobject_cast<Application*>(qApp);
}

Application::Application(int &argc, char **argv, const QVersionNumber &version)
    : QtApplicationClass(argc, argv),
      m_versionNumber(version)
{
    connect(m_undoGroup, &QUndoGroup::canUndoChanged, this, &Application::canUndoChanged);
    connect(m_undoGroup, &QUndoGroup::canRedoChanged, this, &Application::canRedoChanged);
    connect(m_undoGroup, &QUndoGroup::undoTextChanged, this, &Application::undoTextChanged);
    connect(m_undoGroup, &QUndoGroup::redoTextChanged, this, &Application::redoTextChanged);

    this->setBaseWindowTitle("scrite - build your screenplay");

    const QString settingsFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("settings.ini");
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);

    TransliterationSettings::instance(this);
}

Application::~Application()
{

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
    text2.replace("Ctrl", this->controlKey(), Qt::CaseInsensitive);
    text2.replace("Alt", this->altKey(), Qt::CaseInsensitive);
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
    QFontDatabase fontdb;

    QJsonObject ret;
    ret.insert("families", QJsonArray::fromStringList(fontdb.families()));

    QJsonArray sizes;
    QList<int> stdSizes = fontdb.standardSizes();
    Q_FOREACH(int stdSize, stdSizes)
        sizes.append( QJsonValue(stdSize) );
    ret.insert("standardSizes", sizes);

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
        const QString explorer = QStandardPaths::locate(QStandardPaths::AppDataLocation, "explorer.exe");
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

QJsonArray Application::enumerationModel(QObject *object, const QString &enumName) const
{
    QJsonArray ret;

    if( object == nullptr || enumName.isEmpty() )
        return ret;

    const QMetaObject *mo = object->metaObject();
    const int enumIndex = mo->indexOfEnumerator( qPrintable(enumName) );
    if( enumIndex < 0 )
        return ret;

    const QMetaEnum enumInfo = mo->enumerator(enumIndex);
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
    char m_padding[4];
    QBasicTimer m_timer;
    QJSValue m_function;
    QJSValueList m_arguments;
};

ExecLater::ExecLater(int howMuchLater, const QJSValue &function, const QJSValueList &args, QObject *parent)
    : QObject(parent), m_function(function), m_arguments(args)
{
    howMuchLater = qBound(0, howMuchLater, 60*60*1000);
    m_timer.start(howMuchLater, this);
}

ExecLater::~ExecLater()
{
}

void ExecLater::timerEvent(QTimerEvent *event)
{
    if(m_timer.timerId() == event->timerId())
    {
        m_function.call(m_arguments);
        m_timer.stop();

        GarbageCollector::instance()->add(this);
    }
}

void Application::execLater(int howMuchLater, const QJSValue &function, const QJSValueList &args)
{
    new ExecLater(howMuchLater, function, args, this);
}

bool Application::notify(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if(ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_M)
        {
            emit minimizeWindowRequest();
            return true;
        }
    }

    const bool ret = QtApplicationClass::notify(object, event);

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
