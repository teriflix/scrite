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

#include "announcement.h"
#include "application.h"

Announcement::Announcement(QObject *parent) : QObject(parent)
{
    connect(AnnouncementBroadcast::instance(), &AnnouncementBroadcast::shout, this,
            &Announcement::hearing);
}

Announcement::~Announcement() { }

Announcement *Announcement::qmlAttachedProperties(QObject *object)
{
    return new Announcement(object);
}

void Announcement::shout(const QString &type, const QJsonValue &data)
{
    AnnouncementBroadcast::instance()->doShout(this, type, data);
}

void Announcement::hearing(Announcement *from, const QString &type, const QJsonValue &data)
{
    if (from == this)
        return;

    emit incoming(type, data);
}

///////////////////////////////////////////////////////////////////////////////

AnnouncementBroadcast *AnnouncementBroadcast::instance()
{
    static AnnouncementBroadcast *theInstance = new AnnouncementBroadcast(qApp);
    return theInstance;
}

AnnouncementBroadcast::AnnouncementBroadcast(QObject *parent) : QObject(parent) { }

AnnouncementBroadcast::~AnnouncementBroadcast() { }

void AnnouncementBroadcast::doShout(Announcement *from, const QString &type, const QJsonValue &data)
{
    emit shout(from, type, data);
}
