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

#ifndef URLATTRIBUTES_H
#define URLATTRIBUTES_H

#include <QUrl>
#include <QObject>
#include <QPointer>
#include <QJsonObject>

#ifndef Q_OS_MAC
#include <QWebEnginePage>
#endif

class UrlAttributes : public QObject
{
    Q_OBJECT

public:
    UrlAttributes(QObject *parent=nullptr);
    ~UrlAttributes();

    enum Status
    {
        Null, // No URL set
        Ready, // Attributes available
        Loading, // Attributes being fetched
        Error // Error while fetching attributes
    };
    Q_ENUM(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Status status() const { return m_status; }
    Q_SIGNAL void statusChanged();

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    void setUrl(const QUrl &val);
    QUrl url() const { return m_url; }
    Q_SIGNAL void urlChanged();

    Q_PROPERTY(QJsonObject attributes READ attributes NOTIFY attributesChanged)
    QJsonObject attributes() const { return m_attributes; }
    Q_SIGNAL void attributesChanged();

private:
    void onWebPageLoadFinished(bool ok);
    void setStatus(Status val);
    void setAttributes(const QJsonObject &val);
    QJsonObject createDefaultAttributes() const;

private:
    QUrl m_url;
    Status m_status = Null;
    QJsonObject m_attributes;
#ifndef Q_OS_MAC
    QPointer<QWebEnginePage> m_webPage;
#endif
};

#endif // URLATTRIBUTES_H
