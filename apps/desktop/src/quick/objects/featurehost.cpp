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

#include "featurehost.h"
#include <QQmlEngine>
#include <QTimer>

Q_GLOBAL_STATIC(QList<FeatureHost *>, FeatureHosts)
Q_GLOBAL_STATIC(QList<FeatureProvider *>, FeatureProviders)

FeatureHost::FeatureHost(QObject *parent)
    : QObject(parent), m_item(qobject_cast<QQuickItem *>(parent))
{
    ::FeatureHosts->append(this);
}

FeatureHost::~FeatureHost()
{
    emit aboutToDelete(this);
    ::FeatureHosts->removeOne(this);
}

void FeatureHost::setUri(const QString &val)
{
    if (m_uri == val || !m_uri.isEmpty() || find(val) != nullptr)
        return;

    m_uri = val;
    emit uriChanged();

    resolveProviderHosts(this);
}

void FeatureHost::classBegin()
{
    m_componentComplete = false;
}

void FeatureHost::componentComplete()
{
    m_componentComplete = true;

    resolveProviderHosts(this);
}

FeatureHost *FeatureHost::qmlAttachedProperties(QObject *object)
{
    return new FeatureHost(object);
}

FeatureHost *FeatureHost::find(const QString &uri)
{
    for (FeatureHost *host : std::as_const(*::FeatureHosts)) {
        if (host->uri() == uri)
            return host;
    }

    return nullptr;
}

int FeatureHost::resolveProviderHosts(FeatureHost *host)
{
    int count = 0;

    if (host->m_componentComplete) {
        if (host != nullptr && !host->uri().isEmpty()) {
            for (FeatureProvider *provider : std::as_const(*::FeatureProviders)) {
                if (provider->resolveHost(host))
                    ++count;
            }
        }
    }

    return count;
}

///////////////////////////////////////////////////////////////////////////////

FeatureProvider::FeatureProvider(QObject *parent) : QObject(parent)
{
    connect(this, &FeatureProvider::hostChanged, this, &FeatureProvider::provideFeature);
    connect(this, &FeatureProvider::delegateChanged, this, &FeatureProvider::repealFeature);
    connect(this, &FeatureProvider::delegateChanged, this, &FeatureProvider::provideFeature);

    ::FeatureProviders->append(this);
}

FeatureProvider::~FeatureProvider()
{
    this->repealFeature();

    ::FeatureProviders->removeOne(this);
}

void FeatureProvider::setHostUri(const QString &val)
{
    // Target names are set-once
    if (m_hostUri == val || !m_hostUri.isEmpty())
        return;

    m_hostUri = val;
    emit hostUriChanged();

    for (FeatureHost *host : std::as_const(*::FeatureHosts)) {
        if (host->uri() == m_hostUri) {
            if (resolveHost(host))
                break;
        }
    }
}

void FeatureProvider::setDelegate(QQmlComponent *val)
{
    if (m_delegate == val)
        return;

    m_delegate = val;
    emit delegateChanged();
}

bool FeatureProvider::resolveHost(FeatureHost *host)
{
    if (m_host != nullptr || m_hostUri.isEmpty())
        return false;

    if (host == nullptr)
        host = FeatureHost::find(m_hostUri);

    if (host != nullptr && host->uri() == m_hostUri) {
        m_host = host;
        connect(m_host, &FeatureHost::aboutToDelete, this, &FeatureProvider::hostDestroyed);
        emit hostChanged();
        return true;
    }

    return false;
}

void FeatureProvider::hostDestroyed(FeatureHost *host)
{
    if (host == m_host) {
        m_host = nullptr;
        emit hostChanged();
    }
}

void FeatureProvider::featureDestroyed(QObject *obj)
{
    if (m_feature == obj) {
        m_feature = nullptr;
        emit featureChanged();
    }
}

void FeatureProvider::provideFeature()
{
    if (m_host != nullptr && m_feature == nullptr && m_delegate != nullptr) {
        QQmlContext *context = QQmlEngine::contextForObject(this);
        m_feature = m_delegate->create(context);
        if (m_feature != nullptr) {
            QQuickItem *qmlItem = qobject_cast<QQuickItem *>(m_feature);
            if (qmlItem) {
                QQuickItem *parentItem = m_host->item();
                if (parentItem)
                    qmlItem->setParentItem(parentItem);
            }
            m_host->featureAdded(m_feature);

            connect(m_host, &FeatureHost::aboutToDelete, m_feature, &QObject::deleteLater);
            connect(m_feature, &QObject::destroyed, this, &FeatureProvider::featureDestroyed);
            emit featureChanged();
        }
    }
}

void FeatureProvider::repealFeature()
{
    if (m_host != nullptr && m_feature != nullptr) {
        m_host->featureRemoved(m_feature);
        m_feature->deleteLater();
        m_feature = nullptr;
    }
}
