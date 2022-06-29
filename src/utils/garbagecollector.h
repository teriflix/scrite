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

#ifndef GARBAGECOLLECTOR_H
#define GARBAGECOLLECTOR_H

#include <QObject>

#include "execlatertimer.h"

/**
 * We need a class that deletes a QObject much later than what
 * QObject::deleteLater() does for us. Thats what GarbageCollector
 * does for us.
 */

class GarbageCollector : public QObject
{
    Q_OBJECT

public:
    static GarbageCollector *instance();
    ~GarbageCollector();

    void avoidChildrenOf(QObject *parent);
    void add(QObject *ptr);

protected:
    GarbageCollector(QObject *parent = nullptr);
    void timerEvent(QTimerEvent *event);
    void onObjectDestroyed(QObject *obj);

private:
    QObjectList m_objects;
    QObjectList m_shredder;
    QObjectList m_avoidList;
    ExecLaterTimer m_timer;
};

#endif // GARBAGECOLLECTOR_H
