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

#ifndef ANNOUNCEMENT_H
#define ANNOUNCEMENT_H

#include <QObject>
#include <QQmlEngine>

class Announcement : public QObject
{
    Q_OBJECT

public:
    Announcement(QObject *parent=nullptr);
    ~Announcement();

    static Announcement *qmlAttachedProperties(QObject *object);

    Q_INVOKABLE void shout(const QString &type, const QJsonValue &data);
    Q_SIGNAL void incoming(const QString &type, const QJsonValue &data);
};
Q_DECLARE_METATYPE(Announcement*)
QML_DECLARE_TYPEINFO(Announcement, QML_HAS_ATTACHED_PROPERTIES)

#endif // ANNOUNCEMENT_H
