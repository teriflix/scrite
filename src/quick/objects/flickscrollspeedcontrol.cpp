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

#include "flickscrollspeedcontrol.h"
#include "execlatertimer.h"
#include "application.h"

#include <QQmlProperty>

FlickScrollSpeedControl::FlickScrollSpeedControl(QObject *parent)
    : QObject(parent), m_flickable(this, "flickable")
{
    if (parent->inherits("QQuickFlickable")) {
        m_flickable = qobject_cast<QQuickItem *>(parent);
        m_flickable->installEventFilter(this);
        this->computeValues();
    }
}

FlickScrollSpeedControl::~FlickScrollSpeedControl() { }

FlickScrollSpeedControl *FlickScrollSpeedControl::qmlAttachedProperties(QObject *object)
{
    return new FlickScrollSpeedControl(object);
}

void FlickScrollSpeedControl::setFlickable(QQuickItem *val)
{
    if (m_flickable == val)
        return;

    if (!m_flickable.isNull())
        m_flickable->removeEventFilter(this);

    m_flickable = val;

    if (!m_flickable.isNull())
        m_flickable->installEventFilter(this);

    emit flickableChanged();

    this->computeValues();
}

void FlickScrollSpeedControl::setDefaultFlickDeceleration(qreal val)
{
    if (m_defaultFlickDeceleration == val)
        return;

    m_defaultFlickDeceleration = val;
    emit defaultFlickDecelerationChanged();

    this->computeValues();
}

void FlickScrollSpeedControl::setDefaultMaximumVelocity(qreal val)
{
    if (m_defaultMaximumVelocity == val)
        return;

    m_defaultMaximumVelocity = val;
    emit defaultMaximumVelocityChanged();

    this->computeValues();
}

void FlickScrollSpeedControl::setFlickDecelerationFactor(qreal val)
{
    if (qFuzzyCompare(m_flickDecelerationFactor, val))
        return;

    m_flickDecelerationFactor = val;
    emit flickDecelerationFactorChanged();

    this->computeValues();
}

void FlickScrollSpeedControl::setMaximumVelocityFactor(qreal val)
{
    if (qFuzzyCompare(m_maximumVelocityFactor, val))
        return;

    m_maximumVelocityFactor = val;
    emit maximumVelocityFactorChanged();

    this->computeValues();
}

void FlickScrollSpeedControl::setFactor(qreal val)
{
    if (qFuzzyCompare(m_factor, val))
        return;

    m_factor = val;
    emit factorChanged();

    this->computeValues();
}

bool FlickScrollSpeedControl::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

#ifndef Q_OS_MAC
    if (m_flickable.isNull())
        return false;

    const bool wheeling = (event->type() == QEvent::Wheel);
    if (wheeling != m_wheeling) {
        m_wheeling = wheeling;
        this->computeValues();
    }
#else
    Q_UNUSED(event)
#endif

    return false;
}

void FlickScrollSpeedControl::computeValues()
{
#ifndef Q_OS_MAC
    if (m_flickable.isNull())
        return;

    QQmlProperty flickDecelerationProp(m_flickable, "flickDeceleration");
    QQmlProperty maximumFlickVelocityProp(m_flickable, "maximumFlickVelocity");

    if (m_wheeling) {
        flickDecelerationProp.write(10000);
    } else {
        const qreal mv = m_defaultMaximumVelocity
                * (m_maximumVelocityFactor < 0 ? m_factor : m_maximumVelocityFactor);
        const qreal fd = m_defaultFlickDeceleration
                * (m_flickDecelerationFactor < 0 ? m_factor : m_flickDecelerationFactor);

        flickDecelerationProp.write(fd);
        maximumFlickVelocityProp.write(mv);
    }
#endif
}
