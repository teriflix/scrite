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
#include "application.h"
#include "callgraph.h"

Q_GLOBAL_STATIC(GarbageCollector *, TheGarbageCollector)

GarbageCollector *GarbageCollector::instance()
{
    // CAPTURE_FIRST_CALL_GRAPH;
    if (*TheGarbageCollector == nullptr)
        *TheGarbageCollector = new GarbageCollector(qApp);
    return *TheGarbageCollector;
}

GarbageCollector::GarbageCollector(QObject *parent)
    : QObject(parent), m_timer("GarbageCollector.m_timer")
{
    // CAPTURE_CALL_GRAPH;
}

GarbageCollector::~GarbageCollector()
{
    m_timer.stop();
    qDeleteAll(m_objects);
    *TheGarbageCollector = nullptr;
}

void GarbageCollector::avoidChildrenOf(QObject *parent)
{
    if (TheGarbageCollector == nullptr)
        return;

    if (parent != nullptr) {
        disconnect(parent, &QObject::destroyed, this, &GarbageCollector::onObjectDestroyed);
        m_avoidList.append(parent);
    }
}

void GarbageCollector::add(QObject *ptr)
{
    if (TheGarbageCollector == nullptr)
        return;

    if (ptr == nullptr || m_objects.contains(ptr) || m_shredder.contains(ptr))
        return;

    if (m_avoidList.contains(ptr->parent()))
        return;

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Adding to Garbage Collector: " << ptr;
#endif

    connect(ptr, &QObject::destroyed, this, &GarbageCollector::onObjectDestroyed);
    m_objects.append(ptr);
    m_timer.start(100, this);
}

void GarbageCollector::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        m_shredder = m_objects;
        m_objects.clear();

        while (!m_shredder.isEmpty()) {
            QDeferredDeleteEvent dde;
            Application::instance()->sendEvent(m_shredder.takeFirst(), &dde);
        }
    }
}

void GarbageCollector::onObjectDestroyed(QObject *obj)
{
    if (m_objects.removeOne(obj))
        m_timer.start(100, this);

    m_shredder.removeOne(obj);
    m_avoidList.removeOne(obj);
}
