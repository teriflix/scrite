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

#include "utils.h"

#include "application.h"
#include "qobjectlistmodel.h"
#include "enumerationmodel.h"

#include <QDir>
#include <QtMath>
#include <QLocale>
#include <QHostInfo>
#include <QSettings>
#include <QClipboard>
#include <QSslSocket>
#include <QMetaObject>
#include <QDataStream>
#include <QPainterPath>
#include <QColorDialog>
#include <QKeySequence>
#include <QMetaProperty>
#include <QJsonDocument>
#include <QVersionNumber>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QOperatingSystemVersion>

/**
 * \brief Registers meta types used by the Utils class.
 */
void Utils::registerTypes()
{
    static bool typesRegistered = false;
    if (!typesRegistered) {
        qRegisterMetaType<Utils::KeyCombinations>("Utils::KeyCombinations");
        qRegisterMetaType<Utils::FileInfo>("Utils::FileInfo");
        qRegisterMetaType<Utils::ObjectConfigFieldChoice>("Utils::ObjectConfigFieldChoice");
        qRegisterMetaType<QList<Utils::ObjectConfigFieldChoice>>(
                "QList<Utils::ObjectConfigFieldChoice>");
        qRegisterMetaType<Utils::ObjectConfigField>("Utils::ObjectConfigField");
        qRegisterMetaType<QList<Utils::ObjectConfigField>>("QList<Utils::ObjectConfigField>");
        qRegisterMetaType<Utils::ObjectConfigFieldGroup>("Utils::ObjectConfigFieldGroup");
        qRegisterMetaType<QList<Utils::ObjectConfigFieldGroup>>(
                "QList<Utils::ObjectConfigFieldGroup>");
        qRegisterMetaType<Utils::ObjectConfig>("Utils::ObjectConfig");

        typesRegistered = true;
    }
}

///////////////////////////////////////////////////////////////////////////////

QStringList Utils::KeyCombinations::modifiers() const
{
    QStringList ret;

    if (this->metaModifier)
        ret.append(Platform::modifierDescription(Qt::MetaModifier));

    if (this->controlModifier)
        ret.append(Platform::modifierDescription(Qt::ControlModifier));

    if (this->altModifier)
        ret.append(Platform::modifierDescription(Qt::AltModifier));

    if (this->shiftModifier)
        ret.append(Platform::modifierDescription(Qt::ShiftModifier));

    return ret;
}

QString Utils::KeyCombinations::toShortcut() const
{
    const QKeySequence ks = this->toKeySequence();
    if (ks.isEmpty())
        return QString();
    return ks.toString();
}

QKeySequence Utils::KeyCombinations::toKeySequence() const
{
    if (this->keyCodes.isEmpty())
        return QKeySequence();

    int k1 = 0, k2 = 0, k3 = 0, k4 = 0;
    if (this->metaModifier)
        k1 += Qt::MetaModifier;
    if (this->controlModifier)
        k1 += Qt::ControlModifier;
    if (this->altModifier)
        k1 += Qt::AltModifier;
    if (this->shiftModifier)
        k1 += Qt::ShiftModifier;

    k1 += this->keyCodes[0];
    if (this->keyCodes.size() >= 2)
        k2 = this->keyCodes[1];
    if (this->keyCodes.size() >= 3)
        k3 = this->keyCodes[2];
    if (this->keyCodes.size() >= 4)
        k4 = this->keyCodes[3];

    return QKeySequence(k1, k2, k3, k4);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Returns the platform type.
 * \return The platform type as Utils::Platform::Type.
 */
Utils::Platform::Type Utils::Platform::type()
{
#ifdef Q_OS_MAC
    return MacOSDesktop;
#else
#ifdef Q_OS_WIN
    return WindowsDesktop;
#else
    return LinuxDesktop;
#endif
#endif
}

/**
 * \brief Returns the platform type as a string.
 * \return A string representation of the platform type.
 */
QString Utils::Platform::typeString()
{
    switch (type()) {
    case WindowsDesktop:
        return QStringLiteral("Windows");
    case LinuxDesktop:
        return QStringLiteral("Linux");
    case MacOSDesktop:
        return QStringLiteral("macOS");
    }

    return QStringLiteral("Unknown");
}

/**
 * \brief Returns the major version of the operating system.
 * \return The major version number.
 */
int Utils::Platform::osMajorVersion()
{
    const QList<int> osv = osVersion();
    return osv[0];
}

/**
 * \brief Returns the operating system version as a list of integers.
 * \return A list containing major, minor, and micro version numbers.
 */
QList<int> Utils::Platform::osVersion()
{
    const QOperatingSystemVersion v = QOperatingSystemVersion::current();
#ifdef Q_OS_WIN
    if (v.majorVersion() > 0) {
        if (v.majorVersion() == 10) {
            if (v.minorVersion() == 0)
                return { (v.microVersion() >= 22000 ? 11 : 10), v.microVersion(), 0 };
        }
    }
#endif

    return { v.majorVersion(), v.minorVersion(), v.minorVersion() };
}

/**
 * \brief Returns the operating system version as a string.
 * \return A string representation of the OS version.
 */
QString Utils::Platform::osVersionString()
{
    const QList<int> osv = osVersion();
    if (osv[0] > 0)
        return QVersionNumber(osv[0], osv[1], osv[2]).toString();

    return QSysInfo::productVersion();
}

/**
 * \brief Returns version of Qt used to build this app.
 * \return A string representation of the Qt version.
 */
QString Utils::Platform::qtVersionString()
{
    return QStringLiteral(QT_VERSION_STR);
}

/**
 * \brief Returns version of OpenSSL used to build this app.
 * \return A string representation of the OpenSSL version.
 */
QString Utils::Platform::openSslVersionString()
{
    return QSslSocket::sslLibraryVersionString();
}

/**
 * \brief Returns the host name of the machine.
 * \return The local host name.
 */
QString Utils::Platform::hostName()
{
    return QHostInfo::localHostName();
}

/**
 * Returns path to the folder in which settings.ini for this platform is stored
 */
QString Utils::Platform::settingsPath()
{
    const QFileInfo fi(Application::instance()->settingsFilePath());
    return fi.absoluteFilePath();
}

/**
 * Returns complete path of the settings.ini on this platform
 */
QString Utils::Platform::settingsFile()
{
    return Application::instance()->settingsFilePath();
}

/**
 * Returns complete path for the relative name supplied here, such that it shows up
 * in the same folder as settings.ini file for this platform.
 */
QString Utils::Platform::configPath(const QString &relativeName)
{
    const QFileInfo fi(Application::instance()->settingsFilePath());
    const QString ret = fi.absoluteDir().absoluteFilePath(relativeName);
    QDir().mkpath(QFileInfo(ret).absolutePath());
    return ret;
}

QString Utils::Platform::modifierDescription(int modifier)
{

    switch (modifier) {
    case Qt::ShiftModifier:
        return QStringLiteral("Shift") + (isMacOSDesktop() ? " ⇧" : "");
    case Qt::ControlModifier:
        return isMacOSDesktop() ? QStringLiteral("Cmd ⌘") : QStringLiteral("Ctrl");
    case Qt::AltModifier:
        return QStringLiteral("Alt") + (isMacOSDesktop() ? " ⌥" : "");
    case Qt::MetaModifier:
        return isMacOSDesktop() ? QStringLiteral("Ctrl ⌃") : QStringLiteral("Meta");
    }

    return QString();
}

/**
 * \brief Returns the architecture of the platform.
 * \return The architecture as Utils::Platform::Architecture.
 */
Utils::Platform::Architecture Utils::Platform::architecture()
{
    return QSysInfo::WordSize == 32 ? x86 : x64;
}

/**
 * \brief Returns the architecture of the platform as a string
 * \return Either "x86" or "x64"
 */
QString Utils::Platform::architectureString()
{
    switch (architecture()) {
    case x86:
        return "x86";
    case x64:
        return "x64";
    }
    return "unknown";
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Returns an empty QImage.
 * \return An empty QImage object.
 */
QImage Utils::Gui::emptyQImage()
{
    return QImage();
}

QString Utils::Gui::shortcut(int k1, int k2, int k3, int k4)
{
    return keySequence(k1, k2, k3, k4).toString();
}

QKeySequence Utils::Gui::keySequence(int k1, int k2, int k3, int k4)
{
    return QKeySequence(k1, k2, k3, k4);
}

Utils::KeyCombinations Utils::Gui::keyCombinations(const QString &shortcut)
{
    Utils::KeyCombinations result;
    const QKeySequence keySequence = QKeySequence::fromString(shortcut);

    for (int i = 0; i < keySequence.count(); i++) {
        const int key = keySequence[i];
        if (i == 0) {
            if (key & Qt::ControlModifier)
                result.controlModifier = true;
            if (key & Qt::AltModifier)
                result.altModifier = true;
            if (key & Qt::ShiftModifier)
                result.shiftModifier = true;
            if (key & Qt::MetaModifier)
                result.metaModifier = true;
            // Note: Qt::KeypadModifier and Qt::GroupSwitchModifier are not handled in the struct
        }

        const int keyOnly = key & ~Qt::KeyboardModifierMask;
        if (keyOnly > 0) {
            result.keyCodes.append(static_cast<Qt::Key>(keyOnly));
            result.keys.append(QKeySequence(keyOnly).toString(QKeySequence::PortableText));
        }
    }

    return result;
}

QString Utils::Gui::standardShortcut(int standardKey)
{
    return QKeySequence(QKeySequence::StandardKey(standardKey)).toString();
}

QKeySequence Utils::Gui::standardKeySequence(int standardKey)
{
    return QKeySequence(QKeySequence::StandardKey(standardKey));
}

/**
 * \brief Converts a portable shortcut string to native format.
 * \param shortcut The shortcut string in portable format.
 * \return The shortcut string in native format, or empty if invalid.
 */
QString Utils::Gui::nativeShortcut(const QString &shortcut)
{
    if (shortcut.isEmpty())
        return QString();

    const QKeySequence keySequence = QKeySequence::fromString(shortcut);
    if (keySequence.isEmpty())
        return QString();

    return keySequence.toString(QKeySequence::NativeText);
}

/**
 * \brief Checks if the given QQuickItem accepts text input.
 * \param item The QQuickItem to check.
 * \return True if the item accepts input method, false otherwise.
 */
bool Utils::Gui::acceptsTextInput(QQuickItem *item)
{
    return item && item->flags() & QQuickItem::ItemAcceptsInputMethod;
}

/**
 * \brief Logs a message to stdout.
 * \param message The message to log.
 */
void Utils::Gui::log(const QString &message)
{
    fprintf(stdout, "%s\n", qPrintable(message));
    fflush(stdout);

#if 0
    QFile file(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/scrite.log");
    if (file.open(QFile::Append)) {
        const QString line = "[" + QDateTime::currentDateTime().toString() + "]: " + message + "\n";
        file.write(line.toLatin1());
        file.flush();
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Converts an HTTPS URL to HTTP.
 * \param url The URL to convert.
 * \return The URL with HTTPS scheme changed to HTTP, or the original URL if not HTTPS.
 */
QUrl Utils::Url::toHttp(const QUrl &url)
{
    if (url.scheme() != QStringLiteral("https"))
        return url;

    QUrl url2 = url;
    url2.setScheme(QStringLiteral("http"));
    return url2;
}

/**
 * \brief Creates a QUrl from a local file path.
 * \param fileName The file path.
 * \return A file:// URL for the given path.
 */
QUrl Utils::Url::fromPath(const QString &fileName)
{
    return QUrl::fromLocalFile(fileName);
}

/**
 * \brief Extracts the local file path from a file:// URL.
 * \param url The URL to convert.
 * \return The local file path.
 */
QString Utils::Url::toPath(const QUrl &url)
{
    return url.toLocalFile();
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Checks if a QVariant value is of a specific type.
 * \param value The QVariant to check.
 * \param typeName The type name to compare against.
 * \return True if the value is of the specified type, false otherwise.
 */
bool Utils::Object::isOfType(const QVariant &value, const QString &typeName)
{
    if (value.userType() == QMetaType::QObjectStar) {
        QObject *object = value.value<QObject *>();
        return object && object->inherits(qPrintable(typeName));
    }

    return QString::fromLatin1(value.typeName()) == typeName;
}

/**
 * \brief Returns the type name of a QVariant value.
 * \param value The QVariant to query.
 * \return The type name as a string.
 */
QString Utils::Object::typeOf(const QVariant &value)
{
    if (value.userType() == QMetaType::QObjectStar) {
        QObject *object = value.value<QObject *>();
        if (object == nullptr)
            return QString();

        return QString::fromLatin1(object->metaObject()->className());
    }

    return QString::fromLatin1(value.typeName());
}

/**
 * \brief Changes a property of a QObject.
 * \param object The QObject whose property to change.
 * \param name The property name.
 * \param value The new value for the property.
 * \return True if the property was changed successfully, false otherwise.
 */
bool Utils::Object::changeProperty(QObject *object, const QString &name, const QVariant &value)
{
    if (object == nullptr)
        return false;

    const QMetaObject *mo = object->metaObject();
    const int propIndex = mo->indexOfProperty(qPrintable(name));
    if (propIndex < 0)
        return false;

    const QMetaProperty prop = mo->property(propIndex);
    if (!prop.isWritable())
        return false;

    return prop.write(object, value);
}

/**
 * \brief Resets a property of a QObject to its default value.
 * \param object The QObject whose property to reset.
 * \param propName The property name.
 * \return True if the property was reset successfully, false otherwise.
 */
bool Utils::Object::resetProperty(QObject *object, const QString &propName)
{
    if (object == nullptr || propName.isEmpty())
        return false;

    const QMetaObject *mo = object->metaObject();
    const int propIndex = mo->indexOfProperty(qPrintable(propName));
    if (propIndex < 0)
        return false;

    const QMetaProperty prop = mo->property(propIndex);
    if (!prop.isResettable())
        return false;

    return prop.reset(object);
}

/**
 * \brief Queries the value of a property from a QObject.
 * \param object The QObject to query.
 * \param name The property name.
 * \return The property value, or an invalid QVariant if not found or readable.
 */
QVariant Utils::Object::queryProperty(QObject *object, const QString &name)
{
    if (object == nullptr)
        return QVariant();

    const QMetaObject *mo = object->metaObject();
    const int propIndex = mo->indexOfProperty(qPrintable(name));
    if (propIndex < 0)
        return QVariant();

    const QMetaProperty prop = mo->property(propIndex);
    if (!prop.isReadable())
        return QVariant();

    return prop.read(object);
}

/**
 * \brief Saves the configuration of a QObject to objectconfig.json.
 * \param object The QObject to save.
 * \return True if saved successfully, false otherwise.
 */
bool Utils::Object::save(QObject *object)
{
    if (object == nullptr)
        return false;

    const QMetaObject *mo = object->metaObject();
    const QString objectClass = QLatin1String(mo->className());

    QJsonObject objectJson = QObjectSerializer::toJson(object);

    const QStringList keys = objectJson.keys();
    for (const QString &key : keys) {
        const QString cikey = key + "_IsPersistent";
        const int cii = mo->indexOfClassInfo(qPrintable(cikey));
        if (cii >= 0) {
            const QMetaClassInfo ci = mo->classInfo(cii);
            if (!qstrcmp(ci.value(), "false"))
                objectJson.remove(key);
        }
    }

    const QString configFilePath =
            QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                    .absoluteFilePath("objectconfig.json");

    QJsonObject configObject;

    {
        QFile configFile(configFilePath);
        if (configFile.open(QFile::ReadOnly)) {
            QJsonDocument configFileDoc = QJsonDocument::fromJson(configFile.readAll());
            configObject = configFileDoc.object();
        }
    }

    configObject.insert(objectClass, objectJson);

    {
        QFile configFile(configFilePath);
        if (configFile.open(QFile::WriteOnly)) {
            QJsonDocument configFileDoc(configObject);
            configFile.write(configFileDoc.toJson(QJsonDocument::Indented));
            return true;
        }
    }

    return false;
}

/**
 * \brief Loads the configuration of a QObject from objectconfig.json.
 * \param object The QObject to load into.
 * \return True if loaded successfully, false otherwise.
 */
bool Utils::Object::load(QObject *object)
{
    if (object == nullptr)
        return false;

    const QString configFilePath =
            QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                    .absoluteFilePath("objectconfig.json");

    QJsonObject configObject;

    QFile configFile(configFilePath);
    if (configFile.open(QFile::ReadOnly)) {
        QJsonDocument configFileDoc = QJsonDocument::fromJson(configFile.readAll());
        configObject = configFileDoc.object();
    } else
        return false;

    const QMetaObject *mo = object->metaObject();
    const QString objectClass = QLatin1String(mo->className());

    if (configObject.contains(objectClass)) {
        QJsonObject objectJson = configObject.value(objectClass).toObject();

        const QStringList keys = objectJson.keys();
        for (const QString &key : keys) {
            const QString cikey = key + "_IsPersistent";
            const int cii = mo->indexOfClassInfo(qPrintable(cikey));
            if (cii >= 0) {
                const QMetaClassInfo ci = mo->classInfo(cii);
                if (!qstrcmp(ci.value(), "false"))
                    objectJson.remove(key);
            }
        }

        return QObjectSerializer::fromJson(objectJson, object);
    }

    return false;
}

/**
 * \brief Generates the configuration metadata for a QObject. This is useful for auto-generating
 * UI for editing object properties.
 * \param object The QObject to analyze.
 * \param from The QMetaObject to start from (optional).
 * \return The ObjectConfig containing fields and groups.
 */
Utils::ObjectConfig Utils::Object::configuration(const QObject *object, const QMetaObject *from)
{
    Utils::ObjectConfig ret;
    if (object == nullptr)
        return ret;

    const QMetaObject *mo = object->metaObject();
    if (from == nullptr || !mo->inherits(from))
        from = object->metaObject();

    auto queryClassInfo = [mo](const char *key) {
        const int ciIndex = mo->indexOfClassInfo(key);
        if (ciIndex < 0)
            return QString();
        const QMetaClassInfo ci = mo->classInfo(ciIndex);
        return QString::fromLatin1(ci.value());
    };

    auto queryPropertyInfo = [queryClassInfo](const QMetaProperty &prop, const char *key) {
        const QString ciKey = QString::fromLatin1(prop.name()) + "_" + QString::fromLatin1(key);
        return queryClassInfo(qPrintable(ciKey));
    };

    auto addFieldToGroup = [&ret, queryClassInfo](const ObjectConfigField &field) {
        int index = -1;
        if (field.group.isEmpty() && !ret.groups.isEmpty())
            index = 0;
        else {
            for (int i = 0; i < ret.groups.size(); i++) {
                const ObjectConfigFieldGroup &group = ret.groups.at(i);
                if (group.name == field.group) {
                    index = i;
                    break;
                }
            }
        }

        if (index < 0) {
            const QString key = field.group + QStringLiteral("_Description");

            ObjectConfigFieldGroup newGroup;
            newGroup.name = field.group;
            newGroup.description = queryClassInfo(qPrintable(key));
            ret.groups.append(newGroup);

            index = ret.groups.size() - 1;
        }

        ret.groups[index].fields.append(field);
    };

    ret.title = queryClassInfo("Title");
    ret.description = queryClassInfo("Description");

    for (int i = from->propertyOffset(); i < mo->propertyCount(); i++) {
        const QMetaProperty prop = mo->property(i);
        if (!prop.isWritable() || !prop.isStored() || !prop.isDesignable())
            continue;

        ObjectConfigField field;
        field.name = QString::fromLatin1(prop.name());
        field.label = queryPropertyInfo(prop, "FieldLabel");
        field.note = queryPropertyInfo(prop, "FieldNote");
        field.editor = queryPropertyInfo(prop, "FieldEditor");
        field.min = queryPropertyInfo(prop, "FieldMinValue");
        field.max = queryPropertyInfo(prop, "FieldMaxValue");
        field.ideal = queryPropertyInfo(prop, "FieldDefaultValue");
        field.group = queryPropertyInfo(prop, "FieldGroup");
        field.feature = queryPropertyInfo(prop, "Feature");

        const QString fieldEnum = queryPropertyInfo(prop, "FieldEnum");
        if (!fieldEnum.isEmpty()) {
            const int enumIndex = mo->indexOfEnumerator(qPrintable(fieldEnum));
            const QMetaEnum enumerator = mo->enumerator(enumIndex);

            for (int j = 0; j < enumerator.keyCount(); j++) {
                ObjectConfigFieldChoice choice;
                choice.key = QString::fromLatin1(enumerator.key(j));
                choice.value = enumerator.value(j);

                const QByteArray ciKey =
                        QByteArray(enumerator.name()) + "_" + QByteArray(enumerator.key(j));
                const QString text = queryClassInfo(ciKey);
                if (!text.isEmpty())
                    choice.key = text;

                field.choices.append(choice);
            }
        }

        ret.fields.append(field);
        addFieldToGroup(field);
    }

    return ret;
}

/**
 * \brief Returns the size of the QObject tree.
 * \param object The root QObject.
 * \return The number of objects in the tree, including the root.
 */
int Utils::Object::treeSize(QObject *object)
{
    return object->findChildren<QObject *>(QString(), Qt::FindChildrenRecursively).size() + 1;
}

static QObject *recursivelyFindChild(QObject *object, const QString &className,
                                     const QString &objectName)
{
    if (object == nullptr || (className.isEmpty() && objectName.isEmpty()))
        return nullptr;

    const QObjectList children = object->children();
    for (QObject *child : children) {
        if (!className.isEmpty()) {
            if (child->inherits(qPrintable(className)))
                return child;
        } else if (!objectName.isEmpty()) {
            if (child->objectName() == objectName)
                return child;
        }

        QObject *ret = recursivelyFindChild(child, className, objectName);

        if (ret)
            return ret;
    }

    return nullptr;
};

/**
 * \brief Finds the first child QObject that inherits from the specified type.
 * \param in The parent QObject to search in.
 * \param typeName The class name to match (e.g., "QWidget").
 * \return The first matching child QObject, or nullptr if not found.
 */
QObject *Utils::Object::firstChildByType(QObject *in, const QString &typeName)
{
    return recursivelyFindChild(in, typeName, QString());
}

/**
 * \brief Finds the first child QObject with the specified object name.
 * \param in The parent QObject to search in.
 * \param objectName The object name to match.
 * \return The first matching child QObject, or nullptr if not found.
 */
QObject *Utils::Object::firstChildByName(QObject *in, const QString &objectName)
{
    return recursivelyFindChild(in, QString(), objectName);
}

static QObject *findParent(QObject *object, const QString &className, const QString &objectName)
{
    if (object == nullptr || (className.isEmpty() && objectName.isEmpty()))
        return nullptr;

    auto getObjectParent = [](QObject *object) -> QObject * {
        if (object == nullptr)
            return nullptr;

        QQuickItem *qmlItem = qobject_cast<QQuickItem *>(object);
        if (qmlItem)
            return qmlItem->parentItem();

        return object->parent();
    };

    QObject *parent = getObjectParent(object);
    while (parent != nullptr) {
        if (parent->inherits(qPrintable(className)))
            return parent;

        parent = getObjectParent(parent);
    }

    parent = object->parent();
    while (parent != nullptr) {
        if (!className.isEmpty()) {
            if (parent->inherits(qPrintable(className)))
                return parent;
        } else if (!objectName.isEmpty()) {
            if (parent->objectName() == objectName)
                return parent;
        }

        parent = parent->parent();
    }

    return nullptr;
}

/**
 * \brief Finds the first parent QObject that inherits from the specified type.
 * \param of The QObject to start searching from.
 * \param typeName The class name to match (e.g., "QWidget").
 * \return The first matching parent QObject, or nullptr if not found.
 */
QObject *Utils::Object::firstParentByType(QObject *of, const QString &typeName)
{
    return findParent(of, typeName, QString());
}

/**
 * \brief Finds the first parent QObject with the specified object name.
 * \param of The QObject to start searching from.
 * \param objectName The object name to match.
 * \return The first matching parent QObject, or nullptr if not found.
 */
QObject *Utils::Object::firstParentByName(QObject *of, const QString &objectName)
{
    return findParent(of, QString(), objectName);
}

static QObject *findSibling(QObject *object, const QString &className, const QString &objectName)
{
    if (object == nullptr || object->parent() == nullptr
        || (className.isEmpty() && objectName.isEmpty()))
        return nullptr;

    QObject *parent = object->parent();
    QObjectList siblings = parent->children();
    siblings.removeOne(object);

    while (!siblings.isEmpty()) {
        QObject *sibling = siblings.takeFirst();
        if (!className.isEmpty()) {
            if (sibling->inherits(qPrintable(className)))
                return sibling;
        } else if (!objectName.isEmpty()) {
            if (sibling->objectName() == objectName)
                return sibling;
        }
    }

    return nullptr;
}

/**
 * \brief Finds the first sibling QObject that inherits from the specified type.
 * \param of The QObject to start searching from.
 * \param typeName The class name to match (e.g., "QWidget").
 * \return The first matching sibling QObject, or nullptr if not found.
 */
QObject *Utils::Object::firstSiblingByType(QObject *of, const QString &typeName)
{
    return findSibling(of, typeName, QString());
}

/**
 * \brief Finds the first sibling QObject with the specified object name.
 * \param of The QObject to start searching from.
 * \param objectName The object name to match.
 * \return The first matching sibling QObject, or nullptr if not found.
 */
QObject *Utils::Object::firstSiblingByName(QObject *of, const QString &objectName)
{
    return findSibling(of, QString(), objectName);
}

/**
 * \brief Returns the parent of a QObject.
 * \param object The QObject.
 * \return The parent QObject, or nullptr.
 */
QObject *Utils::Object::parentOf(QObject *object)
{
    return object ? object->parent() : nullptr;
}

/**
 * \brief Reparents a QObject.
 * \param object The QObject to reparent.
 * \param newParent The new parent.
 * \return True if reparented, false otherwise.
 */
bool Utils::Object::reparent(QObject *object, QObject *newParent)
{
    if (object != nullptr) {
        object->setParent(newParent);
        return true;
    }

    return false;
}

static QString enumerationKey(const QMetaObject *metaObject, const QString &enumName, int value)
{
    QString ret;

    if (metaObject == nullptr || enumName.isEmpty())
        return ret;

    const int enumIndex = metaObject->indexOfEnumerator(qPrintable(enumName));
    if (enumIndex < 0)
        return ret;

    const QMetaEnum enumInfo = metaObject->enumerator(enumIndex);
    if (!enumInfo.isValid())
        return ret;

    return QString::fromLatin1(enumInfo.valueToKey(value));
}

QString Utils::Object::enumKey(QObject *object, const QString &enumName, int value)
{
    if (object != nullptr)
        return ::enumerationKey(object->metaObject(), enumName, value);

    return QString();
}

QString Utils::Object::typeEnumKey(const QString &typeName, const QString &enumName, int value)
{
    const int typeId = QMetaType::type(qPrintable(typeName + "*"));
    const QMetaObject *mo =
            typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    if (mo)
        return ::enumerationKey(mo, enumName, value);

    return QString();
}

QAbstractListModel *Utils::Object::enumModel(QObject *object, const QString &enumName)
{
    if (object == nullptr)
        return nullptr;

    EnumerationModel *ret = new EnumerationModel(object);
    ret->setObject(object);
    ret->setEnumeration(enumName);
    return ret;
}

QAbstractListModel *Utils::Object::typeEnumModel(const QString &typeName, const QString &enumName,
                                                 QObject *parent)
{
    const int typeId = QMetaType::type(qPrintable(typeName + "*"));
    const QMetaObject *mo =
            typeId == QMetaType::UnknownType ? nullptr : QMetaType::metaObjectForType(typeId);
    if (mo)
        return new EnumerationModel(mo, enumName, parent);

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

Q_GLOBAL_STATIC(QObjectListModel<QObject *>, ObjectRegistry)

// When adding objects to the registry, we are not using the built-in objectName property itself,
// but rather a dynamic property as mentioned here. This is done on purpose, to ensure separation
// of intents.
static const char *objectNameProperty = "#objectName";

/**
 * \brief Returns the object registry model.
 * \return A pointer to the QAbstractListModel for the registry.
 */
QAbstractListModel *Utils::ObjectRegistry::model()
{
    return ::ObjectRegistry;
}

/**
 * \brief Removes an object from the registry.
 * \param object The QObject to remove.
 */
void Utils::ObjectRegistry::remove(QObject *object)
{
    ::ObjectRegistry->removeAt(::ObjectRegistry->indexOf(object));
}

/**
 * \brief Adds an object to the registry with a name.
 * \param object The QObject to add.
 * \param name The name to assign (optional).
 * \return The assigned name.
 */
QString Utils::ObjectRegistry::add(QObject *object, const QString &name)
{
    if (object == nullptr)
        return QString();

    QString objName = name;
    if (objName.isEmpty())
        objName = object->objectName();
    if (objName.isEmpty())
        objName = QString::fromLatin1(object->metaObject()->className());

    int index = 0;
    QString finalObjName = objName;
    while (find(finalObjName))
        finalObjName = objName + QString::number(index++);

    object->setProperty(objectNameProperty, finalObjName);
    ::ObjectRegistry->append(object);

    return finalObjName;
}

/**
 * \brief Finds an object in the registry by name.
 * \param name The name to search for.
 * \return The QObject if found, or nullptr.
 */
QObject *Utils::ObjectRegistry::find(const QString &name)
{
    const QList<QObject *> &objects = ::ObjectRegistry->list();
    for (QObject *object : objects) {
        const QString objName = object->property(objectNameProperty).toString();
        if (objName == name)
            return object;
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

Utils::ObjectRegister::ObjectRegister(QObject *parent) : QObject(parent) { }

Utils::ObjectRegister::~ObjectRegister()
{
    Utils::ObjectRegistry::remove(this->parent());
}

Utils::ObjectRegister *Utils::ObjectRegister::qmlAttachedProperties(QObject *parent)
{
    return new Utils::ObjectRegister(parent);
}

void Utils::ObjectRegister::setName(const QString &val)
{
    if (m_name == val)
        return;

    if (!m_name.isEmpty())
        Utils::ObjectRegistry::remove(this->parent());

    m_name = val;

    if (!m_name.isEmpty())
        Utils::ObjectRegistry::add(this->parent(), m_name);

    emit nameChanged();
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Opens a color picker dialog and returns the selected color.
 * \param initial The initial color to show.
 * \return The selected color, or the initial color if canceled.
 */
QColor Utils::Color::pick(const QColor &initial)
{
    QColorDialog::ColorDialogOptions options =
            QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog;
    const QColor ret = QColorDialog::getColor(initial, nullptr, "Select Color", options);
    return ret.isValid() ? ret : initial;
}

/**
 * \brief Applies a whitewashing effect to a color.
 * \param c The original color.
 * \param factor The factor to apply (0.0 to 1.0).
 * \return The modified color.
 */
QColor Utils::Color::whitewash(const QColor &c, qreal factor)
{
    factor = qBound(0.0, factor, 1.0);
    const int r = c.red();
    const int g = c.green();
    const int b = c.blue();

    const int newR = r + static_cast<int>((255 - r) * factor);
    const int newG = g + static_cast<int>((255 - g) * factor);
    const int newB = b + static_cast<int>((255 - b) * factor);

    return QColor(newR, newG, newB, c.alpha());
}

/**
 * \brief Mixes two colors based on the alpha of the second.
 * \param a The base color.
 * \param b The color to mix, with alpha determining the mix factor.
 * \return The mixed color.
 */
QColor Utils::Color::mix(const QColor &a, const QColor &b)
{
    auto mixImpl = [](const QColor &color1, const QColor &color2, qreal tintFactor) {
        const int r1 = color1.red();
        const int g1 = color1.green();
        const int b1 = color1.blue();

        const int r2 = color2.red();
        const int g2 = color2.green();
        const int b2 = color2.blue();

        const int newR = r1 + static_cast<int>((255 - r2) * tintFactor);
        const int newG = g1 + static_cast<int>((255 - g2) * tintFactor);
        const int newB = b1 + static_cast<int>((255 - b2) * tintFactor);

        return QColor(newR, newG, newB);
    };

    const QColor _a = QColor(a.red(), a.green(), a.blue());
    const QColor _b = QColor(b.red(), b.green(), b.blue());
    const qreal factor = b.alphaF();
    return mixImpl(_a, _b, factor);
}

QColor Utils::Color::tint(const QColor &baseColor, const QVariant &tintValue)
{
    if (tintValue.canConvert<QColor>()) {
        QColor tintColor = tintValue.value<QColor>();

        int tintAlpha = tintColor.alpha();
        if (tintAlpha == 255)
            return tintColor;
        if (tintAlpha == 0)
            return baseColor;

        qreal a = tintColor.alphaF();
        qreal inv_a = 1.0 - a;

        qreal r = tintColor.redF() * a + baseColor.redF() * inv_a;
        qreal g = tintColor.greenF() * a + baseColor.greenF() * inv_a;
        qreal b = tintColor.blueF() * a + baseColor.blueF() * inv_a;

        return QColor::fromRgbF(r, g, b, a + inv_a * baseColor.alphaF());
    }

    if (tintValue.canConvert<qreal>()) {
        return translucent(baseColor, tintValue.toReal());
    }

    return baseColor;
}

QColor Utils::Color::stacked(const QColor &foreground, const QColor &background)
{
    const qreal fgAlpha = foreground.alphaF();

    if (qFuzzyCompare(fgAlpha, 0.0))
        return background;

    if (qFuzzyCompare(fgAlpha, 1.0))
        return foreground;

    const qreal bgAlpha = 1.0 - fgAlpha;
    const qreal r = foreground.redF() * fgAlpha + background.redF() * bgAlpha;
    const qreal g = foreground.greenF() * fgAlpha + background.greenF() * bgAlpha;
    const qreal b = foreground.blueF() * fgAlpha + background.blueF() * bgAlpha;

    return QColor::fromRgbF(r, g, b, 1.0);
}

/**
 * \brief Makes a color more translucent.
 * \param input The input color.
 * \param alpha The alpha factor to apply.
 * \return The translucent color.
 */
QColor Utils::Color::translucent(const QColor &input, qreal alpha)
{
    QColor ret = input;
    ret.setAlphaF(qBound(0.0, ret.alphaF() * alpha, 1.0));
    return ret;
}

// We'll come back to this function when we implement dark mode
inline qreal evaluateLuminance(const QColor &color, const QColor &background = Qt::white)
{
    const qreal alpha = color.alphaF();
    if (alpha < 1.0) {
        const qreal r = color.redF() * alpha + background.redF() * (1.0 - alpha);
        const qreal g = color.greenF() * alpha + background.greenF() * (1.0 - alpha);
        const qreal b = color.blueF() * alpha + background.blueF() * (1.0 - alpha);
        return (0.299 * r) + (0.587 * g) + (0.114 * b);
    }

    return ((0.299 * color.redF()) + (0.587 * color.greenF()) + (0.114 * color.blueF()));
}

/**
 * \brief Checks if a color is light.
 * \param color The color to check.
 * \return True if the color is light, false otherwise.
 */
bool Utils::Color::isLight(const QColor &color)
{
    return evaluateLuminance(color) > 0.5;
}

/**
 * \brief Checks if a color is very light.
 * \param color The color to check.
 * \return True if the color is very light, false otherwise.
 */
bool Utils::Color::isVeryLight(const QColor &color)
{
    return evaluateLuminance(color) > 0.8;
}

/**
 * \brief Returns the appropriate text color for a background color.
 * \param backgroundColor The background color.
 * \return Black or white, depending on the background.
 */
QColor Utils::Color::textColorFor(const QColor &backgroundColor)
{
    return isLight(backgroundColor) ? Qt::black : Qt::white;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Writes content to a file.
 * \param fileName The file path.
 * \param content The content to write.
 * \return True if written successfully, false otherwise.
 */
bool Utils::File::write(const QString &fileName, const QString &content)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        file.write(content.toLatin1());
        return true;
    }

    return false;
}

/**
 * \brief Reveals a file on the desktop.
 * \param path The file path.
 */
void Utils::File::revealOnDesktop(const QString &path)
{
    Application::instance()->revealFileOnDesktop(path);
}

/**
 * \brief Copies a file to a folder, avoiding name conflicts.
 * \param fromFilePath The source file path.
 * \param toFolder The destination folder.
 * \return The path of the copied file, or empty on failure.
 */
QString Utils::File::copyToFolder(const QString &fromFilePath, const QString &toFolder)
{
    const QFileInfo fromFileInfo(fromFilePath);
    if (fromFileInfo.isDir() || !fromFileInfo.isReadable() || fromFileInfo.isSymbolicLink())
        return QString();

    const QFileInfo toInfo(toFolder);
    QString toFilePath = toInfo.isDir() ? toInfo.dir().absoluteFilePath(fromFileInfo.fileName())
                                        : toInfo.absoluteFilePath();
    int counter = 1;
    while (1) {
        if (QFile::exists(toFilePath)) {
            const QFileInfo toFileInfo(toFilePath);
            toFilePath = toFileInfo.absoluteDir().absoluteFilePath(
                    toFileInfo.completeBaseName() + QStringLiteral(" ") + QString::number(counter++)
                    + QStringLiteral(".") + toFileInfo.suffix());
        } else
            break;
    }

    toFilePath = sanitiseName(toFilePath);

    const bool success = QFile::copy(fromFileInfo.absoluteFilePath(), toFilePath);
    if (success)
        return toFilePath;

    return QString();
}

/**
 * \brief Returns the complete base name of a file path.
 * \param path The file path.
 * \return The base name without extension.
 */
QString Utils::File::completeBaseName(const QString &path)
{
    return QFileInfo(path).completeBaseName();
}

/**
 * \brief Returns the path of a neighboring file.
 * \param filePath The reference file path.
 * \param nfileName The neighboring file name.
 * \return The full path to the neighboring file.
 */
QString Utils::File::neighbouringFilePath(const QString &filePath, const QString &nfileName)
{
    const QFileInfo fi(filePath);
    return fi.absoluteDir().absoluteFilePath(nfileName);
}

/**
 * \brief Returns the directory path of a file.
 * \param name The file path.
 * \return The absolute path of the directory.
 */
QString Utils::File::path(const QString &name)
{
    return QFileInfo(name).absolutePath();
}

/**
 * \brief Reads the content of a file.
 * \param fileName The file path.
 * \return The file content as a string, or empty on failure.
 */
QString Utils::File::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QString();

    return QString::fromLatin1(file.readAll());
}

/**
 * \brief Sanitizes a file name by removing invalid characters and ensuring validity across
 * platforms. \param fileName The file name to sanitize. \param removedChars Optional set to store
 * removed characters. \return The sanitized file path.
 */
QString Utils::File::sanitiseName(const QString &fileName, QSet<QChar> *removedChars)
{
    const QFileInfo fi(fileName);
    QString completeBaseName = fi.completeBaseName();
    bool changed = false;

    // Disallowed characters: \, /, :, *, ?, ", <, >, |, and ASCII control (0-31)
    static const QSet<QChar> disallowedChars = { '\\', '/', ':', '*', '?', '"', '<', '>', '|' };

    // Remove disallowed characters
    for (int i = completeBaseName.length() - 1; i >= 0; i--) {
        const QChar ch = completeBaseName.at(i);
        if (ch.isLetterOrNumber() || (!disallowedChars.contains(ch) && ch.unicode() > 31)) {
            continue;
        }

        if (removedChars) {
            *removedChars += ch;
        }

        completeBaseName.remove(i, 1);
        changed = true;
    }

    // Trim trailing spaces and periods
    completeBaseName = completeBaseName.trimmed();
    while (!completeBaseName.isEmpty() && completeBaseName.endsWith('.')) {
        completeBaseName.chop(1);
        changed = true;
    }

    // Check for Windows reserved names (case-insensitive, even with extensions)
    static const QSet<QString> reservedNames = { "CON",  "PRN",  "AUX",  "NUL",  "COM1", "COM2",
                                                 "COM3", "COM4", "COM5", "COM6", "COM7", "COM8",
                                                 "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5",
                                                 "LPT6", "LPT7", "LPT8", "LPT9" };
    QString baseUpper = completeBaseName.toUpper();
    if (reservedNames.contains(baseUpper)) {
        completeBaseName += "_";
        changed = true;
    }

    // Enforce maximum filename length (255 characters for base name)
    if (completeBaseName.length() > 255) {
        completeBaseName = completeBaseName.left(255);
        changed = true;
        // Re-trim after truncation
        completeBaseName = completeBaseName.trimmed();
        while (!completeBaseName.isEmpty() && completeBaseName.endsWith('.')) {
            completeBaseName.chop(1);
        }
    }

    // Ensure base name isn't empty
    if (completeBaseName.isEmpty()) {
        completeBaseName = "untitled";
        changed = true;
    }

    if (changed) {
        return fi.absoluteDir().absoluteFilePath(completeBaseName + "." + fi.suffix());
    }

    return fileName;
}

/**
 * \brief Returns file information for a path.
 * \param path The file path.
 * \return A FileInfo object.
 */
Utils::FileInfo Utils::File::info(const QString &path)
{
    return Utils::FileInfo({ QFileInfo(path) });
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Sets the mouse cursor shape.
 * \param shape The cursor shape to set.
 */
void Utils::MouseCursor::setShape(Qt::CursorShape shape)
{
    QtApplicationClass::setOverrideCursor(QCursor(shape));
}

/**
 * \brief Restores the default mouse cursor shape.
 */
void Utils::MouseCursor::unsetShape()
{
    QtApplicationClass::restoreOverrideCursor();
}

/**
 * \brief Moves the mouse cursor to a global position.
 * \param globalPos The position to move to.
 * \return True if moved, false otherwise.
 */
bool Utils::MouseCursor::moveTo(const QPointF &globalPos)
{
    QCursor::setPos(globalPos.x(), globalPos.y());
    return true;
}

/**
 * \brief Checks if the mouse is over a QQuickItem.
 * \param item The item to check.
 * \return True if the cursor is over the item, false otherwise.
 */
bool Utils::MouseCursor::isOverItem(QQuickItem *item)
{
    if (item == nullptr)
        return false;

    const QPointF pos = item->mapFromGlobal(QCursor::pos());
    return item->boundingRect().contains(pos);
}

/**
 * \brief Returns the current mouse cursor position.
 * \return The global position of the cursor.
 */
QPointF Utils::MouseCursor::position()
{
    return QCursor::pos();
}

/**
 * \brief Returns the mouse position relative to a QQuickItem.
 * \param item The item.
 * \return The position relative to the item.
 */
QPointF Utils::MouseCursor::itemPosition(QQuickItem *item)
{
    return item ? item->mapFromGlobal(QCursor::pos()) : QPointF();
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Returns the color palette for a given version.
 * \param version The version number.
 * \return A list of colors in the palette.
 */
QList<QColor> Utils::SceneColors::paletteForVersion(const QVersionNumber &version)
{
    // Up-until version 0.2.17 Beta
    if (!version.isNull() && version <= QVersionNumber(0, 2, 17))
        return QList<QColor>() << QColor(Qt::blue) << QColor(Qt::magenta) << QColor(Qt::darkGreen)
                               << QColor(128, 0, 128 /* purple */) << QColor(Qt::yellow)
                               << QColor(255, 165, 0 /* orange */) << QColor(Qt::red)
                               << QColor(165, 42, 42 /* brown */) << QColor(Qt::gray)
                               << QColor(Qt::white);

    // New set of colors
    return QList<QColor>() << QColor(33, 150, 243) // #2196F3 - Blue
                           << QColor(233, 30, 99) // #E91E63 - Pink
                           << QColor(0, 150, 136) // #009688 - Teal
                           << QColor(156, 39, 176) // #9C27B0 - Purple
                           << QColor(255, 235, 59) // #FFEB3B - Yellow
                           << QColor(255, 152, 0) // #FF9800 - Orange
                           << QColor(244, 67, 54) // #F44336 - Red
                           << QColor(121, 85, 72) // #795548 - Brown
                           << QColor(158, 158, 158) // #9E9E9E - Grey
                           << QColor(250, 250, 250) // #FAFAFA - Off-white
                           << QColor(63, 81, 181) // #3F51B5 - Indigo
                           << QColor(205, 220, 57); // #CDDC39 - Lime
}

/**
 * \brief Returns the current color palette.
 * \return A list of colors.
 */
QList<QColor> Utils::SceneColors::palette()
{
    return paletteForVersion(QVersionNumber());
}

/**
 * \brief Picks a color from the palette by index.
 * \param index The index in the palette.
 * \return The color at the index, or white if out of bounds.
 */
QColor Utils::SceneColors::pick(int index)
{
    const QList<QColor> list = palette();
    if (list.isEmpty())
        return QColor(Qt::white);

    return list.at(qMax(index, 0) % list.size());
}

/**
 * \brief Returns the palette as a QVariantList.
 * \return The palette as a list of QVariant.
 */
QVariantList Utils::SceneColors::paletteAsVariantList()
{
    const QList<QColor> list = palette();
    QVariantList ret;
    ret.reserve(list.size());
    for (const QColor &color : list)
        ret << QVariant::fromValue(color);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Checks if two rectangles intersect.
 * \param r1 The first rectangle.
 * \param r2 The second rectangle.
 * \return True if they intersect, false otherwise.
 */
bool Utils::GMath::doRectanglesIntersect(const QRectF &r1, const QRectF &r2)
{
    return r1.intersects(r2);
}

/**
 * \brief Checks if one rectangle is completely inside another.
 * \param bigRect The containing rectangle.
 * \param smallRect The contained rectangle.
 * \return True if smallRect is inside bigRect, false otherwise.
 */
bool Utils::GMath::isRectangleInRectangle(const QRectF &bigRect, const QRectF &smallRect)
{
    return bigRect.contains(smallRect);
}

/**
 * \brief Calculates the distance between two points.
 * \param p1 The first point.
 * \param p2 The second point.
 * \return The distance.
 */
qreal Utils::GMath::distanceBetweenPoints(const QPointF &p1, const QPointF &p2)
{
    return QLineF(p1, p2).length();
}

/**
 * \brief Adjusts a rectangle by margins.
 * \param rect The original rectangle.
 * \param left Left adjustment.
 * \param top Top adjustment.
 * \param right Right adjustment.
 * \param bottom Bottom adjustment.
 * \return The adjusted rectangle.
 */
QRectF Utils::GMath::adjustRectangle(const QRectF &rect, qreal left, qreal top, qreal right,
                                     qreal bottom)
{
    return rect.adjusted(left, top, right, bottom);
}

/**
 * \brief Returns the bounding rectangle for text with a font.
 * \param text The text.
 * \param font The font.
 * \return The bounding rectangle.
 */
QRectF Utils::GMath::boundingRect(const QString &text, const QFont &font)
{
    const QFontMetricsF fm(font);
    return fm.boundingRect(text);
}

/**
 * \brief Returns the intersection of two rectangles.
 * \param of The first rectangle.
 * \param with The second rectangle.
 * \return The intersected rectangle.
 */
QRectF Utils::GMath::intersectedRectangle(const QRectF &of, const QRectF &with)
{
    return of.intersected(with);
}

/**
 * \brief Finds the largest bounding rectangle for a list of strings.
 * \param strings The list of strings.
 * \param font The font.
 * \return The largest bounding rectangle.
 */
QRectF Utils::GMath::largestBoundingRect(const QStringList &strings, const QFont &font)
{
    if (strings.isEmpty())
        return QRectF();

    const QFontMetricsF fm(font);

    QRectF ret;
    for (const QString &item : strings) {
        const QRectF trect = fm.boundingRect(item);
        if (ret.isEmpty())
            ret = trect;
        else
            ret |= trect;
    }

    return ret;
}

/**
 * \brief Queries a sub-rectangle within bounds.
 * \param in The containing rectangle.
 * \param around The center point.
 * \param atBest The desired size.
 * \return The sub-rectangle.
 */
QRectF Utils::GMath::querySubRectangle(const QRectF &in, const QRectF &around, const QSizeF &atBest)
{
    if (in.width() < atBest.width() || in.height() < atBest.height()) {
        QRectF ret(0, 0, atBest.width(), atBest.height());
        ret.moveCenter(around.center());
        return ret;
    }

    QRectF around2;
    if (atBest.width() > in.width() || atBest.height() > in.height())
        around2 =
                QRectF(0, 0, qMin(atBest.width(), in.width()), qMin(atBest.height(), in.height()));
    else
        around2 = QRectF(0, 0, atBest.width(), atBest.height());
    around2.moveCenter(around.center());

    const QSizeF aroundSize = around2.size();

    around2 = in.intersected(around2);
    if (qFuzzyCompare(around2.width(), aroundSize.width())
        && qFuzzyCompare(around2.height(), aroundSize.height()))
        return around2;

    around2.setSize(aroundSize);

    if (around2.left() < in.left())
        around2.moveLeft(in.left());
    else if (around2.right() > in.right())
        around2.moveRight(in.right());

    if (around2.top() < in.top())
        around2.moveTop(in.top());
    else if (around2.bottom() > in.bottom())
        around2.moveBottom(in.bottom());

    return around2;
}

/**
 * \brief Returns the union of two rectangles.
 * \param r1 The first rectangle.
 * \param r2 The second rectangle.
 * \return The united rectangle.
 */
QRectF Utils::GMath::uniteRectangles(const QRectF &r1, const QRectF &r2)
{
    return r1.united(r2);
}

/**
 * \brief Scales a size to fit within another while keeping aspect ratio.
 * \param of The original size.
 * \param into The target size.
 * \return The scaled size.
 */
QSizeF Utils::GMath::scaledSize(const QSizeF &of, const QSizeF &into)
{
    return of.scaled(into, Qt::KeepAspectRatio);
}

/**
 * \brief Returns the center point of a rectangle.
 * \param rect The rectangle.
 * \return The center point.
 */
QPointF Utils::GMath::centerOf(const QRectF &rect)
{
    return rect.center();
}

/**
 * \brief Calculates the translation needed to bring a rectangle inside another.
 * \param bigRect The containing rectangle.
 * \param smallRect The rectangle to move.
 * \return The translation point.
 */
QPointF Utils::GMath::translationRequiredToBringRectangleInRectangle(const QRectF &bigRect,
                                                                     const QRectF &smallRect)
{
    QPointF ret(0, 0);

    if (!bigRect.contains(smallRect)) {
        if (smallRect.left() < bigRect.left())
            ret.setX(bigRect.left() - smallRect.left());
        else if (smallRect.right() > bigRect.right())
            ret.setX(-(smallRect.right() - bigRect.right()));

        if (smallRect.top() < bigRect.top())
            ret.setY(bigRect.top() - smallRect.top());
        else if (smallRect.bottom() > bigRect.bottom())
            ret.setY(-(smallRect.bottom() - bigRect.bottom()));
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Sleeps for a specified number of milliseconds, without blocking UI events.
 * \param ms The milliseconds to sleep (max 2000).
 */
void Utils::TMath::sleep(int ms)
{
    ms = qBound(0, ms, 2000);
    if (ms == 0) {
        QEventLoop eventLoop;
        eventLoop.processEvents(QEventLoop::ExcludeUserInputEvents);
        return;
    }

    QEventLoop eventLoop;
    QTimer::singleShot(ms, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);
}

/**
 * \brief Converts seconds to a QTime object.
 * \param nrSeconds The number of seconds.
 * \return The QTime representation.
 */
QTime Utils::TMath::secondsToTime(int nrSeconds)
{
    if (nrSeconds <= 0)
        return QTime(0, 0, 0);

    int hours = nrSeconds / 3600;
    int minutes = (nrSeconds % 3600) / 60;
    int seconds = nrSeconds % 60;

    return QTime(hours, minutes, seconds);
}

/**
 * \brief Returns a relative time string for a date-time.
 * \param dt The date-time.
 * \return A human-readable relative time string.
 */
QString Utils::TMath::relativeTime(const QDateTime &dt)
{
    if (!dt.isValid())
        return QStringLiteral("Unknown Time");

    const QDateTime now = QDateTime::currentDateTime();
    if (dt > now) {
        return QLocale::system().toString(dt, QLocale::LongFormat);
    }

    if (now.date() == dt.date()) {
        const int secsInMin = 60;
        const int nrSecs = dt.time().secsTo(now.time());

        if (nrSecs < secsInMin)
            return QStringLiteral("Less than a minute ago");

        const int totalMins = qCeil(static_cast<qreal>(nrSecs) / secsInMin);
        const int hours = totalMins / 60;
        const int mins = totalMins % 60;

        if (hours == 0)
            return QString::number(totalMins) + QStringLiteral("m ago");

        return QString::number(hours) + QStringLiteral("h ") + QString::number(mins)
                + QStringLiteral("m ago");
    }

    const int nrDays = dt.date().daysTo(now.date());
    const QString time = dt.time().toString(QStringLiteral("h:mm A"));
    switch (nrDays) {
    case 1:
        return QStringLiteral("Yesterday @ ") + time;
    case 2:
        return QStringLiteral("Day before yesterday @ ") + time;
    case 3:
    case 4:
    case 5:
    case 6:
        return QString::number(nrDays) + QStringLiteral(" days ago @ ") + time;
    default:
        break;
    }

    if (nrDays >= 7 && nrDays < 14)
        return QStringLiteral("Last week ")
                + QLocale::system().standaloneDayName(dt.date().dayOfWeek()) + " @ " + time;

    if (nrDays >= 14 && nrDays < 21)
        return QStringLiteral("Two weeks ago");

    if (nrDays >= 21 && nrDays < 28)
        return QStringLiteral("Three weeks ago");

    if (nrDays >= 28 && nrDays < 60)
        return QStringLiteral("Little more than a month ago");

    return QStringLiteral("More than two months ago");
}

/**
 * Used for display time-offsets in a screenplay text document for example.
 */
QString Utils::TMath::timeLengthString(const QTime &time)
{
    const int h = time.hour();
    const int m = time.minute();
    const int s = time.second();

    if (h > 0) {
        if (m == 0 && s == 0)
            return QStringLiteral("%1h").arg(h); // 1h
        if (s == 0)
            return QStringLiteral("%1:%2h").arg(h).arg(m, 2, 10, QChar('0')); // 1:02h
        return QStringLiteral("%1:%2:%3")
                .arg(h)
                .arg(m, 2, 10, QChar('0'))
                .arg(s, 2, 10, QChar('0')); // 1:02:25 or 1:00:25
    }

    if (m > 0) {
        if (s == 0)
            return QStringLiteral("%1m").arg(m); // 1m
        return QStringLiteral("%1:%2m").arg(m).arg(s, 2, 10, QChar('0')); // 1:20m
    }

    if (s > 0)
        return QStringLiteral("%1s").arg(s); // 20s

    if (time.isNull())
        return QString();

    return QStringLiteral("0s");
}

/**
 * Standard method to be used for displaying time
 */
QString Utils::TMath::timeToString(const QTime &time)
{
    const QString format = QLocale::system().timeFormat(QLocale::ShortFormat);
    return time.toString(format);
}

/**
 * Standard method to be used for displaying date
 */
QString Utils::TMath::dateToString(const QDate &date)
{
    const QString format = QLocale::system().dateFormat(QLocale::ShortFormat);
    return date.toString(format);
}

/**
 * Standard method to be used for displaying date-time
 */
QString Utils::TMath::dateTimeToString(const QDateTime &dateTime)
{
    const QString format = QLocale::system().dateTimeFormat(QLocale::ShortFormat);
    return dateTime.toString(format);
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Sets text to the clipboard.
 * \param text The text to set.
 */
void Utils::Clipboard::set(const QString &text)
{
    Application::clipboard()->setText(text);
}

/**
 * \brief Gets text from the clipboard.
 * \return The clipboard text.
 */
QString Utils::Clipboard::get()
{
    return Application::clipboard()->text();
}

///////////////////////////////////////////////////////////////////////////////

#ifdef Q_OS_WIN
static QString hkue()
{
    static const QString ret = QLatin1String("HKEY_CURRENT_USER\\Environment\\");
    return ret;
}
#endif

/**
 * \brief Sets an environment variable.
 * \param key The variable key.
 * \param value The value to set.
 */
void Utils::SystemEnvironment::set(const QString &key, const QVariant &value)
{
#ifdef Q_OS_WIN
    QSettings settings(hkue(), QSettings::NativeFormat);
    settings.setValue(key, value);
#endif

    QProcessEnvironment::systemEnvironment().insert(key, value.toString());
}

/**
 * \brief Removes an environment variable.
 * \param key The variable key.
 */
void Utils::SystemEnvironment::remove(const QString &key)
{
#ifdef Q_OS_WIN
    QSettings settings(hkue(), QSettings::NativeFormat);
    settings.remove(key);
#endif

    QProcessEnvironment::systemEnvironment().remove(key);
}

/**
 * \brief Gets an environment variable value.
 * \param key The variable key.
 * \param fallback The fallback value if not found.
 * \return The variable value or fallback.
 */
QVariant Utils::SystemEnvironment::get(const QString &key, const QVariant &fallback)
{
#ifdef Q_OS_WIN
    QSettings settings(hkue(), QSettings::NativeFormat);
    if (settings.contains(key)) {
        return settings.value(key, fallback);
    }

    return fallback;
#else
    return QProcessEnvironment::systemEnvironment().value(key, fallback.toString());
#endif
}

///////////////////////////////////////////////////////////////////////////////

/**
 * \brief Converts a string to title case.
 * \param val The input string.
 * \return The title-cased string.
 */
QString Utils::SMath::titleCased(const QString &val)
{
    QString val2 = val.toLower();
    if (val2.isEmpty())
        return val;

    const QList<QChar> exclude = QList<QChar>() << QChar('\'');
    bool capitalize = true;
    for (int i = 0; i < val2.length(); i++) {
        QChar ch = val2.at(i);
        if (ch.isLetter() && ch.script() != QChar::Script_Latin)
            return val;

        if (capitalize && ch.isLetter() && ch.script() == QChar::Script_Latin) {
            val2[i] = ch.toUpper();
            capitalize = false;
        } else if (!ch.isLetter() && !exclude.contains(ch)) {
            capitalize = true;
        }
    }

    return val2;
}

/**
 * \brief Creates a unique ID using UUID.
 * \return A unique string ID.
 */
QString Utils::SMath::createUniqueId()
{
    return QUuid::createUuid().toString();
}

bool Utils::SMath::doListsIntersect(const QStringList &a, const QStringList &b,
                                    Qt::CaseSensitivity cs)
{
    if (a.isEmpty() || b.isEmpty())
        return false;

    // For performance, iterate over the smaller list and check against the larger one.
    const QStringList &smaller = (a.size() < b.size()) ? a : b;
    const QStringList &larger = (a.size() < b.size()) ? b : a;

    for (const QString &item : smaller) {
        if (larger.contains(item, cs))
            return true;
    }

    return false;
}

bool Utils::SMath::isValidUrl(const QString &url)
{
    if (!url.isEmpty()) {
        const QUrl qurl(url);
        return qurl.isValid() && !qurl.isRelative();
    }

    return false;
}

bool Utils::SMath::isValidEmail(const QString &email)
{
    if (!email.isEmpty()) {
        const QRegularExpression emailRegex(
                R"((^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$))");
        return emailRegex.match(email).hasMatch();
    }

    return false;
}

QString Utils::SMath::formatAsBulletPoints(const QVariantList &items)
{
    QString ret;
    QTextStream ts(&ret, QIODevice::WriteOnly);

    bool listStarted = false;
    for (const QVariant &item : items) {
        if (!item.isValid())
            continue;

        const QString text = item.toString();
        if (text.isEmpty())
            continue;

        if (!listStarted) {
            ts << "<ul>";
            listStarted = true;
        }

        ts << "<li>" << text << "</li>";
    }

    if (listStarted)
        ts << "</ul>";

    ts.flush();

    return ret;
}

/**
 * \brief Converts a QPainterPath to a string.
 * \param val The painter path.
 * \return The string representation.
 */
QString Utils::SMath::painterPathToString(const QPainterPath &val)
{
    QByteArray ret;
    {
        QDataStream ds(&ret, QIODevice::WriteOnly);
        ds << val;
    }

    return QString::fromLatin1(ret.toHex());
}

/**
 * \brief Converts a string back to a QPainterPath.
 * \param val The string representation.
 * \return The painter path.
 */
QPainterPath Utils::SMath::stringToPainterPath(const QString &val)
{
    const QByteArray bytes = QByteArray::fromHex(val.toLatin1());
    QDataStream ds(bytes);
    QPainterPath path;
    ds >> path;
    return path;
}

/**
 * \brief Replaces character names in a JSON delta object.
 * \param from The old character name.
 * \param to The new character name.
 * \param delta The JSON delta.
 * \param nrReplacements Optional pointer to count replacements.
 * \return The modified JSON delta.
 */
QJsonObject Utils::SMath::replaceCharacterName(const QString &from, const QString &to,
                                               const QJsonObject &delta, int *nrReplacements)
{
    const QString opsAttr = QStringLiteral("ops");
    const QString insertAttr = QStringLiteral("insert");

    QJsonArray ops = delta.value(opsAttr).toArray();

    int totalCount = 0;

    for (int i = 0; i < ops.size(); i++) {
        QJsonValueRef item = ops[i];
        QJsonObject op = item.toObject();

        QJsonValue insert = op.value(insertAttr);
        if (insert.isString()) {
            int count = 0;
            insert = replaceCharacterName(from, to, insert.toString(), &count);
            if (count > 0) {
                totalCount += count;
                op.insert(insertAttr, insert);
                item = op;
            }
        }
    }

    if (totalCount > 0) {
        if (nrReplacements)
            *nrReplacements = totalCount;

        QJsonObject ret = delta;
        ret.insert(opsAttr, ops);
        return ret;
    }

    return delta;
}

/**
 * \brief Replaces character names in a string.
 * \param from The old character name.
 * \param to The new character name.
 * \param in The input string.
 * \param nrReplacements Optional pointer to count replacements.
 * \return The modified string.
 */
QString Utils::SMath::replaceCharacterName(const QString &from, const QString &to,
                                           const QString &in, int *nrReplacements)
{
    QString text = in.trimmed();
    QList<int> replacePositions;

    int pos = 0; // search from last
    while (pos < text.length()) {
        pos = text.indexOf(from, pos, Qt::CaseInsensitive);
        if (pos < 0)
            break;

        if (pos > 0) {
            const QChar ch = text.at(pos - 1);
            if (!ch.isPunct() && !ch.isSpace()) {
                pos += from.length();
                continue;
            }
        }

        bool found = false;
        if (pos + from.length() < text.length()) {
            const QChar ch = text.at(pos + from.length());
            found = ch.isPunct() || ch.isSpace();
        } else
            found = (text.compare(from, Qt::CaseInsensitive) == 0);

        if (found)
            replacePositions << pos;

        pos += from.length();
    }

    if (!replacePositions.isEmpty()) {
        if (nrReplacements)
            *nrReplacements = replacePositions.size();

        for (int i = replacePositions.size() - 1; i >= 0; i--) {
            const int pos = replacePositions.at(i);
            const bool allCaps = [](const QString &val) {
                return val == val.toUpper();
            }(text.mid(pos, from.length()));
            text = text.replace(pos, from.length(), allCaps ? to.toUpper() : to);
        }

        return text;
    }

    return in;
}

///////////////////////////////////////////////////////////////////////////////

Utils::SystemClipboard::SystemClipboard(QObject *parent) : QObject(parent)
{
    m_clipboard = qApp->clipboard();
    if (m_clipboard != nullptr)
        connect(m_clipboard, &QClipboard::changed, this, &SystemClipboard::textChanged);
}

Utils::SystemClipboard::~SystemClipboard() { }

void Utils::SystemClipboard::setText(const QString &val)
{
    if (m_clipboard != nullptr)
        m_clipboard->setText(val);
}

QString Utils::SystemClipboard::text() const
{
    if (m_clipboard != nullptr)
        return m_clipboard->text();

    return QString();
}
