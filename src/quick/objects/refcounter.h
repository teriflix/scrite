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

#ifndef REFCOUNTER_H
#define REFCOUNTER_H

#include <QQmlEngine>
#include <QReadWriteLock>

#include "application.h"

class RefCounter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(RefCounter)

public:
    explicit RefCounter(QObject *parent = nullptr);
    ~RefCounter();

    static RefCounter *qmlAttachedProperties(QObject *object) { return new RefCounter(object); }

    Q_PROPERTY(int refCount READ refCount WRITE setRefCount NOTIFY refCountChanged)
    int refCount() const;
    Q_SIGNAL void refCountChanged();

    Q_PROPERTY(bool isReffed READ isReffed NOTIFY refCountChanged)
    bool isReffed() const { return this->refCount() > 0; }

    Q_INVOKABLE void ref();
    Q_INVOKABLE bool deref();
    Q_INVOKABLE bool reset();

private:
    void setRefCount(int val);

private:
    int m_refCount = 0;
    mutable QReadWriteLock m_refCountLock;
};

#endif
