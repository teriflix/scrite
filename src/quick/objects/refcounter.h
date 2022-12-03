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

class RefCounter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(RefCounter)

public:
    explicit RefCounter(QObject *parent = nullptr) : QObject(parent) { }
    ~RefCounter() { }

    static RefCounter *qmlAttachedProperties(QObject *object) { return new RefCounter(object); }

    Q_PROPERTY(int refCount READ refCount WRITE setRefCount NOTIFY refCountChanged)
    int refCount() const
    {
        QReadLocker locker(&m_refCountLock);
        return m_refCount;
    }
    Q_SIGNAL void refCountChanged();

    Q_PROPERTY(bool isReffed READ isReffed NOTIFY refCountChanged)
    bool isReffed() const { return m_refCount > 0; }

    Q_INVOKABLE void ref() { this->setRefCount(m_refCount + 1); }
    Q_INVOKABLE bool deref()
    {
        this->setRefCount(m_refCount - 1);
        return this->isReffed();
    }
    Q_INVOKABLE bool reset()
    {
        const bool ret = m_refCount > 0;
        this->setRefCount(0);
        return ret;
    }

private:
    void setRefCount(int val)
    {
        {
            QReadLocker locker(&m_refCountLock);
            if (m_refCount == val)
                return;
        }

        {
            QWriteLocker locker(&m_refCountLock);
            m_refCount = qMax(0, val);
        }

        emit refCountChanged();
    }

private:
    int m_refCount = 0;
    mutable QReadWriteLock m_refCountLock;
};

#endif
