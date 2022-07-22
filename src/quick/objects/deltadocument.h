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

#ifndef DELTADOCUMENT_H
#define DELTADOCUMENT_H

#include <QQmlEngine>
#include <QJsonObject>

class QTextCursor;

class DeltaDocument : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DeltaDocument(QObject *parent = nullptr);
    ~DeltaDocument();

    Q_PROPERTY(QJsonValue content READ content WRITE setContent NOTIFY contentChanged)
    void setContent(const QJsonValue &val);
    QJsonValue content() const { return m_content; }
    Q_SIGNAL void contentChanged();

    Q_PROPERTY(QString plainText READ plainText NOTIFY plainTextChanged)
    QString plainText() const { return m_plainText; }
    Q_SIGNAL void plainTextChanged();

    Q_PROPERTY(QString html READ html NOTIFY htmlChanged)
    QString html() const { return m_html; }
    Q_SIGNAL void htmlChanged();

    struct ResolveResult
    {
        ResolveResult() { }
        ResolveResult(int _callId, const QString &_plainText, const QString &_htmlText)
            : callId(_callId), plainText(_plainText), htmlText(_htmlText)
        {
        }
        int callId = 0;
        QString plainText;
        QString htmlText;
    };

    static ResolveResult blockingResolve(const QJsonObject &content, int callId = 0);
    static void asyncResolve(const QJsonObject &content, int callId, QObject *receiver,
                             std::function<void(const ResolveResult &result)> function);
    static void blockingResolveAndInsertHtml(const QJsonObject &content, QTextCursor &cursor);

private:
    void setHtml(const QString &val);
    void setPlainText(const QString &val);
    void transformNow();
    void transformLater();

private:
    QString m_html;
    QString m_plainText;
    QJsonValue m_content;
    int m_modificationCounter = 0;
};

#endif // DELTADOCUMENT_H
