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

#include "qobjectserializer.h"
#include "timeprofiler.h"

#include <QtDebug>
#include <QStack>
#include <QColor>
#include <QMetaType>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QJsonDocument>
#include <QQmlListProperty>
#include <QQmlListReference>

#ifdef QT_WIDGETS_LIB
#include <QGraphicsObject>
#endif

#include <QFont>
#include <QMargins>
#include <QMarginsF>

// #define SERIALIZE_DYNAMIC_PROPERTIES

class QMarginsFHelper : public QObjectSerializer::Helper
{
public:
    explicit QMarginsFHelper();
    ~QMarginsFHelper() override;

    bool canHandle(int type) const override;
    QJsonValue toJson(const QVariant &value) const override;
    QVariant fromJson(const QJsonValue &value, int type) const override;

private:
    int m_QMarginsF_type = -1;
    int m_QMargins_type = -1;
};

class QListHelper : public QObjectSerializer::Helper
{
public:
    explicit QListHelper();
    ~QListHelper() override;

    bool canHandle(int type) const override;
    QJsonValue toJson(const QVariant &value) const override;
    QVariant fromJson(const QJsonValue &value, int type) const override;

private:
    int m_QListInt_type = -1;
    int m_QListReal_type = -1;
    int m_QListColor_type = -1;
    int m_unused = 0;
};

class QRealHelper : public QObjectSerializer::Helper
{
public:
    explicit QRealHelper();
    ~QRealHelper() override;

    bool canHandle(int type) const override;
    QJsonValue toJson(const QVariant &value) const override;
    QVariant fromJson(const QJsonValue &value, int type) const override;
};

class QFontHelper : public QObjectSerializer::Helper
{
public:
    explicit QFontHelper();
    ~QFontHelper() override;

    bool canHandle(int type) const override;
    QJsonValue toJson(const QVariant &value) const override;
    QVariant fromJson(const QJsonValue &value, int type) const override;
};

class QRectFHelper : public QObjectSerializer::Helper
{
public:
    explicit QRectFHelper();
    ~QRectFHelper() override;

    bool canHandle(int type) const override;
    QJsonValue toJson(const QVariant &value) const override;
    QVariant fromJson(const QJsonValue &value, int type) const override;
};

class ObjectSerializerHelperRegistry : public QList<QObjectSerializer::Helper *>
{
public:
    explicit ObjectSerializerHelperRegistry();
    ~ObjectSerializerHelperRegistry();

    const QObjectSerializer::Helper *findHelper(int type) const;
};

ObjectSerializerHelperRegistry::ObjectSerializerHelperRegistry()
{
    // this->append(new QRealHelper);
    this->append(new QMarginsFHelper);
    this->append(new QListHelper);
    this->append(new QFontHelper);
    this->append(new QRectFHelper);
}

ObjectSerializerHelperRegistry::~ObjectSerializerHelperRegistry()
{
    qDeleteAll(*this);
}

const QObjectSerializer::Helper *ObjectSerializerHelperRegistry::findHelper(int type) const
{
    const int nrHelpers = this->count();
    for (int i = 0; i < nrHelpers; i++) {
        const QObjectSerializer::Helper *helper = this->at(i);
        if (helper->canHandle(type))
            return helper;
    }

    return nullptr;
}

Q_GLOBAL_STATIC(ObjectSerializerHelperRegistry, Helpers)

void QObjectSerializer::registerHelper(QObjectSerializer::Helper *helper)
{
    if (::Helpers()->contains(helper))
        return;

    ::Helpers()->append(helper);
}

QObjectSerializer::Helper::~Helper()
{
    ::Helpers()->removeOne(this);
}

QObjectSerializer::Interface::~Interface() { }

QJsonObject QObjectSerializer::toJson(const QObject *object)
{
    QJsonObject ret;
    if (object == nullptr)
        return ret;

    QObjectSerializer::Interface *interface = qobject_cast<QObjectSerializer::Interface *>(object);
    if (interface != nullptr)
        interface->prepareForSerialization();

    QStack<const QMetaObject *> metaObjects;
    QStringList classNames;
    const QMetaObject *mo = object->metaObject();
    while (mo) {
        metaObjects.push(mo);
        if (interface != nullptr && interface->canSerialize(mo, QMetaProperty()) == false) {
            mo = mo->superClass();
            continue;
        }

        classNames << QString::fromLatin1(mo->className());
        mo = mo->superClass();
    }

    // ret.insert("_class", classNames.join(","));

    const QVariantMap defaultProperties =
            QObjectSerializer::cacheDefaultPropertyValues(object, true);

    while (!metaObjects.isEmpty()) {
        mo = metaObjects.pop();

        const int nrProperties = mo->propertyCount();
        for (int i = mo->propertyOffset(); i < nrProperties; i++) {
            const QMetaProperty prop = mo->property(i);
            if (interface != nullptr && interface->canSerialize(mo, prop) == false)
                continue;

#ifdef QT_WIDGETS_LIB
            // QGraphicsObject::parent property returns a parent QGraphicsObject.
            // While saving a QGraphicsObject, we could end up in recursion if
            // we are saving the QGraphicsObject's children also.
            static const char *parentPropName = "parent";
            if (mo == &QGraphicsObject::staticMetaObject && !qstrcmp(prop.name(), parentPropName))
                continue;
#endif

            const QMetaType propType(prop.userType());

            // The objectName property wont be stored. In all my experiments so far,
            // storing objectName has turned out to be pointless.
            static const char *objectName = "objectName";
            if (!qstrcmp(prop.name(), objectName))
                continue;

            // If the developer has explicitly marked the property as STORED false,
            // then we dont bother saving the property into the JSON
            if (!prop.isStored())
                continue;

            // Unless a property is writable, whats the point in serializing it
            // into JSON. The whole idea of serialization to JSON is that we can
            // unserialize it back into a QObject. Since properties that are readonly
            // cant be unserialized, whats the point of storing them.
            // The only exception to this rule is if the property is returning a QObject
            // type. In which case, we have to serialize it.
            const bool isQObjectPointer = (propType.flags() & QMetaType::PointerToQObject);
            const bool isQQmlListProperty =
                    QByteArray(prop.typeName()).startsWith("QQmlListProperty");
            if (!prop.isWritable() && !isQObjectPointer && !isQQmlListProperty)
                continue;

            const QString propName = QString::fromLatin1(prop.name());
            const QVariant defaultPropValue = defaultProperties.value(propName);
            QVariant propValue = prop.read(object);

            if (isQQmlListProperty) {
                QJsonArray list;

                QQmlListReference listRef(const_cast<QObject *>(object), prop.name());
                for (int i = 0; i < listRef.count(); i++) {
                    const QObject *listItem = listRef.at(i);
                    if (listItem == nullptr)
                        continue;

                    QJsonObject item = QObjectSerializer::toJson(listItem);
                    list.append(item);
                }

                ret.insert(propName, list);
            } else if (prop.isEnumType()) {
                const QMetaEnum propEnum = prop.enumerator();
                propValue = QString::fromLatin1(propEnum.valueToKey(propValue.toInt()));
                if (defaultPropValue == propValue.toString())
                    continue;

                ret.insert(propName, propValue.toString());
            } else if (prop.isFlagType()) {
                const QMetaEnum propEnum = prop.enumerator();
                propValue = QString::fromLatin1(propEnum.valueToKeys(propValue.toInt()));
                if (defaultPropValue == propValue.toString())
                    continue;

                ret.insert(propName, propValue.toString());
            } else if (propType.flags() & QMetaType::PointerToQObject) {
                propValue.convert(QMetaType::QObjectStar);

                const QObject *propObject = propValue.value<QObject *>();
                if (propObject != nullptr) {
                    const QJsonObject propJson = QObjectSerializer::toJson(propObject);
                    if (!propJson.isEmpty())
                        ret.insert(propName, propJson);
                }
            } else if (propValue.userType() == QMetaType::QJsonValue) {
                const QJsonValue propJsonValue = propValue.toJsonValue();
                if (defaultPropValue.toJsonValue() == propValue.toJsonValue())
                    continue;

                ret.insert(propName, propJsonValue);
            } else if (propValue.userType() == QMetaType::QJsonObject) {
                const QJsonObject propJsonObject = propValue.toJsonObject();
                if (defaultPropValue.toJsonObject() == propJsonObject)
                    continue;

                ret.insert(propName, propJsonObject);
            } else if (propValue.userType() == QMetaType::QJsonArray) {
                const QJsonArray propJsonArray = propValue.toJsonArray();
                if (defaultPropValue.toJsonArray() == propJsonArray)
                    continue;

                ret.insert(propName, propJsonArray);
            } else {
                const QObjectSerializer::Helper *helper = ::Helpers()->findHelper(prop.userType());
                if (helper == nullptr) {
                    if (propValue == defaultPropValue)
                        continue;

                    ret.insert(propName, QJsonValue::fromVariant(propValue));
                } else {
                    const QJsonValue propJsonValue = helper->toJson(propValue);
                    if (propJsonValue == defaultPropValue.toJsonValue())
                        continue;

                    ret.insert(propName, propJsonValue);
                }
            }
        }
    }

#ifdef SERIALIZE_DYNAMIC_PROPERTIES
    const QList<QByteArray> dynPropNames = object->dynamicPropertyNames();
    for (const QByteArray &propName : dynPropNames) {
        const QVariant propValue = object->property(propName);
        const QString key = QString("(%1)").arg(QString::fromLatin1(propName));
        const QJsonValue value = QJsonValue::fromVariant(propValue);
        ret.insert(key, value);
    }
#endif

    if (interface != nullptr)
        interface->serializeToJson(ret);

    return ret;
}

bool QObjectSerializer::fromJson(const QJsonObject &json, QObject *object, QObjectFactory *factory)
{
    if (object == nullptr)
        return false;

    if (json.isEmpty())
        return false;

    QObjectSerializer::Interface *interface = qobject_cast<QObjectSerializer::Interface *>(object);
    if (interface != nullptr)
        interface->prepareForDeserialization();

    QStack<const QMetaObject *> metaObjects;
    QStringList classNames;
    const QMetaObject *mo = object->metaObject();
    while (mo) {
        metaObjects.push(mo);
        if (interface != nullptr && interface->canSerialize(mo, QMetaProperty()) == false) {
            mo = mo->superClass();
            continue;
        }

        classNames << QString::fromLatin1(mo->className());
        mo = mo->superClass();
    }

    while (!metaObjects.isEmpty()) {
        mo = metaObjects.pop();

        const int nrProperties = mo->propertyCount();
        for (int i = mo->propertyOffset(); i < nrProperties; i++) {
            const QMetaProperty prop = mo->property(i);
            if (interface != nullptr && interface->canSerialize(mo, prop) == false)
                continue;

            const QMetaType propType(prop.userType());

            static const char *objectName = "objectName";
            if (!qstrcmp(prop.name(), objectName))
                continue;

            if (!prop.isStored())
                continue;

            const bool isQObjectPointer = (propType.flags() & QMetaType::PointerToQObject);
            const bool isQQmlListProperty =
                    QByteArray(prop.typeName()).startsWith("QQmlListProperty");
            if (!prop.isWritable() && !isQObjectPointer && !isQQmlListProperty)
                continue;

            const QString propName = QString::fromLatin1(prop.name());
            if (!json.contains(propName))
                continue;

            const QJsonValue jsonPropValue = json.value(propName);

            if (isQQmlListProperty) {
                const QJsonArray list = jsonPropValue.toArray();

                QQmlListReference listRef(const_cast<QObject *>(object), prop.name());
                const bool canAddObjects =
                        interface && interface->canSetPropertyFromObjectList(propName)
                        && listRef.canAppend();

                QObjectFactory listItemFactory;
                const QByteArray className(listRef.listElementType()->className());
                listItemFactory.add(listRef.listElementType());

                QList<QObject *> propertyObjects;
                if (canAddObjects)
                    propertyObjects.reserve(list.size());
                else if (listRef.canAppend())
                    listRef.clear();

                for (int i = 0; i < list.size(); i++) {
                    const QJsonObject listItem = list.at(i).toObject();

                    if (listRef.canAppend()) {
                        QObject *listItemObject =
                                listItemFactory.create(className, listRef.object());
                        QObjectSerializer::fromJson(listItem, listItemObject, factory);
                        if (canAddObjects)
                            propertyObjects.append(listItemObject);
                        else
                            listRef.append(listItemObject);
                    } else {
                        QObject *listItemObject = listRef.at(i);
                        if (listItemObject == nullptr)
                            continue;
                        QObjectSerializer::fromJson(listItem, listItemObject, factory);
                    }
                }

                if (canAddObjects)
                    interface->setPropertyFromObjectList(propName, propertyObjects);

                continue;
            }

            if (prop.isEnumType() || prop.isFlagType()) {
                const QByteArray key = jsonPropValue.toString().toLatin1();
                const QMetaEnum enumerator = prop.enumerator();
                const int value = prop.isFlagType()
                        ? (key.isEmpty() ? 0 : enumerator.keysToValue(key))
                        : enumerator.keyToValue(key);
                prop.write(object, value);
                continue;
            }

            if (propType.flags() & QMetaType::PointerToQObject) {
                QObjectFactory *usableFactory = factory;
                QObjectFactory stopGapFactory;

                const QVariant propValue = prop.read(object);
                QObject *propObject = propValue.value<QObject *>();
                if (propObject == nullptr) {
                    if (factory == nullptr) {
                        stopGapFactory.add(QMetaType::metaObjectForType(prop.userType()));
                        usableFactory = &stopGapFactory;
                    } else
                        factory->add(QMetaType::metaObjectForType(prop.userType()));

                    if (prop.isWritable() && usableFactory != nullptr) {
                        const QByteArray className = QByteArray(prop.typeName()).replace('*', "");
                        propObject = usableFactory->create(className, object);
                        if (propObject == nullptr)
                            continue;

                        prop.write(object, QVariant::fromValue(propObject));
                    } else
                        continue;
                }

                const QJsonObject propJson = jsonPropValue.toObject();
                QObjectSerializer::fromJson(propJson, propObject, usableFactory);
                continue;
            }

            switch (prop.userType()) {
            case QMetaType::QJsonValue:
                prop.write(object, QVariant::fromValue<QJsonValue>(jsonPropValue));
                continue;
            case QMetaType::QJsonObject:
                prop.write(object, QVariant::fromValue<QJsonObject>(jsonPropValue.toObject()));
                continue;
            case QMetaType::QJsonArray:
                prop.write(object, QVariant::fromValue<QJsonArray>(jsonPropValue.toArray()));
                continue;
            default:
                break;
            }

            const QObjectSerializer::Helper *helper = ::Helpers()->findHelper(prop.userType());
            const QJsonValue propJsonValue = json.value(propName);
            const QVariant propValue = helper == nullptr
                    ? propJsonValue.toVariant()
                    : helper->fromJson(propJsonValue, prop.userType());
            prop.write(object, propValue);
        }
    }

#ifdef SERIALIZE_DYNAMIC_PROPERTIES
    QJsonObject::const_iterator it = json.constBegin();
    QJsonObject::const_iterator end = json.constEnd();
    while (it != end) {
        const QString key = it.key();
        if (key.isEmpty() || key.at(0) != QChar('(')) {
            ++it;
            continue;
        }

        const QByteArray propName = key.mid(1, key.lastIndexOf(')') - 1).toLatin1();
        const QVariant propValue = it.value().toVariant();

        if (key.endsWith('+')) {
            const QVariant existingPropValue = object->property(propName);
            if (!existingPropValue.isValid())
                object->setProperty(propName, propValue);
            else {
                QVariant newPropValue;
                switch (existingPropValue.userType()) {
                case QMetaType::Int:
                case QMetaType::Bool:
                case QMetaType::Double:
                case QMetaType::QString: {
                    QVariantList list;
                    list << existingPropValue;
                    if (propValue.userType() == existingPropValue.userType())
                        list << propValue;
                    else if (propValue.userType() == QMetaType::QStringList
                             || propValue.userType() == QMetaType::QVariantList)
                        list += propValue.toList();
                    newPropValue = list;
                } break;
                case QMetaType::QStringList: {
                    QStringList list = existingPropValue.toStringList();
                    list += propValue.toStringList();
                    newPropValue = list;
                } break;
                case QMetaType::QVariantMap: {
                    QVariantMap map = existingPropValue.toMap();
                    map.unite(propValue.toMap());
                    newPropValue = map;
                } break;
                default:
                    break;
                }

                object->setProperty(propName, newPropValue);
            }
        } else
            object->setProperty(propName, propValue);

        ++it;
    }
#endif

    if (interface != nullptr)
        interface->deserializeFromJson(json);

    return true;
}

QString QObjectSerializer::toJsonString(const QObject *object)
{
    const QJsonObject json = QObjectSerializer::toJson(object);
    const QJsonDocument doc(json);
    return doc.toJson();
}

bool QObjectSerializer::fromJsonString(const QString &json, QObject *object,
                                       QObjectFactory *factory)
{
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    const QJsonObject jsonObject = doc.object();
    return QObjectSerializer::fromJson(jsonObject, object, factory);
}

///////////////////////////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(QMarginsF)
Q_DECLARE_METATYPE(QMargins)

QMarginsFHelper::QMarginsFHelper()
{
    m_QMarginsF_type = qRegisterMetaType<QMarginsF>("QMarginsF");
    m_QMargins_type = qRegisterMetaType<QMargins>("QMargins");
}

QMarginsFHelper::~QMarginsFHelper() { }

bool QMarginsFHelper::canHandle(int type) const
{
    return (type == m_QMargins_type) || (type == m_QMarginsF_type);
}

QJsonValue QMarginsFHelper::toJson(const QVariant &value) const
{
    if (value.userType() == m_QMarginsF_type) {
        const QMarginsF margins = value.value<QMarginsF>();
        return QString("%1,%2,%3,%4")
                .arg(margins.left())
                .arg(margins.top())
                .arg(margins.right())
                .arg(margins.bottom());
    }

    if (value.userType() == m_QMargins_type) {
        const QMargins margins = value.value<QMargins>();
        return QString("%1,%2,%3,%4")
                .arg(margins.left())
                .arg(margins.top())
                .arg(margins.right())
                .arg(margins.bottom());
    }

    return QString();
}

QVariant QMarginsFHelper::fromJson(const QJsonValue &value, int type) const
{
    const QStringList comps = value.toString().split(",", Qt::SkipEmptyParts);
    if (comps.size() != 4) {
        if (type == m_QMarginsF_type)
            return QVariant::fromValue<QMarginsF>(QMarginsF());

        if (type == m_QMargins_type)
            return QVariant::fromValue<QMargins>(QMargins());

        return QVariant();
    }

    if (type == m_QMarginsF_type)
        return QVariant::fromValue<QMarginsF>(
                QMarginsF(comps.at(0).toDouble(), comps.at(1).toDouble(), comps.at(2).toDouble(),
                          comps.at(3).toDouble()));

    if (type == m_QMargins_type)
        return QVariant::fromValue<QMargins>(QMargins(comps.at(0).toInt(), comps.at(1).toInt(),
                                                      comps.at(2).toInt(), comps.at(3).toInt()));

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<qreal>)
Q_DECLARE_METATYPE(QList<QColor>)

QListHelper::QListHelper()
{
    m_QListInt_type = qRegisterMetaType<QList<int>>("QList<int>");
    m_QListReal_type = qRegisterMetaType<QList<qreal>>("QList<qreal>");
    m_QListColor_type = qRegisterMetaType<QList<QColor>>("QList<QColor>");
    m_unused = 0;
}

QListHelper::~QListHelper() { }

bool QListHelper::canHandle(int type) const
{
    return (type == m_QListInt_type) || (type == m_QListReal_type) || (type == m_QListColor_type);
}

QJsonValue QListHelper::toJson(const QVariant &value) const
{
    QJsonArray ret;

    if (value.userType() == m_QListInt_type) {
        const QList<int> ints = value.value<QList<int>>();
        for (int i : ints)
            ret.append(i);
        return ret;
    }

    if (value.userType() == m_QListReal_type) {
        const QList<qreal> reals = value.value<QList<qreal>>();
        for (qreal r : reals)
            ret.append(r);
        return ret;
    }

    if (value.userType() == m_QListColor_type) {
        const QList<QColor> colors = value.value<QList<QColor>>();
        for (const QColor &c : colors)
            ret.append(c.name());
        return ret;
    }

    return QJsonValue();
}

QVariant QListHelper::fromJson(const QJsonValue &value, int type) const
{
    const QJsonArray array = value.toArray();

    if (type == m_QListInt_type) {
        QList<int> ints;
        for (int i = 0; i < array.size(); i++)
            ints << array.at(i).toInt();
        return QVariant::fromValue<QList<int>>(ints);
    }

    if (type == m_QListReal_type) {
        QList<qreal> reals;
        for (int i = 0; i < array.size(); i++)
            reals << array.at(i).toDouble();
        return QVariant::fromValue<QList<qreal>>(reals);
    }

    if (type == m_QListColor_type) {
        QList<QColor> colors;
        for (int i = 0; i < array.size(); i++)
            colors << QColor(array.at(i).toString());
        return QVariant::fromValue<QList<QColor>>(colors);
    }

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////

QRealHelper::QRealHelper() { }

QRealHelper::~QRealHelper() { }

bool QRealHelper::canHandle(int type) const
{
    return type == QMetaType::QReal || type == QMetaType::Double || type == QMetaType::Float;
}

QJsonValue QRealHelper::toJson(const QVariant &value) const
{
    const qreal val = value.toReal();
    return QString::number(val);
}

QVariant QRealHelper::fromJson(const QJsonValue &value, int type) const
{
    const double val = value.toDouble();

    QVariant ret(val);
    ret.convert(type);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

QFontHelper::QFontHelper() { }

QFontHelper::~QFontHelper() { }

bool QFontHelper::canHandle(int type) const
{
    return type == QMetaType::QFont;
}

QJsonValue QFontHelper::toJson(const QVariant &value) const
{
    const QFont font = value.value<QFont>();
    const QMetaObject *fontMetaObject = &(QFont::staticMetaObject);

    auto enumValue = [fontMetaObject](const char *name, int value) {
        const int ei = fontMetaObject->indexOfEnumerator(name);
        if (ei < 0)
            return QString::number(value);

        const QMetaEnum e = fontMetaObject->enumerator(ei);
        return QString::fromLatin1(e.valueToKey(value));
    };

    static const QFont defaultFont;

    QJsonObject ret;

    if (font.family() != defaultFont.family())
        ret.insert("family", font.family());

    if (font.pixelSize() != defaultFont.pixelSize())
        ret.insert("pixelSize", font.pixelSize());

    if (font.pointSize() != defaultFont.pointSize())
        ret.insert("pointSize", font.pointSize());

    if (font.weight() != defaultFont.weight())
        ret.insert("weight", font.weight());

    if (font.capitalization() != defaultFont.capitalization())
        ret.insert("caps", enumValue("Capitalization", font.capitalization()));

    if (font.stretch() != defaultFont.stretch())
        ret.insert("stretch", enumValue("Stretch", font.stretch()));

    if (font.letterSpacingType() != defaultFont.letterSpacingType()
        || !qFuzzyCompare(font.letterSpacing(), defaultFont.letterSpacing())) {
        ret.insert("spacingType", enumValue("SpacingType", font.letterSpacingType()));
        ret.insert("spacing", font.letterSpacing());
    }

    if (font.style() != defaultFont.style())
        ret.insert("style", enumValue("Style", font.style()));

    if (font.underline() != defaultFont.underline())
        ret.insert("underline", font.underline());

    return ret;
}

QVariant QFontHelper::fromJson(const QJsonValue &value, int type) const
{
    if (type != QMetaType::QFont)
        return QVariant::fromValue<QFont>(QFont());

    const QMetaObject *fontMetaObject = &(QFont::staticMetaObject);
    auto enumValue = [fontMetaObject](const char *name, const QJsonValue &key) {
        const int ei = fontMetaObject->indexOfEnumerator(name);
        if (ei < 0)
            return 0;

        const QMetaEnum e = fontMetaObject->enumerator(ei);
        const QString keyStr = key.toString();
        return e.keyToValue(qPrintable(keyStr));
    };

    const QJsonObject json = value.toObject();

    QFont font;
    if (json.contains("family"))
        font.setFamily(json.value("family").toString());

    if (json.contains("pixelSize"))
        font.setPixelSize(json.value("pixelSize").toInt());

    if (json.contains("pointSize"))
        font.setPointSize(json.value("pointSize").toInt());

    if (json.contains("weight"))
        font.setWeight(json.value("weight").toInt());

    if (json.contains("caps"))
        font.setCapitalization(
                QFont::Capitalization(enumValue("Capitalization", json.value("caps"))));

    if (json.contains("stretch"))
        font.setStretch(enumValue("Stretch", json.value("stretch")));

    if (json.contains("spacingType") && json.contains("spacing"))
        font.setLetterSpacing(
                QFont::SpacingType(enumValue("SpacingType", json.value("spacingType"))),
                json.value("spacing").toDouble());

    if (json.contains("style"))
        font.setStyle(QFont::Style(enumValue("Style", json.value("style"))));

    if (json.contains("underline"))
        font.setUnderline(json.value("underline").toBool());

    return QVariant::fromValue<QFont>(font);
}

///////////////////////////////////////////////////////////////////////////////

QRectFHelper::QRectFHelper() { }

QRectFHelper::~QRectFHelper() { }

bool QRectFHelper::canHandle(int type) const
{
    return type == QMetaType::QRect || type == QMetaType::QRectF;
}

QJsonValue QRectFHelper::toJson(const QVariant &value) const
{
    QRectF rect;
    if (value.userType() == QMetaType::QRect)
        rect = value.value<QRect>();
    else
        rect = value.value<QRectF>();

    QJsonObject ret;
    ret.insert("x", rect.x());
    ret.insert("y", rect.y());
    ret.insert("width", rect.width());
    ret.insert("height", rect.height());
    return ret;
}

QVariant QRectFHelper::fromJson(const QJsonValue &value, int type) const
{
    const QJsonObject object = value.toObject();

    const QRectF rect(object.value("x").toDouble(), object.value("y").toDouble(),
                      object.value("width").toDouble(), object.value("height").toDouble());

    if (type == QMetaType::QRectF)
        return rect;

    return rect.toRect();
}

///////////////////////////////////////////////////////////////////////////////

QVariantMap QObjectSerializer::cacheDefaultPropertyValues(const QObject *object, bool readonly)
{
    static QMap<QByteArray, QVariantMap> defaultPropertyValueMap;

    QVariantMap ret;
    if (object == nullptr)
        return ret;

    const QByteArray className(object->metaObject()->className());
    if (defaultPropertyValueMap.contains(className) || readonly)
        return defaultPropertyValueMap.value(className);

    QObjectSerializer::Interface *interface = qobject_cast<QObjectSerializer::Interface *>(object);

    QStack<const QMetaObject *> metaObjects;
    QStringList classNames;
    const QMetaObject *mo = object->metaObject();
    while (mo) {
        metaObjects.push(mo);
        if (interface != nullptr && interface->canSerialize(mo, QMetaProperty()) == false) {
            mo = mo->superClass();
            continue;
        }

        mo = mo->superClass();
    }

    while (!metaObjects.isEmpty()) {
        mo = metaObjects.pop();
        const int nrProperties = mo->propertyCount();
        for (int i = mo->propertyOffset(); i < nrProperties; i++) {
            const QMetaProperty prop = mo->property(i);
            if (interface != nullptr && interface->canSerialize(mo, prop) == false)
                continue;

#ifdef QT_WIDGETS_LIB
            static const char *parentPropName = "parent";
            if (mo == &QGraphicsObject::staticMetaObject && !qstrcmp(prop.name(), parentPropName))
                continue;
#endif

            const QMetaType propType(prop.userType());
            static const char *objectName = "objectName";
            if (!qstrcmp(prop.name(), objectName))
                continue;

            if (!prop.isStored())
                continue;

            const bool isQObjectPointer = (propType.flags() & QMetaType::PointerToQObject);
            if (isQObjectPointer)
                continue;

            const QString propName = QString::fromLatin1(prop.name());
            const QVariant propValue = prop.read(object);

            if (prop.isEnumType()) {
                const QMetaEnum propEnum = prop.enumerator();
                ret.insert(propName, QString::fromLatin1(propEnum.valueToKey(propValue.toInt())));
            } else if (prop.isFlagType()) {
                const QMetaEnum propEnum = prop.enumerator();
                ret.insert(propName, QString::fromLatin1(propEnum.valueToKeys(propValue.toInt())));
            } else if (propType.flags() & QMetaType::PointerToQObject) {
                continue;
            } else if (propValue.canConvert(QMetaType::QJsonValue)) {
                ret.insert(propName, propValue.toJsonValue());
            } else {
                const QObjectSerializer::Helper *helper = ::Helpers()->findHelper(prop.userType());
                if (helper == nullptr)
                    ret.insert(propName, propValue);
                else
                    ret.insert(propName, helper->toJson(propValue));
            }
        }
    }

    defaultPropertyValueMap.insert(className, ret);

    return ret;
}
