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

#ifdef SCRITE_ENABLE_AUTOMATION

#include "pausestep.h"

#include <QElapsedTimer>
#include <QEventLoop>

PauseStep::PauseStep(QObject *parent) : AbstractAutomationStep(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &PauseStep::finish);
}

PauseStep::~PauseStep() { }

void PauseStep::setDurationType(PauseStep::DurationType val)
{
    if (m_durationType == val)
        return;

    m_durationType = val;
    emit durationTypeChanged();
}

void PauseStep::setDuration(int val)
{
    if (m_duration == val)
        return;

    m_duration = val;
    emit durationChanged();
}

void PauseStep::run()
{
    if (m_duration > 0) {
        const int msecs = m_durationType == Milliseconds ? m_duration : m_duration * 1000;
        m_timer.setInterval(msecs);
        m_timer.setSingleShot(true);
        m_timer.start();
    }
}

#endif // SCRITE_ENABLE_AUTOMATION
