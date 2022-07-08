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

#ifndef SCREENPLAYTEXTDOCUMENTOFFSETS_H
#define SCREENPLAYTEXTDOCUMENTOFFSETS_H

#include <QTime>
#include <QTextDocument>

#include "screenplay.h"
#include "formatting.h"
#include "qobjectproperty.h"
#include "genericarraymodel.h"

class ScreenplayTextDocumentOffsets : public GenericArrayModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ScreenplayTextDocumentOffsets(QObject *parent = nullptr);
    ~ScreenplayTextDocumentOffsets();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged)
    void setScreenplay(Screenplay *val);
    Screenplay *screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(QTextDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    void setDocument(QTextDocument *val);
    QTextDocument *document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    Q_PROPERTY(ScreenplayFormat* format READ format WRITE setFormat NOTIFY formatChanged)
    void setFormat(ScreenplayFormat *val);
    ScreenplayFormat *format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    void setFileName(const QString &val);
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    bool isBusy() const { return m_busy; }
    Q_SIGNAL void busyChanged();

    Q_INVOKABLE QString fileNameFrom(const QString &mediaFileNameOrUrl) const;

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorMessageChanged)
    bool hasError() const { return !m_errorMessage.isEmpty(); }

    Q_INVOKABLE void clearErrorMessage() { this->setErrorMessage(QString()); }

    Q_INVOKABLE QString timestampToString(int timeInMs) const;

    Q_INVOKABLE QJsonObject offsetInfoAt(int row) const { return this->at(row).toObject(); }
    Q_INVOKABLE QJsonObject offsetInfoAtPoint(const QPointF &pos) const;
    Q_INVOKABLE QJsonObject offsetInfoAtTime(int timeInMs, int rowHint = -1) const;
    Q_INVOKABLE int evaluateTimeAtPoint(const QPointF &pos, int rowHint = -1) const;
    Q_INVOKABLE QPointF evaluatePointAtTime(int timeInMs, int rowHint = -1) const;
    Q_INVOKABLE void setTime(int row, int timeInMs, bool adjustFollowingRows = false);
    Q_INVOKABLE void resetTime(int row, bool andFollowingRows = false);
    Q_INVOKABLE void toggleSceneTimeLock(int row);
    Q_INVOKABLE void adjustUnlockedTimes(int duration = 0);
    Q_INVOKABLE void unlockAllSceneTimes();
    Q_INVOKABLE void resetAllTimes();

    Q_INVOKABLE int currentSceneHeadingIndex(int row) const;
    Q_INVOKABLE int nextSceneHeadingIndex(int row) const;
    Q_INVOKABLE int previousSceneHeadingIndex(int row) const;

private:
    void setBusy(bool val);
    void reloadDocument();
    void reloadDocumentNow();
    void setErrorMessage(const QString &val);
    void loadOffsets();
    void saveOffsets();

private:
    bool m_busy = false;
    QTimer *m_reloadTimer = nullptr;
    QObjectProperty<Screenplay> m_screenplay;
    QObjectProperty<QTextDocument> m_document;
    QObjectProperty<ScreenplayFormat> m_format;

    QString m_fileName;
    QString m_errorMessage;
};

#endif // SCREENPLAYTEXTDOCUMENTOFFSETS_H
