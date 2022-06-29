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

#ifndef QOBJECTSERIALIZER_H
#define QOBJECTSERIALIZER_H

#include <QMap>
#include <QObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include "qobjectfactory.h"

namespace QObjectSerializer {
class Helper
{
public:
    virtual ~Helper();
    virtual bool canHandle(int type) const = 0;
    virtual QJsonValue toJson(const QVariant &value) const = 0;
    virtual QVariant fromJson(const QJsonValue &value, int type) const = 0;
};
void registerHelper(Helper *helper);

class Interface
{
public:
    virtual ~Interface();
    virtual void prepareForSerialization() { }
    virtual void prepareForDeserialization() { }
    virtual bool canSerialize(const QMetaObject *, const QMetaProperty &) const { return true; }
    virtual void serializeToJson(QJsonObject &) const { }
    virtual void deserializeFromJson(const QJsonObject &) { }

    virtual bool canSetPropertyFromObjectList(const QString & /*propName*/) const { return false; }
    virtual void setPropertyFromObjectList(const QString & /*propName*/,
                                           const QList<QObject *> & /*objects*/)
    {
    }
};

QString toJsonString(const QObject *object);
bool fromJsonString(const QString &json, QObject *object, QObjectFactory *factory = nullptr);

QJsonObject toJson(const QObject *object);
bool fromJson(const QJsonObject &json, QObject *object, QObjectFactory *factory = nullptr);

QVariantMap cacheDefaultPropertyValues(const QObject *object, bool readonly = false);
};

#define CACHE_DEFAULT_PROPERTY_VALUES                                                              \
    static bool defaultPropertyValuesCached = false;                                               \
    if (!defaultPropertyValuesCached) {                                                            \
        QObjectSerializer::cacheDefaultPropertyValues(this);                                       \
        defaultPropertyValuesCached = true;                                                        \
    }

Q_DECLARE_INTERFACE(QObjectSerializer::Interface,
                    "com.prashanthudupa.QObjectSerializer.Interface/1.0")

#endif // QOBJECTSERIALIZER_H
