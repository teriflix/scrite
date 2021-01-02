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

#ifndef SCREENPLAYTEXTDOCUMENTOFFSETS_H
#define SCREENPLAYTEXTDOCUMENTOFFSETS_H

#include <QTime>
#include <QTextDocument>
#include <QAbstractListModel>

#include "screenplay.h"
#include "formatting.h"
#include "qobjectproperty.h"

class ScreenplayTextDocumentOffsets : public QAbstractListModel
{
    Q_OBJECT

public:
    ScreenplayTextDocumentOffsets(QObject *parent = nullptr);
    ~ScreenplayTextDocumentOffsets();

    Q_PROPERTY(Screenplay* screenplay READ screenplay WRITE setScreenplay NOTIFY screenplayChanged)
    void setScreenplay(Screenplay* val);
    Screenplay* screenplay() const { return m_screenplay; }
    Q_SIGNAL void screenplayChanged();

    Q_PROPERTY(QTextDocument* document READ document WRITE setDocument NOTIFY documentChanged)
    void setDocument(QTextDocument* val);
    QTextDocument* document() const { return m_document; }
    Q_SIGNAL void documentChanged();

    Q_PROPERTY(ScreenplayFormat* format READ format WRITE setFormat NOTIFY formatChanged)
    void setFormat(ScreenplayFormat* val);
    ScreenplayFormat* format() const { return m_format; }
    Q_SIGNAL void formatChanged();

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    void setFileName(const QString &val);
    QString fileName() const { return m_fileName; }
    Q_SIGNAL void fileNameChanged();

    Q_INVOKABLE QString fileNameFrom(const QString &mediaFileNameOrUrl) const;

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorMessageChanged)
    bool hasError() const { return !m_errorMessage.isEmpty(); }

    Q_INVOKABLE void clearErrorMessage() { this->setErrorMessage(QString()); }

    Q_PROPERTY(int offsetCount READ offsetCount NOTIFY offsetCountChanged)
    int offsetCount() const { return m_offsets.size(); }
    Q_SIGNAL void offsetCountChanged();

    Q_INVOKABLE QJsonObject offsetInfoAt(int row) const;
    Q_INVOKABLE QJsonObject offsetInfoAtPoint(const QPointF &pos) const;
    Q_INVOKABLE QJsonObject offsetInfoAtTime(int timeInMs, int rowHint=-1) const;
    Q_INVOKABLE int evaluateTimeAtPoint(const QPointF &pos, int rowHint=-1) const;
    Q_INVOKABLE QPointF evaluatePointAtTime(int timeInMs, int rowHint=-1) const;
    Q_INVOKABLE void setTime(int row, int timeInMs, bool adjustFollowingRows=false);
    Q_INVOKABLE void resetTime(int row, bool andFollowingRows=false);
    Q_INVOKABLE void resetAllTimes();

    enum Roles
    {
        ScreenplayElementIndexRole = Qt::UserRole,
        SceneIndexRole,
        SceneNumberRole,
        SceneHeadingRole,
        PageNumberRole,
        TimeOffsetRole,
        PixelOffsetRole,
        OffsetInfoRole
    };
    QHash<int,QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &data, int role=TimeOffsetRole);

private:
    void reloadDocument();
    void setErrorMessage(const QString &val);
    void loadOffsets();
    void saveOffsets();

private:
    QTimer *m_reloadTimer = nullptr;
    QObjectProperty<Screenplay> m_screenplay;
    QObjectProperty<QTextDocument> m_document;
    QObjectProperty<ScreenplayFormat> m_format;

    struct _OffsetInfo
    {
        int row = -1;
        int elementIndex = -1;
        int sceneIndex = -1;
        QString sceneNumber;
        QString sceneHeading;
        int pageNumber = -1;
        QTime sceneTime;
        qreal pixelOffset = 0;
        QJsonObject toJson() const;
    };
    QList<_OffsetInfo> m_offsets;

    QString m_fileName;
    QString m_errorMessage;
};

#endif // SCREENPLAYTEXTDOCUMENTOFFSETS_H
