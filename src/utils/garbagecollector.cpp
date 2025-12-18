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

#include "garbagecollector.h"

#include <QCoreApplication>

Q_GLOBAL_STATIC(GarbageCollector *, TheGarbageCollector)

GarbageCollector *GarbageCollector::instance()
{
    if (*TheGarbageCollector == nullptr)
        *TheGarbageCollector = new GarbageCollector(qApp);
    return *TheGarbageCollector;
}

GarbageCollector::GarbageCollector(QObject *parent) : QObject(parent) { }

GarbageCollector::~GarbageCollector() { }

void GarbageCollector::avoidChildrenOf(QObject *parent)
{
    Q_UNUSED(parent)
}

void GarbageCollector::add(QObject *ptr)
{
    if (ptr)
        ptr->deleteLater();
}
