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

#ifndef BATCHCHANGE_H
#define BATCHCHANGE_H

#include <QQmlEngine>
#include "execlatertimer.h"

class BatchChange : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit BatchChange(QObject *parent = nullptr);
    ~BatchChange();

    // clang-format off
    Q_PROPERTY(QVariant trackChangesOn
               READ trackChangesOn
               WRITE setTrackChangesOn
               NOTIFY trackChangesOnChanged)
    // clang-format on
    void setTrackChangesOn(const QVariant &val);
    QVariant trackChangesOn() const { return m_trackChangesOn; }
    Q_SIGNAL void trackChangesOnChanged();

    // clang-format off
    Q_PROPERTY(int delay
               READ delay
               WRITE setDelay
               NOTIFY delayChanged)
    // clang-format on
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

    // clang-format off
    Q_PROPERTY(QVariant value
               READ value
               NOTIFY valueChanged)
    // clang-format on
    QVariant value() const { return m_value; }
    Q_SIGNAL void valueChanged();

private:
    void timerEvent(QTimerEvent *event);

private:
    int m_delay = 35;
    QVariant m_value;
    QVariant m_trackChangesOn;
    ExecLaterTimer m_timer;
};

#endif // BATCHCHANGE_H
