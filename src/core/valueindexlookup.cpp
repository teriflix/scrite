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

#include "valueindexlookup.h"

ValueIndexLookup::ValueIndexLookup(QObject *parent) : QObject { parent } { }

ValueIndexLookup::~ValueIndexLookup() { }

int ValueIndexLookup::insert(const QString &value, int defaultValue)
{
    if (value.isEmpty())
        return defaultValue;

    int idx = m_lookup.value(value, -1);
    if (idx < 0) {
        idx = m_index++;
        m_lookup.insert(value, idx);
    }

    return idx;
}

int ValueIndexLookup::remove(const QString &value, int defaultValue)
{
    if (value.isEmpty())
        return defaultValue;

    int idx = m_lookup.value(value, -1);
    if (idx < 0)
        return -1;

    m_lookup.remove(value);
    return idx;
}

int ValueIndexLookup::lookup(const QString &value, int defaultValue) const
{
    if (value.isEmpty())
        return defaultValue;

    return m_lookup.value(value, defaultValue);
}

void ValueIndexLookup::prune(const QStringList &values)
{
    QStringList currentValues = m_lookup.keys();
    for (const QString &value : values) {
        int cidx = currentValues.indexOf(value);
        if (cidx < 0)
            this->insert(value);
        else
            currentValues.removeAt(cidx);
    }

    for (const QString &value : currentValues)
        this->remove(value);
}

void ValueIndexLookup::serializeToJson(QJsonObject &json) const
{
    QJsonObject map;
    auto it = m_lookup.begin();
    while (it != m_lookup.end()) {
        map.insert(it.key(), it.value());
        ++it;
    }

    json.insert("#name", this->objectName());
    json.insert("#lookup", map);
    json.insert("#index", m_index);
}

void ValueIndexLookup::deserializeFromJson(const QJsonObject &json)
{
    const QJsonObject map = json.value("#lookup").toObject();
    auto it = map.begin();
    while (it != map.end()) {
        m_lookup.insert(it.key(), it.value().toInt());
        ++it;
    }
    m_index = json.value("#index").toInt();

    this->setObjectName(json.value("#name").toString());
}
