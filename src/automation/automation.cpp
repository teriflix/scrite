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

#include "automation.h"
#include "application.h"

#include <QFileInfo>

AbstractAutomationStep::AbstractAutomationStep(QObject *parent) : QObject(parent) { }

AbstractAutomationStep::~AbstractAutomationStep() { }

void AbstractAutomationStep::start()
{
    this->clearErrorMessage();
    this->setRunning(true);
    emit started();
    this->run();
}

void AbstractAutomationStep::finish()
{
    this->setRunning(false);
    emit finished();
}

void AbstractAutomationStep::setRunning(bool val)
{
    if (m_running == val)
        return;

    m_running = val;
    emit runningChanged();
}

void AbstractAutomationStep::setErrorMessage(const QString &val)
{
    if (m_errorMessage == val)
        return;

    m_errorMessage = val;
    emit errorMessageChanged();
}

#ifdef SCRITE_ENABLE_AUTOMATION

///////////////////////////////////////////////////////////////////////////////

Automation::Automation(QObject *parent) : QObject(parent) { }

Automation::~Automation() { }

void Automation::setAutoRun(bool val)
{
    if (m_autoRun == val)
        return;

    m_autoRun = val;
    emit autoRunChanged();
}

void Automation::setQuitAppWhenFinished(bool val)
{
    if (m_quitAppWhenFinished == val)
        return;

    m_quitAppWhenFinished = val;
    emit quitAppWhenFinishedChanged();
}

void Automation::start()
{
    if (m_running)
        return;

    this->setRunning(true);

    m_pendingSteps = m_steps;
    this->startNextStep();
}

QString Automation::pathOf(const QString &absFileName) const
{
    if (absFileName.isEmpty())
        return QString();

    const QFileInfo fi(absFileName);
    return fi.absolutePath();
}

QString Automation::fileNameOf(const QString &absFileName, const QString &newSuffix) const
{
    if (absFileName.isEmpty())
        return QString();

    const QFileInfo fi(absFileName);
    if (newSuffix.isEmpty())
        return fi.fileName();

    return fi.baseName() + QStringLiteral(".") + newSuffix;
}

QQmlListProperty<AbstractAutomationStep> Automation::steps()
{
    return QQmlListProperty<AbstractAutomationStep>(
            reinterpret_cast<QObject *>(this), static_cast<void *>(this),
            &Automation::staticAppendStep, &Automation::staticStepCount, &Automation::staticStepAt,
            &Automation::staticClearSteps);
}

void Automation::addStep(AbstractAutomationStep *ptr)
{
    if (ptr == nullptr || m_steps.indexOf(ptr) >= 0)
        return;

    ptr->setParent(this);

    m_steps.append(ptr);
    emit stepCountChanged();
}

void Automation::removeStep(AbstractAutomationStep *ptr)
{
    if (ptr == nullptr)
        return;

    const int index = m_steps.indexOf(ptr);
    if (index < 0)
        return;

    if (ptr->parent() == this)
        GarbageCollector::instance()->add(ptr);

    m_steps.removeAt(index);
    emit stepCountChanged();
}

AbstractAutomationStep *Automation::stepAt(int index) const
{
    return index < 0 || index >= m_steps.size() ? nullptr : m_steps.at(index);
}

void Automation::clearSteps()
{
    while (m_steps.size())
        this->removeStep(m_steps.first());
}

void Automation::staticAppendStep(QQmlListProperty<AbstractAutomationStep> *list,
                                  AbstractAutomationStep *ptr)
{
    reinterpret_cast<Automation *>(list->data)->addStep(ptr);
}

void Automation::staticClearSteps(QQmlListProperty<AbstractAutomationStep> *list)
{
    reinterpret_cast<Automation *>(list->data)->clearSteps();
}

AbstractAutomationStep *Automation::staticStepAt(QQmlListProperty<AbstractAutomationStep> *list,
                                                 int index)
{
    return reinterpret_cast<Automation *>(list->data)->stepAt(index);
}

int Automation::staticStepCount(QQmlListProperty<AbstractAutomationStep> *list)
{
    return reinterpret_cast<Automation *>(list->data)->stepCount();
}

void Automation::classBegin() { }

void Automation::componentComplete()
{
    if (m_autoRun)
        this->start();
}

void Automation::startNextStep()
{
    AbstractAutomationStep *step = qobject_cast<AbstractAutomationStep *>(this->sender());
    if (step != nullptr)
        disconnect(step, &AbstractAutomationStep::finished, this, &Automation::startNextStep);

    if (m_pendingSteps.isEmpty()) {
        this->setRunning(false);

        if (m_quitAppWhenFinished)
            qApp->quit();

        return;
    }

    step = m_pendingSteps.takeFirst();
    connect(step, &AbstractAutomationStep::finished, this, &Automation::startNextStep);
    step->start();
}

void Automation::setRunning(bool val)
{
    if (m_running == val)
        return;

    m_running = val;
    emit runningChanged();
}

#endif // SCRITE_ENABLE_AUTOMATION
