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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QUrl>
#include <QTime>
#include <QRectF>
#include <QColor>
#include <QAction>
#include <QPalette>
#include <QUndoGroup>
#include <QUndoStack>
#include <QJsonArray>
#include <QJsonObject>
#include <QQuickWindow>
#include <QApplication>
#include <QFontDatabase>
#include <QVersionNumber>

#include "undoredo.h"
#include "errorreport.h"

typedef QApplication QtApplicationClass;

class Forms;
class QSettings;
class QQuickItem;
class AutoUpdate;
class QNetworkConfigurationManager;

class Application : public QtApplicationClass
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use Scrite.app to access the only application instance.")

public:
    static QVersionNumber prepare();
    static Application *instance();

    explicit Application(int &argc, char **argv, const QVersionNumber &version);
    virtual ~Application();

    QString deviceId() const;
    QString installationId() const;
    QDateTime installationTimestamp() const;

    // clang-format off
    Q_PROPERTY(int appState
               READ appState
               NOTIFY appStateChanged)
    // clang-format on
    int appState() const;
    Q_SIGNAL void appStateChanged();

    // clang-format off
    Q_PROPERTY(int launchCounter
               READ launchCounter
               CONSTANT )
    // clang-format on
    int launchCounter() const;

    // clang-format off
    Q_PROPERTY(QString buildTimestamp
               READ buildTimestamp
               CONSTANT )
    // clang-format on
    QString buildTimestamp() const;

    // clang-format off
    Q_PROPERTY(QPalette palette
               READ palette
               CONSTANT )
    // clang-format on

    // clang-format off
    Q_PROPERTY(qreal devicePixelRatio
               READ devicePixelRatio
               CONSTANT )
    // clang-format on

    // clang-format off
    Q_PROPERTY(QFont font
               READ applicationFont
               NOTIFY applicationFontChanged)
    // clang-format on
    QFont applicationFont() const { return this->font(); }
    Q_SIGNAL void applicationFontChanged();

    // clang-format off
    Q_PROPERTY(int idealFontPointSize
               READ idealFontPointSize
               NOTIFY idealFontPointSizeChanged)
    // clang-format on
    int idealFontPointSize() const { return m_idealFontPointSize; }
    Q_SIGNAL void idealFontPointSizeChanged();

    // clang-format off
    Q_PROPERTY(int customFontPointSize
               READ customFontPointSize
               WRITE setCustomFontPointSize
               NOTIFY customFontPointSizeChanged)
    // clang-format on
    void setCustomFontPointSize(int val);
    int customFontPointSize() const { return m_customFontPointSize; }
    Q_SIGNAL void customFontPointSizeChanged();

    // clang-format off
    Q_PROPERTY(QVersionNumber version
               READ version
               CONSTANT )
    // clang-format on
    QVersionNumber version() const { return m_versionNumber; }

    // clang-format off
    Q_PROPERTY(QString versionAsString
               READ versionAsString
               CONSTANT )
    // clang-format on
    QString versionAsString() const { return m_versionNumber.toString(); }

    // clang-format off
    Q_PROPERTY(QString versionType
               MEMBER versionType
               CONSTANT)
    // clang-format on
    static const QString versionType;

    // clang-format off
    Q_PROPERTY(QStringList availableThemes
               READ availableThemes
               CONSTANT )
    // clang-format on
    static QStringList availableThemes();

    static QString queryQtQuickStyleFor(const QString &theme);

    // clang-format off
    Q_PROPERTY(bool usingMaterialTheme
               READ usingMaterialTheme
               CONSTANT )
    // clang-format on
    static bool usingMaterialTheme();

    // clang-format off
    Q_PROPERTY(QString baseWindowTitle
               READ baseWindowTitle
               WRITE setBaseWindowTitle
               NOTIFY baseWindowTitleChanged)
    // clang-format on
    void setBaseWindowTitle(const QString &val);
    QString baseWindowTitle() const { return m_baseWindowTitle; }
    Q_SIGNAL void baseWindowTitleChanged();

    // clang-format off
    Q_PROPERTY(QVersionNumber versionNumber
               READ versionNumber
               CONSTANT )
    // clang-format on
    QVersionNumber versionNumber() const { return m_versionNumber; }

    QString settingsFilePath() const;

    // clang-format off
    Q_PROPERTY(AutoUpdate *autoUpdate
               READ autoUpdate
               CONSTANT )
    // clang-format on
    AutoUpdate *autoUpdate() const;

    // clang-format off
    Q_PROPERTY(Forms *forms
               READ forms
               CONSTANT )
    // clang-format on
    Forms *forms() const;

    static QFontDatabase &fontDatabase();

    // TODO: Move this to Platform.fontInfo() ...
    Q_INVOKABLE static QJsonObject systemFontInfo();

    // Use File.revealOnDesktop(), which calls this function anyway
    void revealFileOnDesktop(const QString &pathIn);

    QSettings *settings() const { return m_settings; }

    Q_INVOKABLE static QScreen *windowScreen(QObject *window);

    Q_INVOKABLE bool hasActiveFocus(QQuickWindow *window, QQuickItem *item);
    Q_INVOKABLE bool maybeOpenAnonymously();
    Q_INVOKABLE bool restoreWindowGeometry(QWindow *window, const QString &group);
    Q_INVOKABLE void launchNewInstance(QWindow *window);
    Q_INVOKABLE void launchNewInstanceAndOpen(QWindow *window, const QString &filePath);
    Q_INVOKABLE void launchNewInstanceAndOpenAnonymously(QWindow *window, const QString &filePath);
    Q_INVOKABLE void saveWindowGeometry(QWindow *window, const QString &group);
    Q_INVOKABLE void startNewInstance(QWindow *window, const QString &filePath, bool anonymously);
    Q_INVOKABLE void toggleFullscreen(QWindow *window);

    // Must be called from main.cpp
    void initializeStandardColors(QQmlEngine *);

    // QCoreApplication interface
    bool notify(QObject *, QEvent *);

    // Although public, please do not call it.
    bool notifyInternal(QObject *object, QEvent *event);
    void computeIdealFontPointSize();

#ifdef Q_OS_MAC
    QString fileToOpen() const { return m_fileToOpen; }
    void setHandleFileOpenEvents(bool val = true) { m_handleFileOpenEvents = val; }
#endif

    bool event(QEvent *event);

signals:
    void minimizeWindowRequest();
    void openFileRequest(const QString &filePath);

private:
    bool registerFileTypes();

private:
#ifdef Q_OS_MAC
    QString m_fileToOpen;
    bool m_handleFileOpenEvents = false;
#endif
    QSettings *m_settings = nullptr;
    QUndoGroup *m_undoGroup = new QUndoGroup(this);
    int m_idealFontPointSize = 12;
    int m_customFontPointSize = 0;
    QString m_baseWindowTitle;
    ErrorReport *m_errorReport = new ErrorReport(this);
    QVersionNumber m_versionNumber;
    QVariantList m_standardColors;
};

#endif // APPLICATION_H
