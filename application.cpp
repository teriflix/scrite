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
#include <QProcess>
#include <QSettings>
#include <QFileInfo>
#include <QMetaEnum>
#include <QJsonArray>
#include <QMessageBox>
#include <QJsonObject>
#include <QColorDialog>
#include <QFontDatabase>
#include <QStandardPaths>

Application *Application::instance()
{
    return qobject_cast<Application*>(qApp);
}

Application::Application(int &argc, char **argv, const QVersionNumber &version)
    : QtApplicationClass(argc, argv),
      m_errorReport(new ErrorReport(this)),
      m_versionNumber(version)
{
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

bool Application::notify(QObject *object, QEvent *event)
{
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
