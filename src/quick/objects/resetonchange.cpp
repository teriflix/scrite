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

#include "resetonchange.h"

ResetOnChange::ResetOnChange(QQuickItem *parent)
    : QQuickItem(parent), m_timer("ResetOnChange.m_timer")
{
    this->setFlag(ItemHasContents, false);
    this->setVisible(false);
}

ResetOnChange::~ResetOnChange() { }

void ResetOnChange::setTrackChangesOn(const QVariant &val)
{
    if (m_trackChangesOn == val)
        return;

    const bool resetNow = this->isEnabled() && m_trackChangesOn.isValid();
    m_trackChangesOn = val;
    emit trackChangesOnChanged();

    if (resetNow)
        this->reset();
    else
        this->setValue(m_to);
}

void ResetOnChange::setFrom(const QVariant &val)
{
    if (m_from == val)
        return;

    m_from = val;
    emit fromChanged();
}

void ResetOnChange::setTo(const QVariant &val)
{
    if (m_to == val)
        return;

    m_to = val;
    m_value = m_to;
    emit toChanged();
    emit valueChanged();
}

void ResetOnChange::setDelay(int val)
{
    if (m_delay == val)
        return;

    m_delay = val;
    emit delayChanged();
}

void ResetOnChange::resetNow()
{
    this->reset();
}

void ResetOnChange::setValue(const QVariant &val)
{
    if (m_value == val)
        return;

    m_value = val;
    emit valueChanged();
}

void ResetOnChange::reset()
{
    emit aboutToReset();
    this->setValue(m_from);
    m_timer.start(m_delay, this);
}

void ResetOnChange::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timer.timerId()) {
        m_timer.stop();
        this->setValue(m_to);
        emit justReset();
    }
}
