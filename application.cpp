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

#include "application.h"
#include "autoupdate.h"
#include "logger.h"
#include "undoredo.h"

#include <QDir>
#include <QtDebug>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMetaEnum>
#include <QQuickItem>
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QColorDialog>
#include <QFontDatabase>
#include <QStandardPaths>

#define ENABLE_SCRIPT_HOTKEY

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
    connect(this, &QGuiApplication::fontChanged, this, &Application::applicationFontChanged);

    this->setWindowIcon( QIcon(":/images/appicon.png") );
    this->setBaseWindowTitle("scrite - build your screenplay");

    const QString settingsFile = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absoluteFilePath("settings.ini");
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);

    TransliterationEngine::instance(this);
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
    m_timer.stop();
}

void ExecLater::timerEvent(QTimerEvent *event)
{
    if(m_timer.timerId() == event->timerId())
    {
        m_timer.stop();
        m_function.call(m_arguments);
        GarbageCollector::instance()->add(this);
    }
}

void Application::execLater(QObject *context, int howMuchLater, const QJSValue &function, const QJSValueList &args)
{
    QObject *parent = context ? context : this;
    new ExecLater(howMuchLater, function, args, parent);
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

    QJsonArray fields;
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
    }

    ret.insert("fields", fields);

    return ret;
}

bool Application::notify(QObject *object, QEvent *event)
{
    QPointer<QObject> guard(object);

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

    if(guard.isNull())
        return false;

    const bool ret = QtApplicationClass::notify(object, event);

    if(event->type() == QEvent::ChildAdded && !guard.isNull())
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
    double luma = ((0.299 * bgColor.redF()) + (0.587 * bgColor.greenF()) + (0.114 * bgColor.blueF()));
    return luma > 0.5 ? Qt::black : Qt::white;
}

QRectF Application::boundingRect(const QString &text, const QFont &font) const
{
    const QFontMetricsF fm(font);
    return fm.boundingRect(text);
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
    globalObject.setProperty("document", jsEngine.newQObject(document));
    const QJSValue result = jsEngine.evaluate(program, scriptFile);
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
