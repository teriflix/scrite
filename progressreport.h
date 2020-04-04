/****************************************************************************
**
** Copyright (C) Prashanth Udupa, Bengaluru
** Email: prashanth.udupa@gmail.com
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

#include <QObject>

class ProgressReport : public QObject
{
    Q_OBJECT

public:
    ProgressReport(QObject *parent=nullptr);
    ~ProgressReport();
    Q_SIGNAL void aboutToDelete(ProgressReport *val);

    Q_PROPERTY(ProgressReport* proxyFor READ proxyFor WRITE setProxyFor NOTIFY proxyForChanged)
    void setProxyFor(ProgressReport* val);
    ProgressReport* proxyFor() const { return m_proxyFor; }
    Q_SIGNAL void proxyForChanged();

    Q_PROPERTY(QString progressText READ progressText WRITE setProgressText NOTIFY progressTextChanged)
    void setProgressText(const QString &val);
    QString progressText() const { return m_progressText; }
    Q_SIGNAL void progressTextChanged();

    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    qreal progress() const { return m_progress; }
    Q_SIGNAL void progressChanged();

    qreal progressStep() const;
    void setProgressStep(qreal val);
    void setProgressStepFromCount(int count);
    void tick();

    void start() {
        this->setProgress(0);
        if(m_progressText.isEmpty())
            this->setProgressText("Started");
    }

    void finish() {
        this->setProgress(1);
        if(m_progressText.isEmpty() || m_progressText == "Started")
            this->setProgressText("Finished");
    }

    Q_SIGNAL void progressStarted();
    Q_SIGNAL void progressFinished();

private:
    void setProgress(qreal val);
    void clearProxyFor();
    void updateProgressTextFromProxy();
    void updateProgressFromProxy();

private:
    qreal m_progress;
    qreal m_progressStep;
    QString m_progressText;
    ProgressReport* m_proxyFor;
};

#endif // PROGRESSREPORT_H
