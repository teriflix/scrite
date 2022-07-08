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

#ifndef PAUSESTEP_H
#define PAUSESTEP_H

#include "automation.h"

#include <QTimer>

class PauseStep : public AbstractAutomationStep
{
    Q_OBJECT

public:
    explicit PauseStep(QObject *parent = nullptr);
    ~PauseStep();

    enum DurationType { Milliseconds = 0, Seconds };
    Q_ENUM(DurationType)
    Q_PROPERTY(DurationType durationType READ durationType WRITE setDurationType NOTIFY durationTypeChanged)
    void setDurationType(DurationType val);
    DurationType durationType() const { return m_durationType; }
    Q_SIGNAL void durationTypeChanged();
    DurationType m_durationType = Milliseconds;

    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    void setDuration(int val);
    int duration() const { return m_duration; }
    Q_SIGNAL void durationChanged();

    Q_INVOKABLE void proceed() { this->finish(); }

protected:
    // AbstractAutomationStep interface
    void run();

private:
    int m_duration = 0;
    QTimer m_timer;
};

#endif // PAUSESTEP_H

#endif // SCRITE_ENABLE_AUTOMATION
