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

#ifndef FLICKSCROLLSPEEDCONTROL_H
#define FLICKSCROLLSPEEDCONTROL_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickItem>

#include "qobjectproperty.h"

class FlickScrollSpeedControl : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(FlickScrollSpeedControl)
    QML_UNCREATABLE("Use as attached property.")

public:
    explicit FlickScrollSpeedControl(QObject *parent = nullptr);
    ~FlickScrollSpeedControl();

    static FlickScrollSpeedControl *qmlAttachedProperties(QObject *object);

    Q_PROPERTY(QQuickItem* flickable READ flickable WRITE setFlickable NOTIFY flickableChanged)
    void setFlickable(QQuickItem *val);
    QQuickItem *flickable() const { return m_flickable; }
    Q_SIGNAL void flickableChanged();

    Q_PROPERTY(qreal defaultFlickDeceleration READ defaultFlickDeceleration WRITE setDefaultFlickDeceleration NOTIFY defaultFlickDecelerationChanged)
    void setDefaultFlickDeceleration(qreal val);
    qreal defaultFlickDeceleration() const { return m_defaultFlickDeceleration; }
    Q_SIGNAL void defaultFlickDecelerationChanged();

    Q_PROPERTY(qreal defaultMaximumVelocity READ defaultMaximumVelocity WRITE setDefaultMaximumVelocity NOTIFY defaultMaximumVelocityChanged)
    void setDefaultMaximumVelocity(qreal val);
    qreal defaultMaximumVelocity() const { return m_defaultMaximumVelocity; }
    Q_SIGNAL void defaultMaximumVelocityChanged();

    Q_PROPERTY(qreal flickDecelerationFactor READ flickDecelerationFactor WRITE setFlickDecelerationFactor NOTIFY flickDecelerationFactorChanged)
    void setFlickDecelerationFactor(qreal val);
    qreal flickDecelerationFactor() const { return m_flickDecelerationFactor; }
    Q_SIGNAL void flickDecelerationFactorChanged();

    Q_PROPERTY(qreal maximumVelocityFactor READ maximumVelocityFactor WRITE setMaximumVelocityFactor NOTIFY maximumVelocityFactorChanged)
    void setMaximumVelocityFactor(qreal val);
    qreal maximumVelocityFactor() const { return m_maximumVelocityFactor; }
    Q_SIGNAL void maximumVelocityFactorChanged();

    Q_PROPERTY(qreal factor READ factor WRITE setFactor NOTIFY factorChanged)
    void setFactor(qreal val);
    qreal factor() const { return m_factor; }
    Q_SIGNAL void factorChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void resetFlickable();
    void computeValues();

private:
    QObjectProperty<QQuickItem> m_flickable;
    qreal m_defaultMaximumVelocity =
            2500; // Value of QML_FLICK_DEFAULTMAXVELOCITY in qquickflickablebehavior_p.h
    qreal m_defaultFlickDeceleration =
            1500; // Value of QML_FLICK_DEFAULTDECELERATION in qquickflickablebehavior_p.h
    qreal m_factor = 1.0;
    qreal m_maximumVelocityFactor = -1.0;
    qreal m_flickDecelerationFactor = -1.0;
    bool m_wheeling = false;
};

#endif // FLICKSCROLLSPEEDCONTROL_H
