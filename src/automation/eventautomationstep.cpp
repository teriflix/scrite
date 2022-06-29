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

#include "eventautomationstep.h"
#include "application.h"

#include <QtTest>
#include <QEventLoop>
#include <QElapsedTimer>

EventAutomationStep::EventAutomationStep(QObject *parent)
    : AbstractAutomationStep(parent), m_window(this, "window")
{
}

EventAutomationStep::~EventAutomationStep() { }

void EventAutomationStep::setWindow(QWindow *val)
{
    if (m_window == val)
        return;

    m_window = val;
    emit windowChanged();
}

void EventAutomationStep::setDelay(int val)
{
    if (m_delay == val)
        return;

    m_delay = qBound(0, val, 1000);
    emit delayChanged();
}

void EventAutomationStep::mouseMove(qreal x, qreal y, int button, int modifiers)
{
    Q_UNUSED(button)
    Q_UNUSED(modifiers)

    if (m_window.isNull())
        return;

    QTest::mouseMove(m_window, QPointF(x, y).toPoint());
}

void EventAutomationStep::mouseClick(qreal x, qreal y, int button, int modifiers)
{
    QTest::mouseClick(m_window, Qt::MouseButton(button), Qt::KeyboardModifiers(modifiers),
                      QPointF(x, y).toPoint());
}

void EventAutomationStep::mousePress(qreal x, qreal y, int button, int modifiers)
{
    if (m_window.isNull())
        return;

    QTest::mousePress(m_window, Qt::MouseButton(button), Qt::KeyboardModifiers(modifiers),
                      QPointF(x, y).toPoint());
}

void EventAutomationStep::mouseWheel(qreal x, qreal y, int delta, int orientation, int modifiers)
{
    if (m_window.isNull())
        return;

    const QPointF pos = QPointF(x, y); // * m_window->devicePixelRatio();
    QWheelEvent event(pos, delta, Qt::NoButton, Qt::KeyboardModifiers(modifiers),
                      Qt::Orientation(orientation));
    this->notifyWindow(&event);
}

void EventAutomationStep::mouseRelease(qreal x, qreal y, int button, int modifiers)
{
    if (m_window.isNull())
        return;

    QTest::mouseRelease(m_window, Qt::MouseButton(button), Qt::KeyboardModifiers(modifiers),
                        QPointF(x, y).toPoint());
}

void EventAutomationStep::mouseDoubleClick(qreal x, qreal y, int button, int modifiers)
{
    if (m_window.isNull())
        return;

    QTest::mouseDClick(m_window, Qt::MouseButton(button), Qt::KeyboardModifiers(modifiers),
                       QPointF(x, y).toPoint());
}

void EventAutomationStep::keyPress(int key, int modifiers)
{
    QTest::keyPress(m_window, Qt::Key(key), Qt::KeyboardModifiers(modifiers));
}

void EventAutomationStep::keyRelease(int key, int modifiers)
{
    QTest::keyRelease(m_window, Qt::Key(key), Qt::KeyboardModifiers(modifiers));
}

void EventAutomationStep::keyClick(int key, int modifiers)
{
    QTest::keyClick(m_window, Qt::Key(key), Qt::KeyboardModifiers(modifiers));
}

void EventAutomationStep::keyClicks(const QString &text, int modifiers)
{
    Q_UNUSED(m_window)
    Q_UNUSED(text)
    Q_UNUSED(modifiers)
}

void EventAutomationStep::sleep(int msecs)
{
    if (msecs <= 0)
        return;

    QElapsedTimer timer;
    timer.start();

    while (1) {
        qApp->processEvents(QEventLoop::AllEvents);
        if (timer.elapsed() >= msecs)
            break;
    }
}

void EventAutomationStep::run()
{
    emit automate();
    this->finish();
}

void EventAutomationStep::resetWindow()
{
    m_window = nullptr;
    emit windowChanged();
}

void EventAutomationStep::notifyWindow(QEvent *event)
{
    Application::instance()->notify(m_window, event);
    this->sleep(m_delay);
}

#endif // SCRITE_ENABLE_AUTOMATION
