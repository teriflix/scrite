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

#include "announcement.h"

Q_GLOBAL_STATIC(QList<Announcement*>, Announcements);

Announcement::Announcement(QObject *parent)
    : QObject(parent)
{
    ::Announcements->append(this);
}

Announcement::~Announcement()
{
    ::Announcements->removeOne(this);
}

Announcement *Announcement::qmlAttachedProperties(QObject *object)
{
    return new Announcement(object);
}

void Announcement::shout(const QString &type, const QJsonValue &data)
{
    for(Announcement *a : *::Announcements)
    {
        if(a == this)
            continue;

        emit a->incoming(type, data);
    }
}

