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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QUrl>
#include <QRectF>
#include <QColor>
#include <QAction>
#include <QPalette>
#include <QUndoGroup>
#include <QUndoStack>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QVersionNumber>

#include "undostack.h"
#include "errorreport.h"
#include "transliteration.h"

typedef QApplication QtApplicationClass;
class QSettings;
class QQuickItem;

class Application : public QtApplicationClass
{
    Q_OBJECT

public:
    static Application *instance();
    Application(int &argc, char **argv, const QVersionNumber &version);
    ~Application();

    Q_PROPERTY(QPalette palette READ palette CONSTANT)

    Q_INVOKABLE QString urlToLocalFile(const QUrl &url) { return url.toLocalFile(); }

    enum Platform { LinuxDesktop, WindowsDesktop, MacOS };
    Q_ENUM(Platform)
    Q_PROPERTY(Platform platform READ platform CONSTANT)
    Platform platform() const;

    Q_PROPERTY(bool isMacOSPlatform READ isMacOSPlatform CONSTANT)
#ifdef Q_OS_MAC
    bool isMacOSPlatform() const { return true; }
#else
    bool isMacOSPlatform() const { return false; }
#endif

    Q_PROPERTY(bool isWindowsPlatform READ isWindowsPlatform CONSTANT)
#ifdef Q_OS_WIN
    bool isWindowsPlatform() const { return true; }
#else
    bool isWindowsPlatform() const { return false; }
#endif

    Q_PROPERTY(bool isLinuxPlatform READ isLinuxPlatform CONSTANT)
#ifdef Q_OS_MAC
    bool isLinuxPlatform() const { return false; }
#else
#ifdef Q_OS_UNIX
    bool isLinuxPlatform() const { return true; }
#else
    bool isLinuxPlatform() const { return false; }
#endif
#endif

    Q_PROPERTY(QString controlKey READ controlKey CONSTANT)
    QString controlKey() const;

    Q_PROPERTY(QString altKey READ altKey CONSTANT)
    QString altKey() const;

    Q_INVOKABLE QString polishShortcutTextForDisplay(const QString &text) const;

    Q_PROPERTY(QString baseWindowTitle READ baseWindowTitle WRITE setBaseWindowTitle NOTIFY baseWindowTitleChanged)
    void setBaseWindowTitle(const QString &val);
    QString baseWindowTitle() const { return m_baseWindowTitle; }
    Q_SIGNAL void baseWindowTitleChanged();

    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    QString qtVersion() const { return QString::fromLatin1(QT_VERSION_STR); }

    Q_INVOKABLE QString typeName(QObject *object) const;

    Q_PROPERTY(QVersionNumber versionNumber READ versionNumber CONSTANT)
    QVersionNumber versionNumber() const { return m_versionNumber; }

    Q_PROPERTY(QUndoGroup* undoGroup READ undoGroup CONSTANT)
    QUndoGroup *undoGroup() const { return m_undoGroup; }

    Q_INVOKABLE UndoStack *findUndoStack(const QString &objectName) const;

    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    bool canUndo() const { return m_undoGroup->canUndo(); }
    Q_SIGNAL void canUndoChanged();

    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    bool canRedo() const { return m_undoGroup->canRedo(); }
    Q_SIGNAL void canRedoChanged();

    Q_PROPERTY(QString undoText READ undoText NOTIFY undoTextChanged)
    QString undoText() const { return m_undoGroup->undoText(); }
    Q_SIGNAL void undoTextChanged();

    Q_PROPERTY(QString redoText READ redoText NOTIFY redoTextChanged)
    QString redoText() const { return m_undoGroup->redoText(); }
    Q_SIGNAL void redoTextChanged();

    Q_INVOKABLE QJsonObject systemFontInfo() const;
    Q_INVOKABLE QColor pickColor(const QColor &initial) const;
    Q_INVOKABLE QRectF textBoundingRect(const QString &text, const QFont &font) const;
    Q_INVOKABLE void revealFileOnDesktop(const QString &pathIn);
    Q_INVOKABLE QJsonArray enumerationModel(QObject *object, const QString &enumName) const;
    Q_INVOKABLE QJsonObject fileInfo(const QString &path) const;

    Q_PROPERTY(QString settingsFilePath READ settingsFilePath CONSTANT)
    QString settingsFilePath() const;

    Q_PROPERTY(TransliterationSettings* transliterationSettings READ transliterationSettings CONSTANT)
    TransliterationSettings* transliterationSettings() const { return TransliterationSettings::instance(); }

    Q_INVOKABLE QPointF cursorPosition() const;
    Q_INVOKABLE QPointF mapGlobalPositionToItem(QQuickItem *item, const QPointF &pos) const;

    Q_INVOKABLE void execLater(int howMuchLater, const QJSValue &function, const QJSValueList &args=QJSValueList());

    QSettings *settings() const { return m_settings; }

    // QCoreApplication interface
    bool notify(QObject *, QEvent *);

signals:
    void minimizeWindowRequest();

private:
    QSettings *m_settings = nullptr;
    QUndoGroup *m_undoGroup = new QUndoGroup(this);
    QString m_baseWindowTitle;
    ErrorReport *m_errorReport = new ErrorReport(this);
    QVersionNumber m_versionNumber;
};

#endif // APPLICATION_H
