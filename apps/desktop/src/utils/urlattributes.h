/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef URLATTRIBUTES_H
#define URLATTRIBUTES_H

#include <QUrl>
#include <QPointer>
#include <QQmlEngine>
#include <QJsonObject>
#include <QNetworkReply>

class UrlAttributes : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit UrlAttributes(QObject *parent = nullptr);
    ~UrlAttributes();

    enum Status {
        Null, // No URL set
        Ready, // Attributes available
        Loading, // Attributes being fetched
        Error // Error while fetching attributes
    };
    Q_ENUM(Status)
    // clang-format off
    Q_PROPERTY(Status status
               READ status
               NOTIFY statusChanged)
    // clang-format on
    Status status() const { return m_status; }
    Q_SIGNAL void statusChanged();

    // clang-format off
    Q_PROPERTY(QUrl url
               READ url
               WRITE setUrl
               NOTIFY urlChanged)
    // clang-format on
    void setUrl(const QUrl &val);
    QUrl url() const { return m_url; }
    Q_SIGNAL void urlChanged();

    // clang-format off
    Q_PROPERTY(bool isUrlValid
               READ isUrlValid
               NOTIFY urlChanged)
    // clang-format on
    bool isUrlValid() const { return !m_url.isEmpty() && m_url.isValid(); }

    // clang-format off
    Q_PROPERTY(QJsonObject attributes
               READ attributes
               NOTIFY attributesChanged)
    // clang-format on
    QJsonObject attributes() const { return m_attributes; }
    Q_SIGNAL void attributesChanged();

private:
    void setStatus(Status val);
    void onHttpRequestFinished();
    void setAttributes(const QJsonObject &val);
    QJsonObject createDefaultAttributes() const;

private:
    QUrl m_url;
    Status m_status = Null;
    QJsonObject m_attributes;
    QPointer<QNetworkReply> m_reply;
};

#endif // URLATTRIBUTES_H
