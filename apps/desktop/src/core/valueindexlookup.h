/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef VALUEINDEXLOOKUP_H
#define VALUEINDEXLOOKUP_H

#include <QObject>
#include <QQmlEngine>

#include "qobjectserializer.h"

class ValueIndexLookup : public QObject, public QObjectSerializer::Interface
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QObjectSerializer::Interface)

public:
    explicit ValueIndexLookup(QObject *parent = nullptr);
    virtual ~ValueIndexLookup();

    Q_INVOKABLE bool isEmpty() const { return m_lookup.isEmpty(); }

    Q_INVOKABLE int insert(const QString &value, int defaultValue = -1);
    Q_INVOKABLE int remove(const QString &value, int defaultValue = -1);
    Q_INVOKABLE int lookup(const QString &value, int defaultValue = -1) const;
    Q_INVOKABLE void prune(const QStringList &values);

    // Interface interface
    void serializeToJson(QJsonObject &json) const;
    void deserializeFromJson(const QJsonObject &json);

private:
    QMap<QString, int> m_lookup;
    int m_index = 0;
};

#endif // VALUEINDEXLOOKUP_H
