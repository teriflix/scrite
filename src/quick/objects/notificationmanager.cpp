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

#include "notificationmanager.h"
#include "notification.h"
#include "application.h"

#include <QtDebug>

static NotificationManager *theInstance = nullptr;

NotificationManager *NotificationManager::instance()
{
    if (::theInstance == nullptr)
        new NotificationManager(qApp);
    return ::theInstance;
}

NotificationManager::NotificationManager(QObject *parent) : QAbstractListModel(parent)
{
    ::theInstance = this;
}

NotificationManager::~NotificationManager()
{
    ::theInstance = nullptr;
}

int NotificationManager::count() const
{
    return m_notifications.size();
}

Notification *NotificationManager::notificationAt(int row) const
{
    if (row < 0 || row >= m_notifications.size())
        return nullptr;

    return m_notifications.at(row);
}

int NotificationManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_notifications.size();
}

QVariant NotificationManager::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_notifications.size())
        return QVariant();

    if (role != NotificationRole)
        return QVariant();

    Notification *notification = m_notifications.at(index.row());
    QObject *notificationObject = qobject_cast<QObject *>(notification);
    return QVariant::fromValue<QObject *>(notificationObject);
}

QHash<int, QByteArray> NotificationManager::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NotificationRole] = "notificiation";
    return roles;
}

void NotificationManager::dismissNotification(int row)
{
    if (row < 0 || row >= m_notifications.size())
        return;

    Notification *notification = m_notifications.at(row);

    // this->removeNotification(notification);, will be called by setActive(false) of Notification
    notification->setActive(false);
}

void NotificationManager::addNotification(Notification *notification)
{
    if (notification == nullptr || m_notifications.contains(notification))
        return;

    const int size = m_notifications.size();
    this->beginInsertRows(QModelIndex(), size, size);
    m_notifications.append(notification);
    this->endInsertRows();

    emit countChanged();
}

void NotificationManager::removeNotification(Notification *notification)
{
    int row = m_notifications.indexOf(notification);
    if (row < 0)
        return;

    this->beginRemoveRows(QModelIndex(), row, row);
    m_notifications.takeAt(row);
    this->endRemoveRows();

    emit countChanged();
}
