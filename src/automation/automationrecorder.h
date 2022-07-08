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

#ifndef AUTOMATIONRECORDER_H
#define AUTOMATIONRECORDER_H

#include <QObject>
#include <QPoint>

#include "execlatertimer.h"

class AutomationRecorder : public QObject
{
    Q_OBJECT

public:
    explicit AutomationRecorder(QObject *parent = nullptr);
    ~AutomationRecorder();

    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    bool isRecording() const { return m_recording; }
    Q_SIGNAL void recordingChanged();

    void startRecording();
    void stopRecording();

protected:
    void timerEvent(QTimerEvent *te);
    bool eventFilter(QObject *object, QEvent *event);

private:
    void setRecording(bool val);
    void toggleRecording();
    void toggleRecordingLater();

private:
    QPoint m_pressPos;
    bool m_recording = false;
    ExecLaterTimer m_timer;
    QVariantList m_recordedStatements;
    qint64 m_lastRecordedStatementTimestamp = 0;
};

#endif // AUTOMATIONRECORDER_H

#endif // #ifdef SCRITE_ENABLE_AUTOMATION
