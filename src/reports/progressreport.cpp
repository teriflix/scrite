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

#include "progressreport.h"

ProgressReport::ProgressReport(QObject *parent) : QObject(parent), m_proxyFor(this, "proxyFor") { }

ProgressReport::~ProgressReport()
{
    emit aboutToDelete(this);
}

void ProgressReport::setProxyFor(ProgressReport *val)
{
    if (m_proxyFor == val)
        return;

    if (m_proxyFor != nullptr) {
        disconnect(m_proxyFor, &ProgressReport::aboutToDelete, this,
                   &ProgressReport::resetProxyFor);
        disconnect(m_proxyFor, &ProgressReport::progressTextChanged, this,
                   &ProgressReport::updateProgressTextFromProxy);
        disconnect(m_proxyFor, &ProgressReport::progressChanged, this,
                   &ProgressReport::updateProgressFromProxy);
        disconnect(m_proxyFor, &ProgressReport::statusChanged, this,
                   &ProgressReport::updateStatusFromProxy);
    }

    m_proxyFor = val;

    if (m_proxyFor != nullptr) {
        connect(m_proxyFor, &ProgressReport::aboutToDelete, this, &ProgressReport::resetProxyFor);
        connect(m_proxyFor, &ProgressReport::progressTextChanged, this,
                &ProgressReport::updateProgressTextFromProxy);
        connect(m_proxyFor, &ProgressReport::progressChanged, this,
                &ProgressReport::updateProgressFromProxy);
        connect(m_proxyFor, &ProgressReport::statusChanged, this,
                &ProgressReport::updateStatusFromProxy);

        this->setProgressText(m_proxyFor->progressText());
        this->setProgress(m_proxyFor->progress());
        this->setStatus(m_proxyFor->status());
    } else {
        this->setProgressText(QString());
        this->setProgress(1.0);
        this->setStatus(NotStarted);
    }

    emit proxyForChanged();
}

void ProgressReport::setProgressText(const QString &val)
{
    if (m_progressText == val)
        return;

    m_progressText = val;
    emit progressTextChanged();
}

void ProgressReport::setStatus(ProgressReport::Status val)
{
    if (m_status == val)
        return;

    m_status = val;
    emit statusChanged();

    switch (m_status) {
    case Started:
        emit progressStarted();
        break;
    case Finished:
        emit progressFinished();
        break;
    default:
        break;
    }
}

void ProgressReport::setProgress(qreal val)
{
    val = qBound(0.0, val, 1.0);
    if (qFuzzyCompare(m_progress, val))
        return;

    m_progress = val;
    emit progressChanged();

    this->setStatus(InProgress);
}

void ProgressReport::resetProxyFor()
{
    m_proxyFor = nullptr;
    this->setProgressText(QString());
    this->setProgress(1.0);
    this->setStatus(NotStarted);
    emit proxyForChanged();
}

void ProgressReport::updateProgressTextFromProxy()
{
    if (m_proxyFor != nullptr)
        this->setProgressText(m_proxyFor->progressText());
}

void ProgressReport::updateProgressFromProxy()
{
    if (m_proxyFor != nullptr)
        this->setProgress(m_proxyFor->progress());
}

void ProgressReport::updateStatusFromProxy()
{
    if (m_proxyFor != nullptr)
        this->setStatus(m_proxyFor->status());
}

qreal ProgressReport::progressStep() const
{
    return m_progressStep;
}

void ProgressReport::setProgressStep(qreal val)
{
    if (qFuzzyCompare(m_progressStep, val))
        return;

    m_progressStep = val;
}

void ProgressReport::setProgressStepFromCount(int count)
{
    this->setProgressStep(1.0 / qreal(count));
}

void ProgressReport::tick()
{
    if (m_progressStep > 0)
        this->setProgress(m_progress + m_progressStep);
}

void ProgressReport::start()
{
    if (m_progressText.isEmpty())
        this->setProgressText("Started");
    this->setStatus(Started);
    this->setProgress(0);
}

void ProgressReport::finish()
{
    if (m_progressText.isEmpty() || m_progressText == "Started")
        this->setProgressText("Finished");
    this->setProgress(1);
    this->setStatus(Finished);
}
