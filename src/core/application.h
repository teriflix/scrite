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
#include "qobjectlistmodel.h"

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
    ~Application();

    QString deviceId() const;
    QString installationId() const;
    QDateTime installationTimestamp() const;

    Q_PROPERTY(int appState READ appState NOTIFY appStateChanged)
    int appState() const;
    Q_SIGNAL void appStateChanged();

    Q_PROPERTY(int launchCounter READ launchCounter CONSTANT)
    int launchCounter() const;

    Q_PROPERTY(QString buildTimestamp READ buildTimestamp CONSTANT)
    QString buildTimestamp() const;

    Q_PROPERTY(QPalette palette READ palette CONSTANT)
    Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio CONSTANT)
    Q_PROPERTY(QFont font READ applicationFont NOTIFY applicationFontChanged)
    QFont applicationFont() const { return this->font(); }
    Q_SIGNAL void applicationFontChanged();

    Q_PROPERTY(int idealFontPointSize READ idealFontPointSize NOTIFY idealFontPointSizeChanged)
    int idealFontPointSize() const { return m_idealFontPointSize; }
    Q_SIGNAL void idealFontPointSizeChanged();

    Q_PROPERTY(int customFontPointSize READ customFontPointSize WRITE setCustomFontPointSize NOTIFY
                       customFontPointSizeChanged)
    void setCustomFontPointSize(int val);
    int customFontPointSize() const { return m_customFontPointSize; }
    Q_SIGNAL void customFontPointSizeChanged();

    Q_PROPERTY(QVersionNumber version READ version CONSTANT)
    QVersionNumber version() const { return m_versionNumber; }

    Q_PROPERTY(QString versionAsString READ versionAsString CONSTANT)
    QString versionAsString() const { return m_versionNumber.toString(); }

    Q_PROPERTY(QStringList availableThemes READ availableThemes CONSTANT)
    static QStringList availableThemes();
    static QString queryQtQuickStyleFor(const QString &theme);

    Q_PROPERTY(bool usingMaterialTheme READ usingMaterialTheme CONSTANT)
    static bool usingMaterialTheme();

    Q_PROPERTY(QString baseWindowTitle READ baseWindowTitle WRITE setBaseWindowTitle NOTIFY
                       baseWindowTitleChanged)
    void setBaseWindowTitle(const QString &val);
    QString baseWindowTitle() const { return m_baseWindowTitle; }
    Q_SIGNAL void baseWindowTitleChanged();

    Q_PROPERTY(QVersionNumber versionNumber READ versionNumber CONSTANT)
    QVersionNumber versionNumber() const { return m_versionNumber; }

    Q_PROPERTY(QString qtVersionString READ qtVersionString CONSTANT)
    static QString qtVersionString() { return QStringLiteral(QT_VERSION_STR); }

    Q_PROPERTY(QString openSslVersionString READ openSslVersionString CONSTANT)
    static QString openSslVersionString();

    Q_PROPERTY(QUndoGroup *undoGroup READ undoGroup CONSTANT)
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

    static QFontDatabase &fontDatabase();

    // TODO: Move this to Platform.fontInfo() ...
    Q_INVOKABLE static QJsonObject systemFontInfo();

    // Use File.revealOnDesktop(), which calls this function anyway
    void revealFileOnDesktop(const QString &pathIn);

    Q_PROPERTY(QString settingsFilePath READ settingsFilePath CONSTANT)
    QString settingsFilePath() const;

    QSettings *settings() const { return m_settings; }

    Q_PROPERTY(AutoUpdate *autoUpdate READ autoUpdate CONSTANT)
    AutoUpdate *autoUpdate() const;

    // GMath.scaledSize()
    Q_INVOKABLE static QSizeF scaledSize(const QSizeF &of, const QSizeF &into);

    // GMath.uniteRectangles()
    Q_INVOKABLE static QRectF uniteRectangles(const QRectF &r1, const QRectF &r2);

    // GMath.adjustRectangle()
    Q_INVOKABLE static QRectF adjustRectangle(const QRectF &rect, qreal left, qreal top,
                                              qreal right, qreal bottom);

    // GMath.isRectangleInRectangle()
    Q_INVOKABLE static bool isRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect);

    // GMath.translationRequiredToBringRectangleInRectangle()
    Q_INVOKABLE static QPointF
    translationRequiredToBringRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect);

    // GMath.distanceBetweenPoints()
    Q_INVOKABLE static qreal distanceBetweenPoints(const QPointF &p1, const QPointF &p2);

    // GMath.querySubRectangle()
    Q_INVOKABLE static QRectF querySubRectangle(const QRectF &in, const QRectF &around,
                                                const QSizeF &atBest);

    // MouseCursor.position()
    Q_INVOKABLE QPoint mouseCursorPosition() const { return QCursor::pos(); }

    // MouseCursor.moveTo()
    Q_INVOKABLE void moveMouseCursor(const QPoint &pos) { QCursor::setPos(pos); }

    // File.copyToFolder()
    Q_INVOKABLE static QString copyFile(const QString &fromFilePath, const QString &toFolder);

    // File.write()
    Q_INVOKABLE static bool writeToFile(const QString &fileName, const QString &fileContent);

    // File.read()
    Q_INVOKABLE static QString fileContents(const QString &fileName);

    // File.completeBaseName()
    Q_INVOKABLE static QString fileName(const QString &path);

    // File.path()
    Q_INVOKABLE static QString filePath(const QString &fileName);

    // File.neighbouringFilePath()
    Q_INVOKABLE static QString neighbouringFilePath(const QString &filePath,
                                                    const QString &nfileName);

    // Clipboard.set()
    Q_INVOKABLE static bool copyToClipboard(const QString &text);

    Q_INVOKABLE static QScreen *windowScreen(QObject *window);

    // SystemEnvironment.get()
    Q_INVOKABLE static QString getEnvironmentVariable(const QString &name);

    // SystemEnvironment.get()
    Q_INVOKABLE static QString getWindowsEnvironmentVariable(const QString &name,
                                                             const QString &defaultValue);

    // SystemEnvironment.set()
    Q_INVOKABLE static void changeWindowsEnvironmentVariable(const QString &name,
                                                             const QString &value);

    // SystemEnvironment.remove()
    Q_INVOKABLE static void removeWindowsEnvironmentVariable(const QString &name);

    // MouseCursor.position()
    Q_INVOKABLE static QPointF globalMousePosition();

    // SMath.titleCased()
    Q_INVOKABLE static QString camelCased(const QString &val);

    Q_INVOKABLE void saveWindowGeometry(QWindow *window, const QString &group);
    Q_INVOKABLE bool restoreWindowGeometry(QWindow *window, const QString &group);
    Q_INVOKABLE void launchNewInstance(QWindow *window);
    Q_INVOKABLE void launchNewInstanceAndOpenAnonymously(QWindow *window, const QString &filePath);
    Q_INVOKABLE void launchNewInstanceAndOpen(QWindow *window, const QString &filePath);
    Q_INVOKABLE void startNewInstance(QWindow *window, const QString &filePath, bool anonymously);
    Q_INVOKABLE bool maybeOpenAnonymously();
    Q_INVOKABLE void toggleFullscreen(QWindow *window);
    Q_INVOKABLE bool hasActiveFocus(QQuickWindow *window, QQuickItem *item);

    // Object.resetProperty()
    Q_INVOKABLE static bool resetObjectProperty(QObject *object, const QString &propName);

    // Object.save()
    Q_INVOKABLE static bool saveObjectConfiguration(QObject *object);

    // Object.load()
    Q_INVOKABLE static bool restoreObjectConfiguration(QObject *object);

    // Object.treeSize()
    Q_INVOKABLE static int objectTreeSize(QObject *ptr);

    // SMath.createUniqueId()
    Q_INVOKABLE static QString createUniqueId();

    // TMath.sleep()
    Q_INVOKABLE static void sleep(int ms);

    // TMath.secondsToTime()
    Q_INVOKABLE static QTime secondsToTime(int nrSeconds);

    // TMath.relativeTime()
    Q_INVOKABLE static QString relativeTime(const QDateTime &dt);

    Q_PROPERTY(Forms *forms READ forms CONSTANT)
    Forms *forms() const;

    // Gui.emptyQImage
    Q_PROPERTY(QImage emptyQImage READ emptyQImage CONSTANT)
    static QImage emptyQImage() { return QImage(); }

    // Must be called from main.cpp
    void initializeStandardColors(QQmlEngine *);

    Q_INVOKABLE static QList<QColor>
    standardColorsForVersion(const QVersionNumber &version = QVersionNumber());

    // QCoreApplication interface
    bool notify(QObject *, QEvent *);

    // Although public, please do not call it.
    bool notifyInternal(QObject *object, QEvent *event);
    void computeIdealFontPointSize();

#ifdef Q_OS_MAC
    QString fileToOpen() const { return m_fileToOpen; }
    void setHandleFileOpenEvents(bool val = true) { m_handleFileOpenEvents = val; }
#endif

    // Utils::SMath::painterPathToString()
    static QString painterPathToString(const QPainterPath &val);

    // Utils::SMath::stringToPainterPath()
    static QPainterPath stringToPainterPath(const QString &val);

    // Utils::SMath::replaceCharacterName()
    static QJsonObject replaceCharacterName(const QString &from, const QString &to,
                                            const QJsonObject &delta,
                                            int *nrReplacements = nullptr);

    // Utils::SMath::replaceCharacterName()
    static QString replaceCharacterName(const QString &from, const QString &to, const QString &in,
                                        int *nrReplacements = nullptr);

    Q_SIGNAL void openFileRequest(const QString &filePath);

    // Utils::File::sanitiseName
    static QString sanitiseFileName(const QString &fileName, QSet<QChar> *removedChars = nullptr);

    // Gui.log() or Utils::Gui::log()
    Q_INVOKABLE static void log(const QString &message);

    bool event(QEvent *event);

signals:
    void minimizeWindowRequest();

public:
    Q_PROPERTY(QAbstractListModel *objectReigstry READ objectRegistry CONSTANT STORED false)
    QObjectListModel<QObject *> *objectRegistry() const
    {
        return &(const_cast<Application *>(this)->m_objectRegistry);
    }

    // ObjectRegistry.add()
    Q_INVOKABLE QString registerObject(QObject *object, const QString &name);

    // ObjectRegistry.remove()
    Q_INVOKABLE void unregisterObject(QObject *object);

    // ObjectRegistry.find()
    Q_INVOKABLE QObject *findRegisteredObject(const QString &name) const;

    // Utils::Object::firstChildByType
    Q_INVOKABLE static QObject *findFirstChildOfType(QObject *object, const QString &className);

    // Utils::Object::firstParentByType
    Q_INVOKABLE static QObject *findFirstParentOfType(QObject *object, const QString &className);

    // Utils::Object::firstSiblingByType
    Q_INVOKABLE static QObject *findFirstSiblingOfType(QObject *object, const QString &className);

    // Utils::Object::parentOf
    Q_INVOKABLE static QObject *parentOf(QObject *object);

    // Utils::Object::reparent
    Q_INVOKABLE static bool reparent(QObject *object, QObject *newParent);

private:
    bool loadScript();
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
    QObjectListModel<QObject *> m_objectRegistry;
    QNetworkConfigurationManager *m_networkConfiguration = nullptr;
};

#endif // APPLICATION_H
