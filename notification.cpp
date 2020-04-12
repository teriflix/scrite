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

#include "notification.h"
#include "notificationmanager.h"
#include "logger.h"

#include <QEvent>

Notification::Notification(QObject *parent)
    : QObject(parent)
{

}

Notification::~Notification()
{
    if(NotificationManager::instance())
        NotificationManager::instance()->removeNotification(this);
}

Notification *Notification::qmlAttachedProperties(QObject *object)
{
    return new Notification(object);
}

void Notification::setTitle(const QString &val)
{
    if(m_title == val)
        return;

    m_title = val;
    emit titleChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "title");
}

void Notification::setText(const QString &val)
{
    if(m_text == val)
        return;

    m_text = val;
    emit textChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "text");
}

void Notification::setColor(const QColor &val)
{
    if(m_color == val)
        return;

    m_color = val;
    emit colorChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "color");
}

void Notification::setTextColor(const QColor &val)
{
    if(m_textColor == val)
        return;

    m_textColor = val;
    emit textColorChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "textColor");
}

void Notification::setActive(bool val)
{
    if(m_active == val)
        return;

    m_active = val;
    if(val)
    {
        NotificationManager::instance()->addNotification(this);
        if( m_autoClose && m_buttons.isEmpty() )
            m_autoCloseTimer.start(m_autoCloseDelay, this);
    }
    else
    {
        NotificationManager::instance()->removeNotification(this);
        emit dismissed();
        m_autoCloseTimer.stop();
    }

    emit activeChanged();

    Logger::qtPropertyInfo(this, "active");
    if(m_active)
    {
        Logger::qtPropertyInfo(this, "title");
        Logger::qtPropertyInfo(this, "text");
        Logger::qtPropertyInfo(this, "buttons");
        Logger::qtPropertyInfo(this, "autoClose");
        Logger::qtPropertyInfo(this, "autoCloseDelay");
        Logger::qtPropertyInfo(this, "color");
        Logger::qtPropertyInfo(this, "textColor");
    }
}

void Notification::setAutoClose(bool val)
{
    if(m_autoClose == val)
        return;

    m_autoClose = val;
    if(m_active)
        m_autoCloseTimer.start(m_autoCloseDelay, this);
    else
        m_autoCloseTimer.stop();

    emit autoCloseChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "autoClose");
}

void Notification::setAutoCloseDelay(int val)
{
    if(m_autoCloseDelay == val)
        return;

    m_autoCloseDelay = val;
    if(m_autoCloseTimer.isActive())
        m_autoCloseTimer.start(m_autoCloseDelay, this);

    emit autoCloseDelayChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "autoCloseDelay");
}

void Notification::setButtons(const QStringList &val)
{
    if(m_buttons == val)
        return;

    m_buttons = val;
    emit buttonsChanged();

    if(m_active)
        Logger::qtPropertyInfo(this, "buttons");
}

void Notification::notifyButtonClick(int index)
{
    if( !m_active )
    {
        Logger::qtInfo(this, "Attempting to click on a notification button when not active.");
        return;
    }

    if( index < 0 || index >= m_buttons.size() )
    {
        Logger::qtInfo(this, QString("Invalid button index set for notifyButtonClick(%1). Max = %2").arg(index).arg(m_buttons.size()-1));
        return;
    }

    Logger::qtInfo(this, QString("Buttion '%1' was clicked.").arg(m_buttons.at(index)));
    emit buttonClicked(index);

    this->setActive(false);
}

void Notification::doAutoClose()
{   
    if(m_active && m_autoClose)
    {
        Logger::qtPropertyInfo(this, "doAutoClose");
        this->setActive(false);
    }
}

void Notification::timerEvent(QTimerEvent *te)
{
    if(te->timerId() == m_autoCloseTimer.timerId())
    {
        m_autoCloseTimer.stop();
        this->doAutoClose();
    }
}
