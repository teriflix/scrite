/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "basictimer.h"
#include <QList>

#ifndef QT_NO_DEBUG
Q_GLOBAL_STATIC(QList<BasicTimer*>, BasicTimerList)
#endif

BasicTimer *BasicTimer::get(int timerId)
{
#ifndef QT_NO_DEBUG
    Q_FOREACH(BasicTimer *timer, *BasicTimerList)
    {
        if(timer->timerId() == timerId)
            return timer;
    }
#endif

    return nullptr;
}

BasicTimer::BasicTimer(const QString &name)
    : QBasicTimer(), m_name(name)
{
#ifndef QT_NO_DEBUG
    BasicTimerList->append(this);
#endif
}

BasicTimer::~BasicTimer()
{
#ifndef QT_NO_DEBUG
    BasicTimerList->removeOne(this);
#endif
}
