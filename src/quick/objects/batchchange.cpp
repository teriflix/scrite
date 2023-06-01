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

#include "batchchange.h"

#include <QTimerEvent>

BatchChange::BatchChange(QObject *parent) : QObject(parent) { }

BatchChange::~BatchChange() { }

void BatchChange::setTrackChangesOn(const QVariant &val)
{
    if (m_trackChangesOn == val)
        return;

    m_trackChangesOn = val;
    emit trackChangesOnChanged();

    m_timer.start(m_delay, this);
}

void BatchChange::setDelay(int val)
{
    const int val2 = qBound(10, val, 5000);
    if (m_delay == val2)
        return;

    m_delay = val2;
    emit delayChanged();
}

void BatchChange::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        m_value = m_trackChangesOn;
        emit valueChanged();
    } else
        QObject::timerEvent(event);
}
