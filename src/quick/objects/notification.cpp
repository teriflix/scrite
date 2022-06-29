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

#include "notification.h"
#include "notificationmanager.h"

#include <QEvent>
#include <QtDebug>

Notification::Notification(QObject *parent)
    : QObject(parent), m_autoCloseTimer("Notification.m_autoCloseTimer")
{
}

Notification::~Notification()
{
    if (NotificationManager::instance())
        NotificationManager::instance()->removeNotification(this);
}

Notification *Notification::qmlAttachedProperties(QObject *object)
{
    return new Notification(object);
}

void Notification::setTitle(const QString &val)
{
    if (m_title == val)
        return;

    m_title = val;
    emit titleChanged();
}

void Notification::setText(const QString &val)
{
    if (m_text == val)
        return;

    m_text = val;
    emit textChanged();
}

void Notification::setColor(const QColor &val)
{
    if (m_color == val)
        return;

    m_color = val;
    emit colorChanged();
}

void Notification::setTextColor(const QColor &val)
{
    if (m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();
}

void Notification::setImage(const QUrl &val)
{
    if (m_image == val)
        return;

    m_image = val;
    emit imageChanged();
}

void Notification::setActive(bool val)
{
    if (m_active == val)
        return;

    m_active = val;
    if (val) {
        NotificationManager::instance()->addNotification(this);
        if (m_autoClose && m_buttons.isEmpty())
            m_autoCloseTimer.start(m_autoCloseDelay, this);
    } else {
        NotificationManager::instance()->removeNotification(this);
        emit dismissed();
        m_autoCloseTimer.stop();
    }

    emit activeChanged();
}

void Notification::setAutoClose(bool val)
{
    if (m_autoClose == val)
        return;

    m_autoClose = val;
    if (m_active)
        m_autoCloseTimer.start(m_autoCloseDelay, this);
    else
        m_autoCloseTimer.stop();

    emit autoCloseChanged();
}

void Notification::setAutoCloseDelay(int val)
{
    if (m_autoCloseDelay == val)
        return;

    m_autoCloseDelay = val;
    if (m_autoCloseTimer.isActive())
        m_autoCloseTimer.start(m_autoCloseDelay, this);

    emit autoCloseDelayChanged();
}

void Notification::setButtons(const QStringList &val)
{
    if (m_buttons == val)
        return;

    m_buttons = val;
    emit buttonsChanged();
}

void Notification::notifyButtonClick(int index)
{
    if (!m_active)
        return;

    if (index < 0 || index >= m_buttons.size())
        return;

    emit buttonClicked(index);

    this->setActive(false);
}

void Notification::notifyImageClick()
{
    if (!m_image.isEmpty()) {
        emit imageClicked();

        this->setActive(false);
    }
}

void Notification::doAutoClose()
{
    if (m_active && m_autoClose)
        this->setActive(false);
}

void Notification::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_autoCloseTimer.timerId()) {
        m_autoCloseTimer.stop();
        this->doAutoClose();
    }
}
