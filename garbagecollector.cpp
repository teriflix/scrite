/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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

GarbageCollector *GarbageCollector::instance()
{
    static GarbageCollector *theInstance = new GarbageCollector(qApp);
    return theInstance;
}


GarbageCollector::GarbageCollector(QObject *parent)
    : QObject(parent)
{

}


GarbageCollector::~GarbageCollector()
{
    m_timer.stop();
    qDeleteAll(m_objects);
}

void GarbageCollector::add(QObject *ptr)
{
    if(ptr == nullptr || m_objects.contains(ptr))
        return;

    connect(ptr, &QObject::destroyed, this, &GarbageCollector::onObjectDestroyed);
    m_objects.append(ptr);
    m_timer.start(100, this);
}

void GarbageCollector::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_timer.timerId())
    {
        m_timer.stop();

        QObjectList copy(m_objects);
        m_objects.clear();
        qDeleteAll(copy);
    }
}

void GarbageCollector::onObjectDestroyed(QObject *obj)
{
    m_objects.removeOne(obj);
    m_timer.start(100, this);
}
