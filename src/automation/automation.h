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

#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <QUrl>
#include <QObject>
#include <QQmlParserStatus>
#include <QQmlListProperty>

class QQuickView;

class Automation;
class AbstractAutomationStep : public QObject
{
    Q_OBJECT

public:
    explicit AbstractAutomationStep(QObject *parent = nullptr);
    ~AbstractAutomationStep();

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    bool isRunning() const { return m_running; }
    Q_SIGNAL void runningChanged();

    Q_PROPERTY(bool hasError READ hasError NOTIFY errorMessageChanged)
    bool hasError() const { return !m_errorMessage.isEmpty(); }

    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    QString errorMessage() const { return m_errorMessage; }
    Q_SIGNAL void errorMessageChanged();

signals:
    void started();
    void finished();

protected:
    void start();
    void finish();

    virtual void run() = 0;

    void clearErrorMessage() { this->setErrorMessage(QString()); }
    void setRunning(bool val);
    void setErrorMessage(const QString &val);

private:
    friend class Automation;
    bool m_running = false;
    QString m_errorMessage;
};

#ifdef SCRITE_ENABLE_AUTOMATION

class Automation : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    static void init(QQuickView *qmlWindow);

    explicit Automation(QObject *parent = nullptr);
    ~Automation();

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    bool isRunning() const { return m_running; }
    Q_SIGNAL void runningChanged();

    Q_PROPERTY(bool autoRun READ isAutoRun WRITE setAutoRun NOTIFY autoRunChanged)
    void setAutoRun(bool val);
    bool isAutoRun() const { return m_autoRun; }
    Q_SIGNAL void autoRunChanged();

    Q_PROPERTY(bool quitAppWhenFinished READ isQuitAppWhenFinished WRITE setQuitAppWhenFinished NOTIFY quitAppWhenFinishedChanged)
    void setQuitAppWhenFinished(bool val);
    bool isQuitAppWhenFinished() const { return m_quitAppWhenFinished; }
    Q_SIGNAL void quitAppWhenFinishedChanged();

    Q_INVOKABLE void start();

    // Helper methods
    Q_INVOKABLE QString fromUrl(const QUrl &url) const { return url.toLocalFile(); }
    Q_INVOKABLE QString pathOf(const QString &absFileName) const;
    Q_INVOKABLE QString fileNameOf(const QString &absFileName,
                                   const QString &newSuffix = QString()) const;

    Q_CLASSINFO("DefaultProperty", "steps")
    Q_PROPERTY(QQmlListProperty<AbstractAutomationStep> steps READ steps)
    QQmlListProperty<AbstractAutomationStep> steps();
    Q_INVOKABLE void addStep(AbstractAutomationStep *ptr);
    Q_INVOKABLE void removeStep(AbstractAutomationStep *ptr);
    Q_INVOKABLE AbstractAutomationStep *stepAt(int index) const;
    Q_PROPERTY(int stepCount READ stepCount NOTIFY stepCountChanged)
    int stepCount() const { return m_steps.size(); }
    Q_INVOKABLE void clearSteps();
    Q_SIGNAL void stepCountChanged();

private:
    static void staticAppendStep(QQmlListProperty<AbstractAutomationStep> *list,
                                 AbstractAutomationStep *ptr);
    static void staticClearSteps(QQmlListProperty<AbstractAutomationStep> *list);
    static AbstractAutomationStep *staticStepAt(QQmlListProperty<AbstractAutomationStep> *list,
                                                int index);
    static int staticStepCount(QQmlListProperty<AbstractAutomationStep> *list);
    QList<AbstractAutomationStep *> m_steps, m_pendingSteps;

public:
    // QQmlParserStatus interface
    void classBegin();
    void componentComplete();

private:
    void startNextStep();
    void setRunning(bool val);

private:
    bool m_autoRun = true;
    bool m_running = false;
    bool m_quitAppWhenFinished = true;
};

#else

class QQuickView;
class Automation
{
public:
    static void init(QQuickView *qmlWindow);
};

#endif // SCRITE_ENABLE_AUTOMATION

#endif // AUTOMATION_H
