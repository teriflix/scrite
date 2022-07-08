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

#ifndef PROPERTYALIAS_H
#define PROPERTYALIAS_H

#include <QQmlEngine>
#include "qobjectproperty.h"

class PropertyAlias : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit PropertyAlias(QObject *parent = nullptr);
    ~PropertyAlias();

    Q_PROPERTY(QObject* sourceObject READ sourceObject WRITE setSourceObject NOTIFY sourceObjectChanged)
    void setSourceObject(QObject *val);
    QObject *sourceObject() const { return m_sourceObject; }
    Q_SIGNAL void sourceObjectChanged();

    Q_PROPERTY(QByteArray sourceProperty READ sourceProperty WRITE setSourceProperty NOTIFY sourcePropertyChanged)
    void setSourceProperty(const QByteArray &val);
    QByteArray sourceProperty() const { return m_sourceProperty; }
    Q_SIGNAL void sourcePropertyChanged();

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    void setValue(const QVariant &val);
    QVariant value() const;
    Q_SIGNAL void valueChanged();

private:
    void connectToSource();
    void disconnectFromSource();

private:
    QByteArray m_sourceProperty;
    QMetaMethod m_sourcePropertyNotify;
    QObjectProperty<QObject> m_sourceObject;
    QMetaObject::Connection m_sourceConnection;
};

#endif // PROPERTYALIAS_H
