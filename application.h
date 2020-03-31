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
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QVersionNumber>

#include "errorreport.h"

typedef QApplication QtApplicationClass;

class Application : public QtApplicationClass
{
    Q_OBJECT

public:
    static Application *instance();
    Application(int &argc, char **argv, const QVersionNumber &version);
    ~Application();

    Q_INVOKABLE QString urlToLocalFile(const QUrl &url) {
        return url.toLocalFile();
    }

    enum Platform { LinuxDesktop, WindowsDesktop, MacOS };
    Q_ENUM(Platform)
    Q_PROPERTY(Platform platform READ platform CONSTANT)
    Platform platform() const;

    Q_PROPERTY(QString controlKey READ controlKey CONSTANT)
    QString controlKey() const;

    Q_INVOKABLE QString polishShortcutTextForDispaly(const QString &text) const;

    Q_PROPERTY(QString baseWindowTitle READ baseWindowTitle WRITE setBaseWindowTitle NOTIFY baseWindowTitleChanged)
    void setBaseWindowTitle(const QString &val);
    QString baseWindowTitle() const { return m_baseWindowTitle; }
    Q_SIGNAL void baseWindowTitleChanged();

    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    QString qtVersion() const { return QString::fromLatin1(QT_VERSION_STR); }

    Q_INVOKABLE QString typeName(QObject *object) const;

    Q_PROPERTY(QVersionNumber versionNumber READ versionNumber CONSTANT)
    QVersionNumber versionNumber() const { return m_versionNumber; }

    Q_INVOKABLE QJsonObject systemFontInfo() const;
    Q_INVOKABLE QColor pickColor(const QColor &initial) const;
    Q_INVOKABLE QRectF textBoundingRect(const QString &text, const QFont &font) const;
    Q_INVOKABLE void revealFileOnDesktop(const QString &pathIn);
    Q_INVOKABLE QJsonArray enumerationModel(QObject *object, const QString &enumName) const;

    // QCoreApplication interface
    bool notify(QObject *, QEvent *);

private:
    ErrorReport *m_errorReport;
    QString m_baseWindowTitle;
    QVersionNumber m_versionNumber;
};

#endif // APPLICATION_H
