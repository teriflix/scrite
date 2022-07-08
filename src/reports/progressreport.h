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

#ifndef PROGRESSREPORT_H
#define PROGRESSREPORT_H

#include <QQmlEngine>

#include "qobjectproperty.h"

class ProgressReport : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Instantiation from QML not allowed.")

public:
    explicit ProgressReport(QObject *parent = nullptr);
    ~ProgressReport();
    Q_SIGNAL void aboutToDelete(ProgressReport *val);

    Q_PROPERTY(ProgressReport* proxyFor READ proxyFor WRITE setProxyFor NOTIFY proxyForChanged RESET resetProxyFor)
    void setProxyFor(ProgressReport *val);
    ProgressReport *proxyFor() const { return m_proxyFor; }
    Q_SIGNAL void proxyForChanged();

    Q_PROPERTY(QString progressText READ progressText WRITE setProgressText NOTIFY progressTextChanged)
    void setProgressText(const QString &val);
    QString progressText() const { return m_progressText; }
    Q_SIGNAL void progressTextChanged();

    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    qreal progress() const { return m_progress; }
    Q_SIGNAL void progressChanged();

    enum Status {
        NotStarted = -1,
        Started,
        InProgress,
        Finished,
    };
    Q_ENUM(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Status status() const { return m_status; }
    Q_SIGNAL void statusChanged();

    qreal progressStep() const;
    void setProgressStep(qreal val);
    void setProgressStepFromCount(int count);
    void tick();

    void start();
    void finish();

    Q_SIGNAL void progressStarted();
    Q_SIGNAL void progressFinished();

private:
    void setStatus(Status val);
    void setProgress(qreal val);
    void resetProxyFor();
    void updateProgressTextFromProxy();
    void updateProgressFromProxy();
    void updateStatusFromProxy();

private:
    Status m_status = NotStarted;
    qreal m_progress = 1.0;
    qreal m_progressStep = 0.0;
    QString m_progressText;
    QObjectProperty<ProgressReport> m_proxyFor;
};

QML_DECLARE_TYPE(ProgressReport)

#endif // PROGRESSREPORT_H
