/****************************************************************************
**
** Copyright (C) TERIFLIX Entertainment Spaces Pvt. Ltd. Bengaluru
** Author: Prashanth N Udupa (prashanth.udupa@teriflix.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QAbstractListModel>

class PlayerApp;
class Notification;

class NotificationManager : public QAbstractListModel
{
    Q_OBJECT

public:
    static NotificationManager *instance();
    NotificationManager(QObject *parent = nullptr);
    ~NotificationManager();

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    int count() const;
    Q_SIGNAL void countChanged();

    Q_INVOKABLE Notification *notificationAt(int row) const;

    enum { NotificationRole = Qt::UserRole };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void dismissNotification(int row);

private:
    void addNotification(Notification *notification);
    void dismissNotification(Notification *notification);
    void removeNotification(Notification *notification);

private:
    friend class Notification;
    friend class PlayerApp;
    QList<Notification *> m_notifications;
};

#endif // NOTIFICATIONMANAGER_H
