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

#include "refcounter.h"

RefCounter::RefCounter(QObject *parent) : QObject(parent) { }

RefCounter::~RefCounter() { }

int RefCounter::refCount() const
{
    QReadLocker locker(&m_refCountLock);
    return m_refCount;
}

void RefCounter::ref()
{
    this->setRefCount(this->refCount() + 1);
}

bool RefCounter::deref()
{
    this->setRefCount(this->refCount() - 1);
    return this->isReffed();
}

bool RefCounter::reset()
{
    const bool ret = this->refCount() > 0;
    this->setRefCount(0);
    return ret;
}

void RefCounter::setRefCount(int val)
{
    const int val2 = qMax(0, val);

    {
        QReadLocker locker(&m_refCountLock);
        if (m_refCount == val2)
            return;
    }

    {
        QWriteLocker locker(&m_refCountLock);
        m_refCount = val2;
    }

    emit refCountChanged();
}
