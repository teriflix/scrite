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

#include "execlatertimer.h"
#include "application.h"

#include <QList>
#include <QThread>

#ifndef QT_NO_DEBUG_OUTPUT
Q_GLOBAL_STATIC(QList<ExecLaterTimer *>, ExecLaterTimerList)
#endif

ExecLaterTimer *ExecLaterTimer::get(int timerId)
{
#ifndef QT_NO_DEBUG_OUTPUT
    for (ExecLaterTimer *timer : qAsConst(*ExecLaterTimerList)) {
        if (timer->timerId() == timerId)
            return timer;
    }
#else
    Q_UNUSED(timerId)
#endif

    return nullptr;
}

ExecLaterTimer::ExecLaterTimer(const QString &name, QObject *parent) : QObject(parent), m_name(name)
{
#ifndef QT_NO_DEBUG_OUTPUT
    ExecLaterTimerList->append(this);
#endif

    m_timer.setObjectName("ExecLaterTimer");
    m_timer.setSingleShot(!m_repeat);
    connect(&m_timer, &QTimer::timeout, this, &ExecLaterTimer::onTimeout);
}

ExecLaterTimer::~ExecLaterTimer()
{
    m_destroyed = true;
    this->stop();

#ifndef QT_NO_DEBUG_OUTPUT
    ExecLaterTimerList->removeOne(this);
#endif
}

void ExecLaterTimer::setName(const QString &val)
{
    if (m_name == val)
        return;

    m_name = val;
    emit nameChanged();
}

void ExecLaterTimer::setRepeat(bool val)
{
    if (m_repeat == val)
        return;

    m_repeat = val;
    m_timer.setSingleShot(!m_repeat);
    emit repeatChanged();
}

void ExecLaterTimer::start(int msec, QObject *object)
{
    if (m_timer.isActive())
        this->stop();

    if (object == nullptr || m_destroyed)
        return;

    if (object != m_object) {
        if (object)
            disconnect(object, &QObject::destroyed, this, &ExecLaterTimer::onObjectDestroyed);

        m_object = object;

        if (m_object)
            connect(object, &QObject::destroyed, this, &ExecLaterTimer::onObjectDestroyed);
    }

    if (this->thread() != nullptr && this->thread()->eventDispatcher() != nullptr) {
        m_timer.start(msec);
        m_timerId = m_timer.timerId();
    } else
        m_timerId = -1;
}

void ExecLaterTimer::stop()
{
    m_timer.stop();
    m_timerId = -1;
}

void ExecLaterTimer::discardCall(const char *givenName, QObject *receiver)
{
    if (receiver == nullptr)
        receiver = qApp;

    const QString name = QLatin1String(givenName);
    QTimer *timer = name.isEmpty()
            ? nullptr
            : receiver->findChild<QTimer *>(name, Qt::FindDirectChildrenOnly);
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }
}

void ExecLaterTimer::call(const char *givenName, QObject *receiver,
                          const std::function<void()> &func, int timeout)
{
    if (receiver == nullptr)
        receiver = qApp;

    const QString name = QLatin1String(givenName);
    QTimer *timer = name.isEmpty()
            ? nullptr
            : receiver->findChild<QTimer *>(name, Qt::FindDirectChildrenOnly);
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }

    timer = new QTimer(receiver);
    timer->setObjectName(name);
    timer->setInterval(timeout);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, receiver, [=]() {
        func();
        timer->deleteLater();
    });
    timer->start();
}

void ExecLaterTimer::call(const char *name, const std::function<void()> &func, int timeout)
{
    ExecLaterTimer::call(name, qApp, func, timeout);
}

void ExecLaterTimer::onTimeout()
{
    if (m_object != nullptr && m_timerId >= 0) {
#ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << "Posting Timer [" << m_name << "]." << m_timerId << " to " << m_object;
#endif
        qApp->postEvent(m_object, new QTimerEvent(m_timerId));
    }
}

void ExecLaterTimer::onObjectDestroyed(QObject *ptr)
{
    if (m_object == ptr) {
        m_object = nullptr;
        this->stop();
    }
}
