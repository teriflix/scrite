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

#ifndef OBJECTHOST_H
#define OBJECTHOST_H

#include <QObject>
#include <QQuickItem>

class FeatureHost : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(FeatureHost)
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit FeatureHost(QObject *parent = nullptr);
    ~FeatureHost();
    Q_SIGNAL void aboutToDelete(FeatureHost *host);

    // clang-format off
    Q_PROPERTY(QString uri
               READ uri
               WRITE setUri
               NOTIFY uriChanged)
    // clang-format on
    void setUri(const QString &val);
    QString uri() const { return m_uri; }
    Q_SIGNAL void uriChanged();

    // clang-format off
    Q_PROPERTY(QQuickItem* item
               READ item
               CONSTANT)
    // clang-format on
    QQuickItem *item() const { return m_item; }

    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

signals:
    void featureAdded(QObject *feature);
    void featureRemoved(QObject *feature);

public:
    static FeatureHost *qmlAttachedProperties(QObject *object);
    static FeatureHost *find(const QString &uri);

private:
    static int resolveProviderHosts(FeatureHost *host);

private:
    friend class FeatureProvider;
    QString m_uri;
    QQuickItem *m_item = nullptr;
    bool m_componentComplete = true;
};

class FeatureProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit FeatureProvider(QObject *parent = nullptr);
    ~FeatureProvider();

    // clang-format off
    Q_PROPERTY(QString hostUri
               READ hostUri
               WRITE setHostUri
               NOTIFY hostUriChanged)
    // clang-format on
    void setHostUri(const QString &val);
    QString hostUri() const { return m_hostUri; }
    Q_SIGNAL void hostUriChanged();

    // clang-format off
    Q_PROPERTY(FeatureHost* host
               READ host
               NOTIFY hostChanged)
    // clang-format on
    FeatureHost *host() const { return m_host; }
    Q_SIGNAL void hostChanged();

    // clang-format off
    Q_CLASSINFO("DefaultProperty", "delegate")
    Q_PROPERTY(QQmlComponent* delegate
               READ delegate
               WRITE setDelegate
               NOTIFY delegateChanged)
    // clang-format on
    void setDelegate(QQmlComponent *val);
    QQmlComponent *delegate() const { return m_delegate; }
    Q_SIGNAL void delegateChanged();

    // clang-format off
    Q_PROPERTY(QObject* feature
               READ feature
               NOTIFY featureChanged)
    // clang-format on
    QObject *feature() const { return m_feature; }
    Q_SIGNAL void featureChanged();

private:
    bool resolveHost(FeatureHost *host);
    void hostDestroyed(FeatureHost *host);
    void featureDestroyed(QObject *obj);
    void provideFeature();
    void repealFeature();

private:
    friend class FeatureHost;
    FeatureHost *m_host = nullptr;
    QString m_hostUri;
    QObject *m_feature = nullptr;
    QQmlComponent *m_delegate = nullptr;
};

#endif // OBJECTHOST_H
