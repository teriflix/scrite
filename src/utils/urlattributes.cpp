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

#include "hourglass.h"
#include "urlattributes.h"
#include "garbagecollector.h"

#include <QFile>
#include <QTimer>

/**
 * https://bugreports.qt.io/browse/QTBUG-78240
 *
 * Qt 5.13.x on macOS crashes whenever QWebEnginePage is initialized. This happens
 * when it tries to query for the default font family on macOS. The issue has been
 * fixed in Qt 5.14. Until we move past 5.13, we have to unfortunately turn off
 * fetching Open Graph Attributes in UrlAttributes class on macOS.
 */

UrlAttributes::UrlAttributes(QObject *parent)
    : QObject(parent)
{
}

UrlAttributes::~UrlAttributes()
{

}

void UrlAttributes::setUrl(const QUrl &val)
{
    if(m_url == val
#ifndef Q_OS_MAC
            || !m_webPage.isNull()
#endif
      )
        return;

    m_url = val;
    emit urlChanged();

    this->setAttributes(QJsonObject());

#ifndef Q_OS_MAC
    static const QStringList allowedSchemas = QStringList() << QStringLiteral("http") << QStringLiteral("https");
    if(!m_url.isEmpty() && m_url.isValid() && allowedSchemas.contains(m_url.scheme(), Qt::CaseInsensitive))
    {
        HourGlass hourGlass;
        this->setStatus(Loading);

        m_webPage = new QWebEnginePage(this);
        m_webPage->setUrl(m_url);
        connect(m_webPage, &QWebEnginePage::loadFinished, this, &UrlAttributes::onWebPageLoadFinished);
    }
    else
        this->setStatus(m_url.isEmpty() ? Null : Error);
#else
    this->setAttributes(this->createDefaultAttributes());
    this->setStatus(Ready);
#endif
}

void UrlAttributes::onWebPageLoadFinished(bool ok)
{
#ifndef Q_OS_MAC
    QWebEnginePage *webPage = qobject_cast<QWebEnginePage*>(this->sender());
    if(webPage != m_webPage)
    {
        webPage->deleteLater();
        return;
    }

    if(!ok)
    {
        this->setStatus(Error);
        webPage->deleteLater();
        return;
    }

    static QString jsCode;
    if(jsCode.isEmpty())
    {
        QFile jsFile(QStringLiteral(":/misc/fetchogattribs.js"));
        if(jsFile.open(QFile::ReadOnly))
            jsCode = jsFile.readAll();
    }

    QJsonObject defaultAttribs;
    defaultAttribs.insert(QStringLiteral("url"), webPage->url().toString());
    defaultAttribs.insert(QStringLiteral("type"), QStringLiteral("website"));
    defaultAttribs.insert(QStringLiteral("title"), webPage->title());
    defaultAttribs.insert(QStringLiteral("image"), QStringLiteral(""));
    defaultAttribs.insert(QStringLiteral("description"), QStringLiteral(""));

    if(jsCode.isEmpty())
    {
        this->setAttributes(defaultAttribs);
        this->setStatus(Ready);
        return;
    }

    webPage->runJavaScript(jsCode, [this](const QVariant &result) {
        this->setAttributes(result.toJsonObject());
        this->setStatus(Ready);
    });

    /**
      Ref: https://doc.qt.io/qt-5/qwebenginepage.html#runJavaScript-3

      Documentation for runJavaScript says

      Warning: We guarantee that the callback (resultCallback) is always called,
      but it might be done during page destruction. When QWebEnginePage is deleted,
      the callback is triggered with an invalid value and it is not safe to use
      the corresponding QWebEnginePage or QWebEngineView instance inside it.

      So, lets give the web-page a few seconds time to run the JavaScript code and
      give us the result. Otherwise, we can delete the web-page to incentivise
      QWebEnginePage to run the JS for us and return the result.
      */

    QTimer *timer = new QTimer(webPage);
    timer->setInterval(500);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, webPage, &QWebEnginePage::deleteLater);
    timer->start();

    /**
      By the time the web-page deletes itself, if we have not received the result
      then we can simply emit title and webpage attribus.
      */
    connect(webPage, &QWebEnginePage::destroyed, [this,defaultAttribs]() {
        if(m_status != Ready) {
            this->setAttributes(defaultAttribs);
            this->setStatus(Ready);
        }
    });
#else
    Q_UNUSED(ok)
#endif
}

void UrlAttributes::setStatus(UrlAttributes::Status val)
{
    if(m_status == val)
        return;

    m_status = val;
    emit statusChanged();
}

void UrlAttributes::setAttributes(const QJsonObject &val)
{
    if(m_attributes == val)
        return;

    m_attributes = val;
    emit attributesChanged();
}

QJsonObject UrlAttributes::createDefaultAttributes() const
{
    QJsonObject defaultAttrs;
    defaultAttrs.insert(QStringLiteral("url"), m_url.toString());
    defaultAttrs.insert(QStringLiteral("type"), QStringLiteral("website"));
    defaultAttrs.insert(QStringLiteral("title"), QStringLiteral("Website URL"));
    defaultAttrs.insert(QStringLiteral("image"), QStringLiteral(""));
    defaultAttrs.insert(QStringLiteral("description"), QStringLiteral(""));
    return defaultAttrs;
}


