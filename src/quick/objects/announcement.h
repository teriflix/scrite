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

#ifndef ANNOUNCEMENT_H
#define ANNOUNCEMENT_H

#include <QObject>
#include <QQmlEngine>

class Announcement : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use as attached property.")
    QML_ATTACHED(Announcement)

public:
    explicit Announcement(QObject *parent = nullptr);
    ~Announcement();

    static Announcement *qmlAttachedProperties(QObject *object);

    Q_INVOKABLE void shout(const QString &type, const QJsonValue &data);
    Q_SIGNAL void incoming(const QString &type, const QJsonValue &data);

private:
    void hearing(Announcement *from, const QString &type, const QJsonValue &data);
};

class AnnouncementBroadcast : public QObject
{
    Q_OBJECT

public:
    static AnnouncementBroadcast *instance();
    ~AnnouncementBroadcast();

    void doShout(Announcement *from, const QString &type, const QJsonValue &data);
    Q_SIGNAL void shout(Announcement *from, const QString &type, const QJsonValue &data);

private:
    AnnouncementBroadcast(QObject *parent = nullptr);
};

#endif // ANNOUNCEMENT_H
