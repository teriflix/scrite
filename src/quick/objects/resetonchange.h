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

#ifndef RESETONCHANGE_H
#define RESETONCHANGE_H

#include <QQuickItem>
#include "execlatertimer.h"

class ResetOnChange : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ResetOnChange(QQuickItem *parent = nullptr);
    ~ResetOnChange();

    Q_PROPERTY(QVariant trackChangesOn READ trackChangesOn WRITE setTrackChangesOn NOTIFY trackChangesOnChanged)
    void setTrackChangesOn(const QVariant &val);
    QVariant trackChangesOn() const { return m_trackChangesOn; }
    Q_SIGNAL void trackChangesOnChanged();

    Q_PROPERTY(QVariant from READ from WRITE setFrom NOTIFY fromChanged)
    void setFrom(const QVariant &val);
    QVariant from() const { return m_from; }
    Q_SIGNAL void fromChanged();

    Q_PROPERTY(QVariant to READ to WRITE setTo NOTIFY toChanged)
    void setTo(const QVariant &val);
    QVariant to() const { return m_to; }
    Q_SIGNAL void toChanged();

    Q_PROPERTY(QVariant value READ value NOTIFY valueChanged)
    QVariant value() const { return m_value; }
    Q_SIGNAL void valueChanged();

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

    Q_INVOKABLE void resetNow();

signals:
    void aboutToReset();
    void justReset();

private:
    void setValue(const QVariant &val);
    void reset();
    void timerEvent(QTimerEvent *te);

private:
    int m_delay = 0;
    QVariant m_to = true;
    QVariant m_from = false;
    QVariant m_value = true;
    ExecLaterTimer m_timer;
    QVariant m_trackChangesOn;
};

#endif // RESETONCHANGE_H
