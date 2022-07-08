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

#ifndef EVENTAUTOMATIONSTEP_H
#define EVENTAUTOMATIONSTEP_H

#include "automation.h"
#include "qobjectproperty.h"

#include <QWindow>

class EventAutomationStep : public AbstractAutomationStep
{
    Q_OBJECT

public:
    explicit EventAutomationStep(QObject *parent = nullptr);
    ~EventAutomationStep();

    Q_PROPERTY(QWindow* window READ window WRITE setWindow RESET resetWindow NOTIFY windowChanged)
    void setWindow(QWindow *val);
    QWindow *window() const { return m_window; }
    Q_SIGNAL void windowChanged();

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    void setDelay(int val);
    int delay() const { return m_delay; }
    Q_SIGNAL void delayChanged();

    Q_INVOKABLE void mouseMove(qreal x, qreal y, int button = Qt::LeftButton,
                               int modifiers = Qt::NoModifier);
    Q_INVOKABLE void mouseClick(qreal x, qreal y, int button = Qt::LeftButton,
                                int modifiers = Qt::NoModifier);
    Q_INVOKABLE void mousePress(qreal x, qreal y, int button = Qt::LeftButton,
                                int modifiers = Qt::NoModifier);
    Q_INVOKABLE void mouseWheel(qreal x, qreal y, int delta, int orientation = Qt::Vertical,
                                int modifiers = Qt::NoModifier);
    Q_INVOKABLE void mouseRelease(qreal x, qreal y, int button = Qt::LeftButton,
                                  int modifiers = Qt::NoModifier);
    Q_INVOKABLE void mouseDoubleClick(qreal x, qreal y, int button = Qt::LeftButton,
                                      int modifiers = Qt::NoModifier);

    Q_INVOKABLE void keyPress(int key, int modifiers);
    Q_INVOKABLE void keyRelease(int key, int modifiers);
    Q_INVOKABLE void keyClick(int key, int modifiers);
    Q_INVOKABLE void keyClicks(const QString &text, int modifiers);

    Q_INVOKABLE void sleep(int msecs);

    Q_SIGNAL void automate();

protected:
    void run();

private:
    void resetWindow();
    void notifyWindow(QEvent *event);

private:
    int m_delay = 0;
    QObjectProperty<QWindow> m_window;
};

#endif // EVENTAUTOMATIONSTEP_H

#endif // SCRITE_ENABLE_AUTOMATION
